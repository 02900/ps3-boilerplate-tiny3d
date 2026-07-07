# PS3 Tiny3D Boilerplate

> **📋 GitHub template repo** — click **“Use this template”** (or
> `gh repo create my-game --template 02900/ps3-boilerplate-tiny3d`) to start a new PS3
> homebrew project from this scaffold: Dockerized toolchain, Makefile, CI, PKG packaging
> and the shared [`ps3-homebrew-skills`](https://github.com/02900/ps3-homebrew-skills)
> submodule already wired.

A minimal **starter** for PS3 homebrew on the **PSL1GHT** SDK using **Tiny3D** + **ya2d** — an
embedded PNG sprite that bounces around the screen while a synthesized MikMod tune plays. It
exercises Tiny3D's **2D** mode (`tiny3d_Project2D`) plus `ya2d` textured quads and MikMod audio,
the same 2D path the sibling ports
([ki-blast-arena](https://github.com/02900/ki-blast-arena),
[ps3-mega-mario](https://github.com/02900/ps3-mega-mario)) build on.

It inherits the standard scaffold from
[02900/ps3-homebrew-showcase](https://github.com/02900/ps3-homebrew-showcase) (Dockerized
toolchain, Makefile, CI, PKG packaging) and vendors the shared
[`ps3-homebrew-skills`](https://github.com/02900/ps3-homebrew-skills) as a submodule.

> ## Status: 2D sprite + audio starter
> Loads a PNG sprite, bounces it around the screen and plays synthesized MikMod audio. Builds
> green in the toolchain image and runs on RPCS3. *(The old 3D spinning-cube demo lives on the
> [`3d-chessboard`](https://github.com/02900/ps3-boilerplate-tiny3d/tree/3d-chessboard) branch.)*

## What it does

A minimal "hello, game": an embedded PNG sprite (`data/sprite.png` — an original placeholder)
moves across Tiny3D's 848×512 2D canvas, **bounces off the four edges** (a blip plays on each
bounce) while a short **synthesized tune** loops; the D-pad / left stick nudge it.

```c
ya2d_Texture *sprite = ya2d_loadPNGfromBuffer((void*)sprite_png, sprite_png_size);
...
tiny3d_Clear(SKY, TINY3D_CLEAR_ALL);
tiny3d_Project2D();
ya2d_drawTexture(sprite, x, y);      /* ya2d: PNG decode + textured quad */
tiny3d_Flip();
audio_play_blip();                    /* on bounce */
```

The audio is synthesized in `source/audio.c` (no asset files); the sprite is embedded via
`bin2o`. `ya2d` handles the PNG decode and the textured quad; `ttf_render.*` is still in the
scaffold if you want `/dev_flash` TrueType text.

**Controls** (pad on port 0): **D-pad / left stick** nudge the sprite · **Start** exits to the XMB.

## Building

You need the PSL1GHT toolchain — easiest via the prebuilt Docker image (no local install):

```bash
docker run --rm -v "$PWD":/src -w /src ghcr.io/02900/ps3-toolchain-tiny3d make        # -> src.self
docker run --rm -v "$PWD":/src -w /src ghcr.io/02900/ps3-toolchain-tiny3d make pkg    # -> src.pkg (XMB)
```

Or the helper wrappers (they auto-retry the toolchain's transient emulation segfaults):

```bash
./scripts/build.sh            # build
./scripts/build.sh pkg        # installable PKG
PS3_IP=192.168.1.13 ./scripts/deploy.sh   # ps3load to a console running PS3LoadX
```

> **Platform notes** — **Apple Silicon:** add `--platform linux/amd64` to every `docker run`
> (or `export DOCKER_DEFAULT_PLATFORM=linux/amd64`; the helper scripts rely on that).
> **Windows:** run from a **WSL2** shell. **Linux:** prefix with `sudo` if needed.

Outputs are named after the mount dir (`/src`): `src.elf` / `src.self` / `src.pkg`.

## Running

- **RPCS3:** *File → Install .pkg* → pick `src.pkg` → launch **PS3 3D Test**. (You can also boot
  `src.self` directly.)
- **Real PS3 (HEN/CFW):** install `src.pkg` from the XMB, or `ps3load` the `.self`.

## Project structure

```
ps3-boilerplate-tiny3d/
├── .github/workflows/   # CI: build (toolchain image) + docs link lint
├── source/              # main.c (2D sprite) + audio.c (MikMod synth) + ttf_render.c
├── include/             # ttf_render.h
├── data/                # bin2o-embedded assets (sprite.png)
├── pkgfiles/            # PKG payload: ICON0.PNG
├── .claude/skills/      # Submodule: ps3-homebrew patterns, as Claude skills
├── docs/api/            # Per-library API notes (TINY3D, YA2D, …)
├── scripts/             # Dockerized build.sh / deploy.sh wrappers
├── Makefile             # PSL1GHT build
├── sfo.xml              # App metadata (TITLE_ID: PS33DTEST)
└── README.md
```

## Toolchain & libraries

Built against the toolchain image's libraries: **Tiny3D** (RSX rendering), **ya2d** (2D / textures),
**FreeType** (TTF text), plus the PSL1GHT pad/sysutil APIs. Clay is intentionally **not** wired in
here (this is a rendering sandbox, not a UI app); re-add it like the siblings if a menu is needed.

## Patterns & gotchas

Reusable PS3/PSL1GHT conventions live in the shared
[`.claude/skills/ps3-homebrew/`](https://github.com/02900/ps3-homebrew-skills) submodule, vendored
once and used as Claude Code skills so every port stays in sync. Run `git submodule update --init`
to fetch it. (`docs/PATTERNS.md` is now just a pointer there.) The Tiny3D **2D + audio** path this
repo exercises should stay in sync with those skills — findings here flow back into the rendering skill.

## Credits

- Scaffold & toolchain: [02900/ps3-toolchain](https://github.com/02900/ps3-toolchain),
  [02900/ps3-homebrew-showcase](https://github.com/02900/ps3-homebrew-showcase).

## License

MIT.
