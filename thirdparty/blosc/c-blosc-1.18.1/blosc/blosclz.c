/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Author: Francesc Alted <francesc@blosc.org>
  Creation date: 2009-05-20

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

/*********************************************************************
  The code in this file is heavily based on FastLZ, a lightning-fast
  lossless compression library.  See LICENSES/FASTLZ.txt for details.
**********************************************************************/


#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) && !defined(__MINGW32__)
  #include <windows.h>
  /* stdint.h only available in VS2010 (VC++ 16.0) and newer */
  #if defined(_MSC_VER) && _MSC_VER < 1600
    #include "win32/stdint-windows.h"
  #else
    #include <stdint.h>
  #endif
#else
  #include <stdint.h>
#endif  /* _WIN32 */

#include "blosclz.h"
#include "fastcopy.h"
#include "blosc-common.h"
#include "blosc-comp-features.h"


/*
 * Give hints to the compiler for branch prediction optimization.
 */
#if defined(__GNUC__) && (__GNUC__ > 2)
#define BLOSCLZ_EXPECT_CONDITIONAL(c)    (__builtin_expect((c), 1))
#define BLOSCLZ_UNEXPECT_CONDITIONAL(c)  (__builtin_expect((c), 0))
#else
#define BLOSCLZ_EXPECT_CONDITIONAL(c)    (c)
#define BLOSCLZ_UNEXPECT_CONDITIONAL(c)  (c)
#endif

/*
 * Use inlined functions for supported systems.
 */
#if defined(_MSC_VER) && !defined(__cplusplus)   /* Visual Studio */
#define inline __inline  /* Visual C is not C99, but supports some kind of inline */
#endif

#define MAX_COPY 32U
#define MAX_DISTANCE 8191
#define MAX_FARDISTANCE (65535 + MAX_DISTANCE - 1)

#ifdef BLOSC_STRICT_ALIGN
#define BLOSCLZ_READU16(p) ((p)[0] | (p)[1]<<8)
  #define BLOSCLZ_READU32(p) ((p)[0] | (p)[1]<<8 | (p)[2]<<16 | (p)[3]<<24)
#else
#define BLOSCLZ_READU16(p) *((const uint16_t*)(p))
#define BLOSCLZ_READU32(p) *((const uint32_t*)(p))
#endif

#define HASH_LOG (14U)

/* Simple, but pretty effective hash function for 3-byte sequence */
// This is the original hash function used in fastlz
//#define HASH_FUNCTION(v, p, h) {                         \
//  v = BLOSCLZ_READU16(p);                                \
//  v ^= BLOSCLZ_READU16(p + 1) ^ ( v >> (16 - h));        \
//  v &= (1 << h) - 1;                                     \
//}

// This is used in LZ4 and seems to work pretty well here too
#define HASH_FUNCTION(v, p, h)  \
  v = ((BLOSCLZ_READU32(p) * 2654435761U) >> (32U - h))


#define LITERAL(ip, op, op_limit, anchor, copy) {        \
  if (BLOSCLZ_UNEXPECT_CONDITIONAL(op + 2 > op_limit))   \
    goto out;                                            \
  *op++ = *anchor++;                                     \
  ip = anchor;                                           \
  copy++;                                                \
  if (BLOSCLZ_UNEXPECT_CONDITIONAL(copy == MAX_COPY)) {  \
    copy = 0;                                            \
    *op++ = MAX_COPY-1;                                  \
    nmax_copies++;                                       \
    if (nmax_copies > max_nmax_copies)                   \
      goto out;                                          \
  }                                                      \
  continue;                                              \
}

#define IP_BOUNDARY 2


static uint8_t *get_run(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
  uint8_t x = ip[-1];
  int64_t value, value2;
  /* Broadcast the value for every byte in a 64-bit register */
  memset(&value, x, 8);
  /* safe because the outer check against ip limit */
  while (ip < (ip_bound - sizeof(int64_t))) {
#if defined(BLOSC_STRICT_ALIGN)
    memcpy(&value2, ref, 8);
#else
    value2 = ((int64_t*)ref)[0];
#endif
    if (value != value2) {
      /* Return the byte that starts to differ */
      while (*ref++ == x) ip++;
      return ip;
    }
    else {
      ip += 8;
      ref += 8;
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == x)) ip++;
  return ip;
}

