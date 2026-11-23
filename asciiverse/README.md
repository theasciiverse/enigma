# Enigma

**Darkness falls. Take every piece of gold, and don't look up.**

The first level is called Darkness Falls, and what falls is arrows.
Every `v` on the board drops the moment the space beneath it opens, and
keeps on dropping, straight through soft earth, until something stops
it. If that something is you, the level is over. Every `>` and `<`
slides the same way sideways the instant it can, and every `^` rises.
You are the `@`. You dig through earth, push arrows, sacks and bombs
aside, and collect the `$`: sixty-one pieces of it on that first level
alone. The exit, `E`, only opens when the last piece is yours.

So every step is a question. Dig out the earth under that column of
arrows and they come down, on you if you are standing below. Pull a
piece of gold from beneath a pile and the pile follows. Push an arrow
off a ledge and it drops out of your way; push it the wrong way and you
have walled off a corner of the board for good. Bombs move like arrows and blow up when an arrow hits them,
taking what lies on either side with them. Sacks slide when you push them
until they hit something. Some levels give you a second self, an `O`:
press `x` to swap between them, and if one is crushed the other carries
on. Teleporters, `#`, come in pairs.

Twelve levels, from Darkness Falls through Lateral Thinking, Explosively
Simple and House of Cards to Devil's Despair and Shrouded Glory, each
opening when you finish the one before. Ten save slots keep positions
to come back to, and you can write down the moves of a solution, load
someone else's and step through it forwards and backwards like a film.

## Playing

Run `enigma` in a terminal of at least 60x22.

- **Arrow keys** or **h j k l**: move, dig, push
- **x**: swap to your other self, on levels that have one
- **s**: save the position (a slot from 0 to 9, then **y**)
- **r**: restore a saved position, also after you have died
- **w**: write the moves so far to a file
- **m**: load a move file and watch it: **Space** or **f** forward,
  **b** back, **+** and **-** or **Right** and **Left** ten moves,
  **<** and **>** start and end, **q** to play all of it and carry on
  from where it ends
- **q**: leave the level

On the menu: **Up** and **Down** choose a level, **Enter** plays it,
**1** to **0** choose a save slot, **r** resumes it, **Backspace**
deletes it, **q** quits. `enigma <set>` plays another level set from
the `levels` folder.

Saves and progress live in `$XDG_STATE_HOME/enigma`
(`~/.local/state/enigma`; `%LOCALAPPDATA%\enigma` on Windows).

## Credits

- Enigma: Simon Tatham,
  https://www.chiark.greenend.org.uk/~sgtatham/enigma/ (MIT, `LICENCE`)
- The rules and the twelve original levels: a friend of Tatham's, who
  wishes to remain anonymous and gave the levels to the public domain
  (`README-tatham.txt`)
- Packaged for Asciiverse: changes listed in `PATCHES.md`

Chroma, also in the Asciiverse catalog, plays these twelve levels as
its third set.

## Building

`sh asciiverse/build.sh` leaves the program in `build/<platform>/`;
static Linux builds run it in Alpine (`docker run --rm -v "$PWD":/src -w
/src alpine:3.22 sh asciiverse/build.sh`), Windows in an MSYS2 shell
with PDCurses. `asciiverse/package.sh` makes the av archive and
`.github/workflows/asciiverse-release.yml` builds every release. System
packages call the program `tatham-enigma`.
