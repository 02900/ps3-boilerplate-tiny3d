# data/

Embedded assets go here. The Makefile runs `bin2o` over every `*.png`, `*.jpg`,
`*.bin`, `*.mod`, `*.s3m` in this folder and links it into the executable as
`<name>_<ext>` + `<name>_<ext>_size` symbols.

This 3D test starts with no embedded assets — the spinning cube is generated in
code. Drop a texture here and reference it from `source/main.c` to texture it.
