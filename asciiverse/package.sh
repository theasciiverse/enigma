#!/bin/sh
# Packs Enigma into its archive for av: dist/enigma-<target>.tar.gz (.zip
# for Windows targets), with the program, the levels in levels/, our
# README, Tatham's README and CHANGELOG, the licence and PATCHES.md.
#   asciiverse/package.sh x86_64-unknown-linux-musl build/linux/enigma
set -eu
cd "$(dirname "$0")/.."
target=$1
binary=$2
stage=stage/enigma
rm -rf "$stage" && mkdir -p "$stage/levels" dist
cp "$binary" "$stage/"
cp levels/*.set levels/*.level "$stage/levels/"
cp asciiverse/README.md "$stage/README.md"
cp README "$stage/README-tatham.txt"
cp CHANGELOG LICENCE PATCHES.md "$stage/"
case $target in
  *windows*)
    rm -f "dist/enigma-$target.zip"
    (cd "$stage" && zip -q -r "../../dist/enigma-$target.zip" .) ;;
  *)
    tar --owner=0 --group=0 --numeric-owner -czf "dist/enigma-$target.tar.gz" -C "$stage" . 2>/dev/null \
      || tar -czf "dist/enigma-$target.tar.gz" -C "$stage" . ;;
esac
ls -l dist
