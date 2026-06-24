# Patterns & Gotchas — moved

The PS3 / PSL1GHT homebrew patterns and gotchas are maintained in **one shared place** so they
stay in sync across ports (instead of each repo carrying its own drifting copy):

**→ `.claude/skills/ps3-homebrew/`** — the [`02900/ps3-homebrew-skills`](https://github.com/02900/ps3-homebrew-skills)
submodule, which is also a set of **Claude Code skills**.

```bash
git submodule update --init        # fetch the patterns/skills
```

Then read its [`README.md`](../.claude/skills/ps3-homebrew/README.md) for the full index, or open
any skill under `.claude/skills/ps3-homebrew/skills/` (build, input, rendering, game-loop, audio,
porting). Claude Code auto-loads them when relevant, or invoke one with `/ps3-homebrew:<name>`.

> Note: the **rendering** skill currently documents Tiny3D's **2D** mode (`tiny3d_Project2D`) used
> by the sibling ports. This repo is where the **3D** path (`tiny3d_Project3D`, `MatrixProjPerspective`,
> `tiny3d_SetMatrixModelView`, depth buffer) gets exercised — promising findings here should flow
> back into that skill.
