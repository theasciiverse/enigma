"""Plays Enigma for the catalog preview (record-preview.sh).

Waits for asciiverse's client on the 100x30 tmux session (socket avrec),
shows the menu, starts level 1 and plays the moves in the file given
(from asciiverse/solve.c) at a watchable pace, saves the position in
slot 1, goes back to the menu where the save shows, and quits.
    python3 -I drive.py <moves file>
"""
import subprocess
import sys
import time

MOVES = open(sys.argv[1]).read().strip()
PACE = 0.17


def tmux(*args):
    return subprocess.run(["tmux", "-L", "avrec", *args], capture_output=True,
                          text=True).stdout


def key(k, wait=PACE):
    tmux("send-keys", "-t", "rec", k)
    time.sleep(wait)


for _ in range(100):
    if tmux("list-clients").strip():
        break
    time.sleep(0.1)
time.sleep(2.5)                  # the menu: levels, save slots, the banner
key("Enter", 1.5)                # level 1, Darkness Falls
for m in MOVES:
    key(m)
time.sleep(1.0)
key("s", 1.2)                    # save the position...
key("y", 0.8)                    # ...in slot 1
key("q", 2.5)                    # back to the menu, the save in its slot
key("q", 0.5)