#ifdef __SSE2__
static uint8_t *get_run_16(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
  uint8_t x = ip[-1];

  if (ip < (ip_bound - sizeof(int64_t))) {
    int64_t value, value2;
    /* Broadcast the value for every byte in a 64-bit register */
    memset(&value, x, 8);
#if defined(BLOSC_STRICT_ALIGN)
    memcpy(&value2, ref, 8);
#else
    value2 = ((int64_t*)ref)[0];
#endif
    if (value != value2) {
      /* Return the byte that starts to differ */
      while (*ref++ == x) ip++;
      return ip;
    }
    else {
      ip += 8;
      ref += 8;
    }
  }
  /* safe because the outer check against ip limit */
  while (ip < (ip_bound - sizeof(__m128i))) {
    __m128i value, value2, cmp;
    /* Broadcast the value for every byte in a 128-bit register */
    memset(&value, x, sizeof(__m128i));
    value2 = _mm_loadu_si128((__m128i *)ref);
    cmp = _mm_cmpeq_epi32(value, value2);
    if (_mm_movemask_epi8(cmp) != 0xFFFF) {
      /* Return the byte that starts to differ */
      while (*ref++ == x) ip++;
      return ip;
    }
    else {
      ip += sizeof(__m128i);
      ref += sizeof(__m128i);
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == x)) ip++;
  return ip;
}
#endif


#ifdef __AVX2__
static uint8_t *get_run_32(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
  uint8_t x = ip[-1];
  /* safe because the outer check against ip limit */
  if (ip < (ip_bound - sizeof(int64_t))) {
    int64_t value, value2;
    /* Broadcast the value for every byte in a 64-bit register */
    memset(&value, x, 8);
#if defined(BLOSC_STRICT_ALIGN)
    memcpy(&value2, ref, 8);
#else
    value2 = ((int64_t*)ref)[0];
#endif
    if (value != value2) {
      /* Return the byte that starts to differ */
      while (*ref++ == x) ip++;
      return ip;
    }
    else {
      ip += 8;
      ref += 8;
    }
  }
  if (ip < (ip_bound - sizeof(__m128i))) {
    __m128i value, value2, cmp;
    /* Broadcast the value for every byte in a 128-bit register */
    memset(&value, x, sizeof(__m128i));
    value2 = _mm_loadu_si128((__m128i *) ref);
    cmp = _mm_cmpeq_epi32(value, value2);
    if (_mm_movemask_epi8(cmp) != 0xFFFF) {
      /* Return the byte that starts to differ */
      while (*ref++ == x) ip++;
      return ip;
    } else {
      ip += sizeof(__m128i);
      ref += sizeof(__m128i);
    }
  }
  while (ip < (ip_bound - (sizeof(__m256i)))) {
    __m256i value, value2, cmp;
    /* Broadcast the value for every byte in a 256-bit register */
    memset(&value, x, sizeof(__m256i));
    value2 = _mm256_loadu_si256((__m256i *)ref);
    cmp = _mm256_cmpeq_epi64(value, value2);
    if (_mm256_movemask_epi8(cmp) != 0xFFFFFFFF) {
      /* Return the byte that starts to differ */
      while (*ref++ == x) ip++;
      return ip;
    }
    else {
      ip += sizeof(__m256i);
      ref += sizeof(__m256i);
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == x)) ip++;
  return ip;
}
#endif


/* Return the byte that starts to differ */
static uint8_t *get_match(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
#if !defined(BLOSC_STRICT_ALIGN)
  while (ip < (ip_bound - sizeof(int64_t))) {
    if (*(int64_t*)ref != *(int64_t*)ip) {
      /* Return the byte that starts to differ */
      while (*ref++ == *ip++) {}
      return ip;
    }
    else {
      ip += sizeof(int64_t);
      ref += sizeof(int64_t);
    }
  }
#endif
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == *ip++)) {}
  return ip;
}


