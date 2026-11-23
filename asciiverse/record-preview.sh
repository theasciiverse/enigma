#!/bin/sh
# Records the catalog preview, asciiverse/enigma.cast (hub/RECORDING.md):
# the menu, then level 1, Darkness Falls, played with the best line
# asciiverse/solve.c finds on Enigma's own engine (23 of the 61 pieces of
# gold, the player alive), a save to slot 1 and back to the menu. Enigma
# centres itself, so it runs straight in the 100x30 tmux session
# asciinema records (own socket, no user config). Needs asciinema 3, tmux,
# a C compiler and the built binary.
#   asciiverse/record-preview.sh build/linux/enigma
set -eu
binary=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
here=$(cd "$(dirname "$0")" && pwd)
repo=$(dirname "$here")
out=$here/enigma.cast
home=$(mktemp -d)
trap 'tmux -L avrec kill-server 2>/dev/null; rm -rf "$home"' EXIT
mkdir -p "$home/bin/levels" "$home/solver/levels"
cp "$binary" "$home/bin/enigma"
cp "$repo"/levels/* "$home/bin/levels/"
cp "$repo"/levels/* "$home/solver/levels/"
(cd "$repo" && ${CC:-cc} -std=gnu89 -O2 '-DCURSES_HDR=<ncurses.h>' -I. -o "$home/solver/solve" \
  asciiverse/solve.c engine.c levelfile.c memory.c misc.c)
SOLVE_PARTIAL=1 "$home/solver/solve" 1 > "$home/moves" || true
[ -s "$home/moves" ] || { echo "no moves" >&2; exit 1; }
cd "$home"  # nothing in the recording names a local path
export HOME="$home" XDG_STATE_HOME="$home/state" PATH="$home/bin:$PATH" \
  USER=player LOGNAME=player TERM=xterm-256color COLORTERM=truecolor LANG=C.UTF-8
tmux -L avrec -f /dev/null new-session -d -s rec -x 100 -y 30 "enigma" \; set -g status off
python3 -I "$here/drive.py" "$home/moves" &
drive=$!
asciinema rec --headless --overwrite -q \
  --window-size 100x30 --idle-time-limit 2 \
  --capture-env "TERM,COLORTERM,LANG" --title "Enigma" \
  --command "tmux -L avrec attach -t rec" "$out"
wait "$drive"
