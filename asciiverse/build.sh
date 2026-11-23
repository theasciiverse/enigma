#!/bin/sh
# Builds Enigma for the Asciiverse catalog. Leaves the binary in
# build/<platform>/enigma (enigma.exe on Windows).
#   Linux:   docker run --rm -v "$PWD":/src -w /src alpine:3.22 sh asciiverse/build.sh
#            (fully static against Alpine's ncurses; reads the system's terminfo)
#   macOS:   sh asciiverse/build.sh  (links the system's ncurses)
#   Windows: sh asciiverse/build.sh  in an MSYS2 UCRT64 or CLANGARM64 shell
#            with pdcurses installed (linked statically)
#
# The seven sources are compiled directly rather than through Tatham's
# configure and Makefile: those fix the level and save folders at build
# time (-DLEVELDIR, -DSAVEDIR), and without them Enigma finds its levels
# beside the program and keeps saves in the player's state folder
# (settings.h). The 2004 configure itself still works with
# CC="cc -std=gnu89" (its test programs use implicit int). -std=gnu89
# here too: the sources are C89.
#
# Cross builds set PLATFORM (linux, macos or windows), CC, CFLAGS, LIBS
# and STRIP, e.g. a static Linux build with musl-gcc against a static
# ncursesw, or Windows with x86_64-w64-mingw32-gcc against PDCurses
# (asciiverse/release-local.sh).
set -eu
cd "$(dirname "$0")/.."
[ -n "${PLATFORM:-}" ] && platform=$PLATFORM || case "$(uname -s)" in
  Linux)  platform=linux ;;
  Darwin) platform=macos ;;
  MINGW*|MSYS*|UCRT*|CLANG*) platform=windows ;;
  *) echo "unknown platform $(uname -s)" >&2; exit 1 ;;
esac

if [ -f /etc/alpine-release ] && [ -z "${PLATFORM:-}" ]; then
  apk add --no-cache build-base ncurses-dev ncurses-static >/dev/null
fi

out=build/$platform
mkdir -p "$out"
CC=${CC:-cc}
CFLAGS=${CFLAGS:--O2}
case $platform in
  linux)
    if [ -f /etc/alpine-release ]; then libs="-static -lncursesw"; else libs="-lncurses"; fi
    curses='-DCURSES_HDR=<ncurses.h>'; exe=enigma ;;
  macos)   libs="-lncurses"; curses='-DCURSES_HDR=<ncurses.h>'; exe=enigma ;;
  windows) libs="-static -lpdcurses -ladvapi32"; curses='-DCURSES_HDR=<curses.h>'; exe=enigma.exe ;;
esac
libs=${LIBS:-$libs}
$CC -std=gnu89 $CFLAGS "$curses" -DHAS_ATTR_T -I. -o "$out/$exe" \
  main.c screen.c engine.c memory.c levelfile.c misc.c savefile.c $libs
${STRIP:-strip} "$out/$exe"
