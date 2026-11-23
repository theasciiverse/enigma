# Changes from upstream

This is the Asciiverse packaging of
[Enigma](https://www.chiark.greenend.org.uk/~sgtatham/enigma/), Simon
Tatham's curses puzzle game (MIT, `LICENCE`). History here starts from
his last release tarball, enigma-1.04.tar.gz of 2004-02-29, as one commit
by him, tagged `1.04`. His own repository
(https://git.tartarus.org/simon/enigma.git, history from 2000) is the
`upstream` remote; its only code change after 1.04, "Missing #includes"
(2008), is cherry-picked on top with his authorship. The rest of it
after 1.04 is his release tooling (`Buildscr`, `.gitignore`). The
`asciiverse` branch carries our changes below, one commit each.

## The game

Unchanged: same rules, twelve levels, keys, menus and texts. It needs a
terminal of at least 60x22 (the menu; a level is 36x20 plus its title
and status lines).

## Code

- **Folders** (`settings.h`, `misc.c`, `main.c`): the levels came from
  `LEVELDIR` and everyone's saves and progress went to `SAVEDIR`, both
  fixed at build time by configure (`$prefix/share/enigma/` and a setgid
  `$prefix/var/enigma/`). Built without them, Enigma now finds them when
  it starts: the levels in `levels/` beside the program (the av archive)
  or in `../share/tatham-enigma/levels/` (the system packages), via
  `/proc/self/exe`, `_NSGetExecutablePath` or `GetModuleFileName`; saves
  and progress in `$XDG_STATE_HOME/enigma/` (else `~/.local/state/enigma/`;
  `%LOCALAPPDATA%\enigma\` on Windows), made if missing. Enigma never had
  a per-user folder such as `~/.enigma`, so there is nothing to carry
  over. A build with `-DLEVELDIR` and `-DSAVEDIR` behaves as before.
- **No crash without a password entry** (`misc.c`): `get_user()` used
  `getpwuid()`'s answer unchecked, so under a user id missing from
  `/etc/passwd` (a container, say) Enigma crashed before the menu. It
  falls back to `$USER`, then to "player". The name only goes into save
  file names.
- **Follows a resize** (`screen.c`): every screen already laid itself
  out in the middle of the terminal when drawn, but a resize was ignored
  until the next move. On `KEY_RESIZE` the play and movie screens now
  draw the last level screen again, and the menu works out its place
  again, so all three move back to the middle at once.
- **Windows** (`main.c`): `chdir()` is declared through `<direct.h>`;
  the call was an implicit declaration, an error in C99 and later.
  `GetUserName()` is given the `DWORD` it takes instead of an `int`,
  which Clang (the ARM64 Windows build) refuses.
  Gurusamy Sarathy's Windows support (2001, PDCurses) is otherwise
  Tatham's own.

## Build settings

`asciiverse/build.sh` compiles the seven sources directly with
`-std=gnu89` (they are C89), `-DCURSES_HDR` and `-DHAS_ATTR_T`, instead of
configure and the Makefile, which would fix the folders. The 2004
configure itself still works when given `CC="cc -std=gnu89"`; its test
programs use implicit int, which GCC 14 rejects. Linux links ncurses
statically in Alpine, so the binary needs nothing installed and reads the
system's terminfo; macOS links the system ncurses; Windows links PDCurses
statically from MSYS2, whose package (PDCursesMod) keeps `curses.h` in
`include/pdcurses`, is built wide and UTF-8 (`-DPDC_WIDE
-DPDC_FORCE_UTF8`, or the link misses `endwin`) and needs `winmm`.

## Name

Debian and other distributions ship a different game as `enigma` (the
Oxyd-style one, `/usr/games/enigma`). Inside av the program is `enigma`;
the system packages are named `tatham-enigma`, with the program as
`/usr/bin/tatham-enigma` and the levels in
`/usr/share/tatham-enigma/levels/`, so both can be installed side by side.

## Packaging

- `asciiverse/build.sh` (new): the build above.
- `asciiverse/package.sh` (new): the av archive, with the program, the
  levels in `levels/`, our `README.md`, Tatham's README as
  `README-tatham.txt`, `CHANGELOG`, `LICENCE` and this file.
- `asciiverse/nfpm.yaml` (new): .deb, .rpm, .apk and Arch packages named
  `tatham-enigma`.
- `.github/workflows/asciiverse-release.yml` (new): av archives for Linux
  (static, x86-64 and ARM64), macOS (Intel and Apple silicon) and Windows
  (x86-64 and ARM64), the system packages and SHA256SUMS, on a `*-av.*`
  tag.
- `asciiverse/release-local.sh` (new): builds the static Linux x86-64
  (musl-gcc and a static ncursesw) and Windows x86-64 (mingw-w64 and
  PDCurses) archives on a Linux machine and attaches them to a tag's
  release, for when the workflow cannot run; `asciiverse/build.sh` takes
  `PLATFORM`, `CC`, `CFLAGS`, `LIBS` and `STRIP` for such cross builds.
- `asciiverse/README.md` (new): the package README.
- `asciiverse/solve.c` (new): a solver on Enigma's own engine, a beam
  search from gold to gold. Level 1 is hard: it finds a line taking 23 of
  the 61 pieces of gold with the player alive, not a full solution.
- `asciiverse/record-preview.sh`, `asciiverse/drive.py` (new): the
  catalog preview, the menu and that line played on level 1, then a save.
- `asciiverse/enigma.toml`, `asciiverse/enigma.md`, `asciiverse/enigma.cast`
  (new): the catalog manifest, store page and preview, as drafts for hub.
- `PATCHES.md` (new): this file.
