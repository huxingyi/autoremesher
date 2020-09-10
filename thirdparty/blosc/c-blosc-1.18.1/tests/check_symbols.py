"""Checks the exported symbols."""

from __future__ import print_function, unicode_literals

import argparse
import re
import subprocess
import sys


def check_symbols(path):
  out = subprocess.check_output(['nm', '--defined-only', '-g', path]).decode()

  allowed_symbols = [
      '_?LZ4_.*',
      '_?ZSTD_.*',
      'blosclz_.*',
      '_?POOL_.*',
      'snappy_.*',
      'blosc_.*',
      '_?HIST_.*',
      '_?HUF_.*',
      '_?ZSTDMT_.*',
      '_?FSE_.*',
      '_?XXH.*',
      'gz.*',
      'deflate.*',
      'inflate.*',
      'uncompress',
      'compress',
      'compress2',
      'compressBound',
      'adler32.*',
      'crc32.*',
      'get_crc_table',
      'zcalloc',
      'zcfree',
      'zError',
      'zlib.*',
      'g_debuglevel',
      'z_errmsg',
      'g_ZSTD_.*',
      '_?ERR_.*',
      '_Z[A-Z0-9]+snappy.*',
      '_[_a-z].*',
      '.*__gxx_personality.*',
  ]
  allowed_symbols_pattern = '^' + '|'.join(
      '(?:%s)' % x for x in allowed_symbols) + '$'
  filename = None
  failed = False
  for line in out.split('\n'):
    line = line.strip()
    if not line:
      continue
    if line.endswith(':'):
      filename = line[:-1]
    else:
      addr, symbol_type, name = line.split(' ')
      del addr
      del symbol_type
      if re.match(allowed_symbols_pattern, name):
        continue
      print('ERROR: Found unexpected symbol in %s: %s' % (filename, name))
      failed = True
  return not failed


def main():
  ap = argparse.ArgumentParser()
  ap.add_argument('library', type=str)
  args = ap.parse_args()
  if not check_symbols(args.library):
    sys.exit(1)
  else:
    sys.exit(0)


if __name__ == '__main__':
  main()
