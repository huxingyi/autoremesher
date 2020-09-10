/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Test that library can be used after fork().

  Creation date: 2018-10-34
  Author: Alex Ford <a.sewall.ford@gmail.com>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "test_common.h"


int clevel = 3;
int doshuffle = 1;
size_t typesize = 4;
size_t size = 5*MB;
#define BUFFER_ALIGN_SIZE   8

void *src, *dest, *dest2;
int nbytes, cbytes;

int tests_run = 0;

static const char *test_forksafe(void) {
  /* Compress the input data and store it in dest. */
  blosc_set_nthreads(4);
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src, dest, size + BLOSC_MAX_OVERHEAD);

  pid_t newpid = fork();
  if (newpid == 0) {
    nbytes = blosc_decompress(dest, dest2, size);
    mu_assert("ERROR: Result buffer did not match expected size in child.", nbytes == size);
    exit(0);
  }

  int success = 0;
  int status = 0;
  float sec = 0;
  while (sec < 1) {
    if(waitpid(newpid, &status, WNOHANG) != 0) {
      success = 1;
      break;
    }

    usleep(5000);
    sec += 5000 * 1e-6;
  }

  if(!success) {
    kill(newpid, SIGKILL);
    waitpid(newpid, &status, 0);
  }

  mu_assert("ERROR: Child deadlocked post-fork.", success == 1);
  mu_assert("ERROR: Child crashed.", status == 0);

  return 0;
}


static const char *all_tests(void) {
  mu_run_test(test_forksafe);

  return 0;
}

int main(int argc, char **argv) {
  const char *result;

  printf("STARTING TESTS for %s", argv[0]);

  blosc_init();

  /* Initialize buffers */
  src = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  dest = blosc_test_malloc(BUFFER_ALIGN_SIZE, size + BLOSC_MAX_OVERHEAD);
  dest2 = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);

  /* Fill the input data buffer with random values. */
  blosc_test_fill_random(src, size);

  /* Run all the suite */
  result = all_tests();
  if (result != 0) {
    printf(" (%s)\n", result);
  }
  else {
    printf(" ALL TESTS PASSED");
  }
  printf("\tTests run: %d\n", tests_run);

  blosc_test_free(src);
  blosc_test_free(dest);
  blosc_test_free(dest2);

  blosc_destroy();

  return result != 0;
}