#if defined(__SSE2__)
static uint8_t *get_match_16(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {
  __m128i value, value2, cmp;

  if (ip < (ip_bound - sizeof(int64_t))) {
    if (*(int64_t *) ref != *(int64_t *) ip) {
      /* Return the byte that starts to differ */
      while (*ref++ == *ip++) {}
      return ip;
    } else {
      ip += sizeof(int64_t);
      ref += sizeof(int64_t);
    }
  }
  while (ip < (ip_bound - sizeof(__m128i))) {
    value = _mm_loadu_si128((__m128i *) ip);
    value2 = _mm_loadu_si128((__m128i *) ref);
    cmp = _mm_cmpeq_epi32(value, value2);
    if (_mm_movemask_epi8(cmp) != 0xFFFF) {
      /* Return the byte that starts to differ */
      return get_match(ip, ip_bound, ref);
    }
    else {
      ip += sizeof(__m128i);
      ref += sizeof(__m128i);
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == *ip++)) {}
  return ip;
}
#endif


#if defined(__AVX2__)
static uint8_t *get_match_32(uint8_t *ip, const uint8_t *ip_bound, const uint8_t *ref) {

  if (ip < (ip_bound - sizeof(int64_t))) {
    if (*(int64_t *) ref != *(int64_t *) ip) {
      /* Return the byte that starts to differ */
      while (*ref++ == *ip++) {}
      return ip;
    } else {
      ip += sizeof(int64_t);
      ref += sizeof(int64_t);
    }
  }
  if (ip < (ip_bound - sizeof(__m128i))) {
    __m128i value, value2, cmp;
    value = _mm_loadu_si128((__m128i *) ip);
    value2 = _mm_loadu_si128((__m128i *) ref);
    cmp = _mm_cmpeq_epi32(value, value2);
    if (_mm_movemask_epi8(cmp) != 0xFFFF) {
      /* Return the byte that starts to differ */
      return get_match_16(ip, ip_bound, ref);
    }
    else {
      ip += sizeof(__m128i);
      ref += sizeof(__m128i);
    }
  }
  while (ip < (ip_bound - sizeof(__m256i))) {
    __m256i value, value2, cmp;
    value = _mm256_loadu_si256((__m256i *) ip);
    value2 = _mm256_loadu_si256((__m256i *)ref);
    cmp = _mm256_cmpeq_epi64(value, value2);
    if (_mm256_movemask_epi8(cmp) != 0xFFFFFFFF) {
      /* Return the byte that starts to differ */
      while (*ref++ == *ip++) {}
      return ip;
    }
    else {
      ip += sizeof(__m256i);
      ref += sizeof(__m256i);
    }
  }
  /* Look into the remainder */
  while ((ip < ip_bound) && (*ref++ == *ip++)) {}
  return ip;
}
#endif


int blosclz_compress(const int opt_level, const void* input, int length,
                     void* output, int maxout, int shuffle) {
  uint8_t* ip = (uint8_t*)input;
  uint8_t* ibase = (uint8_t*)input;
  uint8_t* ip_bound = ip + length - IP_BOUNDARY;
  uint8_t* ip_limit = ip + length - 12;
  uint8_t* op = (uint8_t*)output;
  uint8_t* op_limit;
  uint16_t htab[1U << (uint8_t)HASH_LOG];
  int32_t hval;
  uint8_t copy;
  uint32_t nmax_copies = 0;
  unsigned i;
  uint8_t hashlog_[10] = {0, HASH_LOG - 4, HASH_LOG - 4, HASH_LOG - 3 , HASH_LOG - 2,
                          HASH_LOG - 1, HASH_LOG, HASH_LOG, HASH_LOG, HASH_LOG};
  uint8_t hashlog = hashlog_[opt_level];
  // The maximum amount of consecutive MAX_COPY copies before giving up
  // 0 means something very close to RLE
  uint8_t max_nmax_copies_[10] = {255U, 0U, 8U, 8U, 16U, 32U, 32U, 32U, 32U, 64U};  // 255 never used
  uint8_t max_nmax_copies = max_nmax_copies_[opt_level];
  double maxlength_[10] = {-1, .1, .2, .3, .4, .6, .9, .95, 1.0, 1.0};
  int32_t maxlength = (int32_t)(length * maxlength_[opt_level]);

  if (maxlength > (int32_t)maxout) {
    maxlength = (int32_t)maxout;
  }
  op_limit = op + maxlength;

  // Initialize the hash table to distances of 0
  for (i = 0; i < (1U << hashlog); i++) {
    htab[i] = 0;
  }

  /* output buffer cannot be less than 66 bytes or we can get into trouble */
  if (BLOSCLZ_UNEXPECT_CONDITIONAL(maxout < 66 || length < 4)) {
    return 0;
  }

  /* we start with literal copy */
  copy = 2;
  *op++ = MAX_COPY - 1;
  *op++ = *ip++;
  *op++ = *ip++;

  /* main loop */
  while (BLOSCLZ_EXPECT_CONDITIONAL(ip < ip_limit)) {
    const uint8_t* ref;
    uint32_t distance;
    uint32_t len = 3;         /* minimum match length */
    uint8_t* anchor = ip;    /* comparison starting-point */

    /* check for a run */
    if (ip[0] == ip[-1] && BLOSCLZ_READU16(ip - 1) == BLOSCLZ_READU16(ip + 1)) {
      distance = 1;
      ref = anchor - 1 + 3;
      goto match;
    }

    /* find potential match */
    HASH_FUNCTION(hval, ip, hashlog);
    ref = ibase + htab[hval];

    /* calculate distance to the match */
    distance = (int32_t)(anchor - ref);

    /* update hash table if necessary */
    /* not exactly sure why masking the distance works best, but this is what the experiments say */
    if (!shuffle || (distance & (MAX_COPY - 1)) == 0) {
      htab[hval] = (uint16_t) (anchor - ibase);
    }

    if (distance == 0 || (distance >= MAX_FARDISTANCE)) {
      LITERAL(ip, op, op_limit, anchor, copy)
    }

    /* is this a match? check the first 4 bytes */
    if (BLOSCLZ_READU32(ref) == BLOSCLZ_READU32(ip)) {
      len = 4;
      ref += 4;
    }
      /* check just the first 3 bytes */
    else if (*ref++ != *ip++ || *ref++ != *ip++ || *ref++ != *ip) {
      /* no luck, copy as a literal */
      LITERAL(ip, op, op_limit, anchor, copy)
    }

    match:

    /* last matched byte */
    ip = anchor + len;

    /* distance is biased */
    distance--;

    if (!distance) {
      /* zero distance means a run */
#if defined(__AVX2__)
      ip = get_run_32(ip, ip_bound, ref);
#elif defined(__SSE2__)
      ip = get_run_16(ip, ip_bound, ref);
#else
      ip = get_run(ip, ip_bound, ref);
#endif
    }
    else {
#if defined(__AVX2__)
      ip = get_match_32(ip, ip_bound + IP_BOUNDARY, ref);
#elif defined(__SSE2__)
      ip = get_match_16(ip, ip_bound + IP_BOUNDARY, ref);
#else
      ip = get_match(ip, ip_bound + IP_BOUNDARY, ref);
#endif
    }

    /* if we have copied something, adjust the copy count */
    if (copy)
      /* copy is biased, '0' means 1 byte copy */
      *(op - copy - 1) = (uint8_t)(copy - 1);
    else
      /* back, to overwrite the copy count */
      op--;

    /* reset literal counter */
    copy = 0;

    /* length is biased, '1' means a match of 3 bytes */
    ip -= 3;
    len = (int32_t)(ip - anchor);

    /* check that we have space enough to encode the match for all the cases */
    if (BLOSCLZ_UNEXPECT_CONDITIONAL(op + (len / 255) + 6 > op_limit)) goto out;

    /* encode the match */
    if (distance < MAX_DISTANCE) {
      if (len < 7U) {
        *op++ = (uint8_t)((len << 5U) + (distance >> 8U));
        *op++ = (uint8_t)((distance & 255U));
      }
      else {
        *op++ = (uint8_t)((7U << 5U) + (distance >> 8U));
        for (len -= 7U; len >= 255U; len -= 255U)
          *op++ = 255U;
        *op++ = (uint8_t)len;
        *op++ = (uint8_t)((distance & 255U));
      }
    }
    else {
      /* far away, but not yet in the another galaxy... */
      if (len < 7U) {
        distance -= MAX_DISTANCE;
        *op++ = (uint8_t)((len << 5U) + 31U);
        *op++ = 255U;
        *op++ = (uint8_t)(distance >> 8U);
        *op++ = (uint8_t)(distance & 255U);
      }
      else {
        distance -= MAX_DISTANCE;
        *op++ = (7U << 5U) + 31U;
        for (len -= 7U; len >= 255U; len -= 255U)
          *op++ = 255U;
        *op++ = (uint8_t)len;
        *op++ = 255U;
        *op++ = (uint8_t)(distance >> 8U);
        *op++ = (uint8_t)(distance & 255U);
      }
    }

    /* update the hash at match boundary */
    if (ip < ip_limit) {
      HASH_FUNCTION(hval, ip, hashlog);
      htab[hval] = (uint16_t)(ip - ibase);
    }
    ip += 2;
    /* assuming literal copy */
    *op++ = MAX_COPY - 1;

    // reset the number of max copies
    nmax_copies = 0;
  }

  /* left-over as literal copy */
  ip_bound++;
  while (ip <= ip_bound) {
    if (BLOSCLZ_UNEXPECT_CONDITIONAL(op + 2 > op_limit)) goto out;
    *op++ = *ip++;
    copy++;
    if (copy == MAX_COPY) {
      copy = 0;
      *op++ = MAX_COPY - 1;
    }
  }

  /* if we have copied something, adjust the copy length */
  if (copy)
    *(op - copy - 1) = (uint8_t)(copy - 1);
  else
    op--;

  /* marker for blosclz */
  *(uint8_t*)output |= (1U << 5U);

  return (int)(op - (uint8_t*)output);

  out:
  return 0;

}

// See https://habr.com/en/company/yandex/blog/457612/
#ifdef __AVX2__

#if defined(_MSC_VER)
#define ALIGNED_(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED_(x) __attribute__ ((aligned(x)))
#endif
#endif
#define ALIGNED_TYPE_(t, x) t ALIGNED_(x)

static unsigned char* copy_match_16(unsigned char *op, const unsigned char *match, int32_t len)
{
  size_t offset = op - match;
  while (len >= 16) {

    static const ALIGNED_TYPE_(uint8_t, 16) masks[] =
      {
        0,  1,  2,  1,  4,  1,  4,  2,  8,  7,  6,  5,  4,  3,  2,  1, // offset = 0, not used as mask, but for shift
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // offset = 1
        0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,
        0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,  1,  2,  0,
        0,  1,  2,  3,  0,  1,  2,  3,  0,  1,  2,  3,  0,  1,  2,  3,
        0,  1,  2,  3,  4,  0,  1,  2,  3,  4,  0,  1,  2,  3,  4,  0,
        0,  1,  2,  3,  4,  5,  0,  1,  2,  3,  4,  5,  0,  1,  2,  3,
        0,  1,  2,  3,  4,  5,  6,  0,  1,  2,  3,  4,  5,  6,  0,  1,
        0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  1,  2,  3,  4,  5,  6,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  1,  2,  3,  4,  5,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  0,  1,  2,  3,  4,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0,  1,  2,  3,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  0,  1,  2,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,  0,  1,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  0,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  15, // offset = 16
      };

    _mm_storeu_si128((__m128i *)(op),
                     _mm_shuffle_epi8(_mm_loadu_si128((const __m128i *)(match)),
                                      _mm_load_si128((const __m128i *)(masks) + offset)));

    match += masks[offset];

    op += 16;
    len -= 16;
  }
  // Deal with remainders
  for (; len > 0; len--) {
    *op++ = *match++;
  }
  return op;
}
#endif


/**
  Define blosc_decompress and blosc_decompress_unsafe.
 */
#define BLOSCLZ_SAFE
#include "blosclz_impl.inc"
#undef BLOSCLZ_SAFE
#define blosclz_decompress blosclz_decompress_unsafe
#include "blosclz_impl.inc"
#undef blosclz_decompress
