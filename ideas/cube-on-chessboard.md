# Cubo sobre tablero 8×8 movible

Objetivo: disponer el cubo sobre un lienzo 8×8 pintado como tablero de ajedrez,
moverlo por las casillas con las flechas (D-pad). El cubo mide de ancho lo mismo
que una celda del tablero.

## Sistema de coordenadas

- Tablero en el plano **XZ** a `y = 0`. `BOARD_N = 8`, `CELL = 1.0`.
  Abarca `[-4, +4]` en X y Z (centrado en el origen).
- Casilla `(i, j)` con `i, j ∈ 0..7`. Centro de casilla:
  `cell_center(i) = -4 + (i + 0.5) * CELL = i - 3.5`.
- Cubo: medio-lado `0.5` (ancho 1 celda). Se apoya sobre el tablero →
  `y ∈ [0, 1]`, centro en `y = 0.5`.
- Cámara: una sola model-view aplicada a todo (tablero + cubo). Convención
  Tiny3D (ver memoria `project_tiny3d-3d-matrix-convention`): vectores fila,
  cámara mira a +Z, **rotar primero, trasladar a +Z al final**.
  `mv = RotX(camPitch) * RotY(camYaw) * Translation(0, camY, camDist)`.
  `camPitch > 0` mira hacia abajo sobre el +Y (vista cenital del tablero).

## Etapas

1. **Tablero + cámara.** 64 quads en damero (dos colores `(i+j)&1`), cámara
   3/4 cenital (`camPitch ≈ 0.95`, `camDist ≈ 12`). Cubo de 1 celda, estático,
   en una casilla central. Stick = orbitar para ajustar el ángulo. Tunear las
   constantes de cámara mirando RPCS3.
2. **Movimiento con flechas.** D-pad mueve el cubo por casillas, **disparo por
   flanco** (1 pulsación = 1 celda; guardar estado previo de botones), con
   límites `0..7`. HUD muestra la casilla (p.ej. "Casilla: e5"). Start = salir.
3. **Pulido (opcional).** Interpolar el salto entre casillas (lerp de la
   posición), orbital de cámara con el stick, sombra/resalte de la casilla.

## Notas

- Sin texturas: colores planos por vértice (`tiny3d_VertexColor`), igual que el
  cubo. El damero son quads de dos colores.
- Cubo estático (no gira) para que su ancho axis-aligned coincida con la celda;
  si girara, la diagonal se saldría de la casilla.
- Profundidad: `tiny3d_Clear(c, TINY3D_CLEAR_ALL)` limpia el z-buffer; el cubo
  sobre el tablero se resuelve solo por profundidad (sin culling).
