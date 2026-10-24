#!/bin/sh
# Builds lib/libz.a and installs the public zlib headers into include/.
# Runs inside a private copy of vendor/ with CC, AR, CFLAGS, and DOOF_JOBS
# supplied by the Doof compiler.
set -eu

library=libz.a
defines=""
includes="-Izlib"
jobs=${DOOF_JOBS:-1}
sources="zlib/adler32.c zlib/crc32.c zlib/deflate.c zlib/inffast.c zlib/inflate.c zlib/inftrees.c zlib/trees.c zlib/zutil.c"

install_headers() { cp zlib/zlib.h zlib/zconf.h include/; }

mkdir -p include lib obj
install_headers

pids=''
active=0
status=0
objects=''
for source in $sources; do
  object="obj/$(echo "$source" | sed 's|/|_|g; s|\.c$|.o|')"
  objects="$objects $object"
  # shellcheck disable=SC2086
  $CC $CFLAGS $defines $includes -c "$source" -o "$object" &
  pids="$pids $!"
  active=$((active + 1))
  if [ "$active" -ge "$jobs" ]; then
    for pid in $pids; do wait "$pid" || status=1; done
    pids=''
    active=0
  fi
done
for pid in $pids; do wait "$pid" || status=1; done
[ "$status" -eq 0 ]

rm -f "lib/$library"
# shellcheck disable=SC2086
$AR rcs "lib/$library" $objects
