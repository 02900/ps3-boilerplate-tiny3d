# PS3 3D Test

A minimal **real-3D** sandbox for PS3 homebrew on the **PSL1GHT** SDK — a perspective
camera and a spinning colour cube, drawn with the depth buffer. It exists to exercise
Tiny3D's **3D** pipeline (`tiny3d_Project3D`, `MatrixProjPerspective`,
`tiny3d_SetMatrixModelView`, depth test), which the sibling ports
([ki-blast-arena](https://github.com/02900/ki-blast-arena),
[ps3-mega-mario](https://github.com/02900/ps3-mega-mario)) don't — they only use Tiny3D's
2D mode (`tiny3d_Project2D`).

It inherits the standard scaffold from
[02900/ps3-homebrew-template](https://github.com/02900/ps3-homebrew-template) (Dockerized
toolchain, Makefile, CI, PKG packaging) and vendors the shared
[`ps3-homebrew-skills`](https://github.com/02900/ps3-homebrew-skills) as a submodule.

> ## 🧪 Status: scaffold + spinning cube
> One frame loop: clear → 3D pass (perspective cube) → 2D HUD → flip. Build is green in the
> toolchain image; **on-hardware behaviour is unverified** until someone runs it on a PS3 / RPCS3.

## What it does

```
main loop
├─ tiny3d_Clear(SKY, TINY3D_CLEAR_ALL)
├─ tiny3d_Project3D()
│  ├─ MatrixProjPerspective(60°, 16:9, 0.1, 1000)  → tiny3d_SetProjectionMatrix
│  ├─ rotate(pitch,yaw) · translate(0,0,-4.5)       → tiny3d_SetMatrixModelView
│  └─ 6 quads, one colour per face (depth-tested)
├─ tiny3d_Project2D() → ttf HUD
└─ tiny3d_Flip()
```

**Controls** (pad on port 0): left stick / D-pad orbit the cube · **✕** holds the auto-spin ·
**Start** exits to the XMB.

## Building

You need the PSL1GHT toolchain — easiest via the prebuilt Docker image (no local install):

```bash
docker run --rm -v "$PWD":/src -w /src ghcr.io/02900/ps3-toolchain make        # -> src.self
docker run --rm -v "$PWD":/src -w /src ghcr.io/02900/ps3-toolchain make pkg    # -> src.pkg (XMB)
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
ps3-3d-test/
├── .github/workflows/   # CI: build (toolchain image) + docs link lint
├── source/              # main.c (3D cube) + ttf_render.c (2D text overlay)
├── include/             # ttf_render.h
├── data/                # bin2o-embedded assets (empty — the cube is code-generated)
├── pkgfiles/            # PKG payload: ICON0.PNG
├── .claude/skills/      # Submodule: ps3-homebrew patterns, as Claude skills
├── docs/api/            # Per-library API notes (TINY3D, YA2D, …)
├── scripts/             # Dockerized build.sh / deploy.sh wrappers
├── Makefile             # PSL1GHT build
├── sfo.xml              # App metadata (TITLE_ID: PS33DTEST)
└── README.md
```

## Toolchain & libraries

Built against the toolchain image's libraries: **Tiny3D** (3D/RSX), **ya2d** (2D / textures),
**FreeType** (TTF text), plus the PSL1GHT pad/sysutil APIs. Clay is intentionally **not** wired in
here (this is a rendering sandbox, not a UI app); re-add it like the siblings if a menu is needed.

## Patterns & gotchas

Reusable PS3/PSL1GHT conventions live in the shared
[`.claude/skills/ps3-homebrew/`](https://github.com/02900/ps3-homebrew-skills) submodule, vendored
once and used as Claude Code skills so every port stays in sync. Run `git submodule update --init`
to fetch it. (`docs/PATTERNS.md` is now just a pointer there.) The **3D** path this repo exercises
is new territory for those skills — findings here should flow back into the rendering skill.

## Credits

- Scaffold & toolchain: [02900/ps3-toolchain](https://github.com/02900/ps3-toolchain),
  [02900/ps3-homebrew-template](https://github.com/02900/ps3-homebrew-template).

## License

MIT.
