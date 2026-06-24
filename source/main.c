/*
 * PS3 3D Test - cube on a chessboard
 *
 * A minimal *real-3D* sandbox for PS3 homebrew (PSL1GHT + Tiny3D). It draws an
 * 8x8 chessboard (in the XZ plane) and a colour cube the size of one cell
 * resting on top of it, viewed from a 3/4 top-down camera.
 *
 * Scene / camera convention (see ideas/cube-on-chessboard.md):
 *   - Board in the XZ plane at y=0, spanning [-4,+4]; 1x1 cells.
 *   - Cube half-size 0.5 (1 cell wide), sitting on the board (y in [0,1]).
 *   - Third-person orbit camera anchored on the cube: it looks AT the cube and
 *     sits behind/above it at a fixed distance, so you see the cube up close
 *     with the board receding into the distance. The whole scene shares one
 *     model-view. Tiny3D treats it as a ROW vector (v' = v*M) with
 *     MatrixMultiply(old,new)=old*new (leftmost factor hits the vertex first)
 *     and the camera looks down +Z, so the view matrix is:
 *       translate world by -target  ->  yaw  ->  pitch  ->  push +Z by dist
 *     A positive pitch tilts the view downward onto the board.
 *
 * Controls (pad on port 0):
 *   D-pad ................ move the cube one cell (edge-triggered, clamped 0..7)
 *   Left stick ........... orbit the camera around the cube (yaw / pitch)
 *   Start ................ quit to the XMB
 *
 * The camera follows the cube because its target is the cube's position.
 * D-pad movement is mapped to world axes (UP = +Z away from the camera).
 */

#include <stdio.h>
#include <math.h>

#include <ppu-types.h>
#include <io/pad.h>
#include <sysutil/sysutil.h>

#include <tiny3d.h>          /* pulls in matrix.h (MATRIX / VECTOR helpers) */
#include <ya2d/ya2d.h>
#include "ttf_render.h"

#define SCREEN_WIDTH  848
#define SCREEN_HEIGHT 512
#define ASPECT        (16.0f / 9.0f)   /* PS3 stretches the 848x512 buffer to 16:9 */

#define DEG2RAD(d)    ((d) * 0.01745329252f)

/* Board geometry. */
#define BOARD_N     8                      /* 8x8 cells                       */
#define CELL        1.0f                   /* one cell = one world unit       */
#define HALF_BOARD  (BOARD_N * CELL * 0.5f)/* 4.0: board spans [-4, +4]       */
#define CUBE_HALF   (CELL * 0.5f)          /* 0.5: cube is exactly one cell   */

/* Third-person camera (tune by eye in RPCS3). */
#define CAM_DIST    7.0f                   /* distance from camera to the cube */
#define CAM_TARGET_Y 0.8f                  /* look slightly above the cube base */
#define CAM_FOV     50.0f                  /* degrees; lower = less stretch     */

/* Cube hop animation between cells. */
#define MOVE_SPEED  0.10f                  /* anim progress per frame (~10 fr)  */
#define HOP_H       0.35f                  /* peak hop height (world units)     */

/* Colours are RGBA (0xRRGGBBAA) for vertex/ttf; tiny3d_Clear wants 0xAARRGGBB. */
#define SKY_CLEAR     0xff0E1A2E
#define BOARD_LIGHT   0xE8E4CFFF           /* cream squares                   */
#define BOARD_DARK    0x3C7A4EFF           /* green squares                   */
#define HUD_WHITE     0xFFFFFFFF
#define HUD_DIM       0x80B0C8FF

static int running = 1;
static padInfo  pad_info;
static padData  pad_data;

static void sys_callback(u64 status, u64 param, void *userdata)
{
    (void)param; (void)userdata;
    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

static void init_screen(void)
{
    tiny3d_Init(1024 * 1024);
    ya2d_init();

    /* Fonts live in PS3 flash; load a Latin face and hand ya2d its glyph table.
     * ya2d_texturePointer is u32*; init_ttf_table works in u16* units, so cast. */
    TTFLoadFont(0, "/dev_flash/data/font/SCE-PS3-SR-R-LATIN2.TTF", NULL, 0);
    ya2d_texturePointer = (u32*)init_ttf_table((u16*)ya2d_texturePointer);
    set_ttf_window(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

/* World-space centre of cell index i (0..BOARD_N-1) along one axis. */
static float cell_center(int i)
{
    return -HALF_BOARD + ((float)i + 0.5f) * CELL;
}

/* Emit one quad face: 4 corners, single flat colour. */
static void face(float ax, float ay, float az,
                 float bx, float by, float bz,
                 float cx, float cy, float cz,
                 float dx, float dy, float dz,
                 u32 color)
{
    tiny3d_VertexPos(ax, ay, az); tiny3d_VertexColor(color);
    tiny3d_VertexPos(bx, by, bz); tiny3d_VertexColor(color);
    tiny3d_VertexPos(cx, cy, cz); tiny3d_VertexColor(color);
    tiny3d_VertexPos(dx, dy, dz); tiny3d_VertexColor(color);
}

/* 8x8 chessboard, one flat quad per cell, in the XZ plane at y=0. */
static void draw_board(void)
{
    int i, j;
    tiny3d_SetPolygon(TINY3D_QUADS);

    for (j = 0; j < BOARD_N; j++) {
        for (i = 0; i < BOARD_N; i++) {
            float x0 = -HALF_BOARD + i * CELL, x1 = x0 + CELL;
            float z0 = -HALF_BOARD + j * CELL, z1 = z0 + CELL;
            u32 c = ((i + j) & 1) ? BOARD_DARK : BOARD_LIGHT;
            face(x0, 0.0f, z0,  x1, 0.0f, z0,
                 x1, 0.0f, z1,  x0, 0.0f, z1,  c);
        }
    }

    tiny3d_End();
}

/* Colour cube centred at (cx, cy, cz) with the given half-size, one colour per
 * face. */
static void draw_cube(float cx, float cy, float cz, float h)
{
    float x0 = cx - h, x1 = cx + h;
    float y0 = cy - h, y1 = cy + h;
    float z0 = cz - h, z1 = cz + h;

    tiny3d_SetPolygon(TINY3D_QUADS);

    /* -Z front  */ face(x0,y0,z0, x1,y0,z0, x1,y1,z0, x0,y1,z0, 0xE6483AFF); /* red    */
    /* +Z back   */ face(x1,y0,z1, x0,y0,z1, x0,y1,z1, x1,y1,z1, 0xF5A623FF); /* orange */
    /* +X right  */ face(x1,y0,z0, x1,y0,z1, x1,y1,z1, x1,y1,z0, 0x4AC94AFF); /* green  */
    /* -X left   */ face(x0,y0,z1, x0,y0,z0, x0,y1,z0, x0,y1,z1, 0x3FA9F5FF); /* blue   */
    /* +Y top    */ face(x0,y1,z0, x1,y1,z0, x1,y1,z1, x0,y1,z1, 0xF5D23FFF); /* yellow */
    /* -Y bottom */ face(x0,y0,z1, x1,y0,z1, x1,y0,z0, x0,y0,z0, 0xB45FF5FF); /* purple */

    tiny3d_End();
}

int main(void)
{
    /* Camera orbit (driven by the left stick). */
    float camYaw = 0.0f, camPitch = 0.40f;   /* gentle look-down, behind cube */

    /* Cube target cell on the board (start near the centre). */
    int cubeI = 4, cubeJ = 4;

    /* Smooth hop animation: the cube slides from `from` to the target cell's
     * centre as animT goes 0 -> 1 (1.0 = idle / move finished). */
    float curX = cell_center(cubeI), curZ = cell_center(cubeJ);
    float fromX = curX, fromZ = curZ;
    float animT = 1.0f;

    /* Previous D-pad state, for edge-triggered (one press = one cell) moves. */
    int prevUp = 0, prevDown = 0, prevLeft = 0, prevRight = 0;

    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL);
    init_screen();
    ioPadInit(7);

    while (running) {
        sysUtilCheckCallback();

        /* --- input ------------------------------------------------------ */
        ioPadGetInfo(&pad_info);
        if (pad_info.status[0]) {
            ioPadGetData(0, &pad_data);

            if (pad_data.BTN_START) running = 0;

            /* D-pad moves the cube one cell per press (edge-triggered). World
             * axes: UP = +Z (away from camera), RIGHT = +X. A new move is only
             * accepted once the previous hop has finished (animT >= 1). */
            int up = pad_data.BTN_UP, down = pad_data.BTN_DOWN;
            int left = pad_data.BTN_LEFT, right = pad_data.BTN_RIGHT;
            if (animT >= 1.0f) {
                int ni = cubeI, nj = cubeJ;
                if      (up    && !prevUp)    nj = cubeJ + 1;
                else if (down  && !prevDown)  nj = cubeJ - 1;
                else if (right && !prevRight) ni = cubeI + 1;
                else if (left  && !prevLeft)  ni = cubeI - 1;
                if (ni < 0) ni = 0; if (ni > BOARD_N - 1) ni = BOARD_N - 1;
                if (nj < 0) nj = 0; if (nj > BOARD_N - 1) nj = BOARD_N - 1;
                if (ni != cubeI || nj != cubeJ) {
                    fromX = curX; fromZ = curZ;   /* start the hop from here */
                    cubeI = ni;   cubeJ = nj;
                    animT = 0.0f;
                }
            }
            prevUp = up; prevDown = down; prevLeft = left; prevRight = right;

            /* Left stick orbits the camera (0..255, centred at 128). */
            float lx = (pad_data.ANA_L_H - 128) / 128.0f;
            float ly = (pad_data.ANA_L_V - 128) / 128.0f;
            if (lx < -0.2f || lx > 0.2f) camYaw   += lx * 0.04f;
            if (ly < -0.2f || ly > 0.2f) camPitch += ly * 0.03f;
        }

        /* Keep the look-down angle sane (just above ground to near top-down). */
        if (camPitch < 0.1f) camPitch = 0.1f;
        if (camPitch > 1.3f) camPitch = 1.3f;

        /* Advance the hop animation. Position eases (smoothstep) from `from` to
         * the target cell; hopY is a sine arc that is 0 at both ends. */
        if (animT < 1.0f) {
            animT += MOVE_SPEED;
            if (animT > 1.0f) animT = 1.0f;
        }
        float s = animT * animT * (3.0f - 2.0f * animT);   /* smoothstep */
        curX = fromX + (cell_center(cubeI) - fromX) * s;
        curZ = fromZ + (cell_center(cubeJ) - fromZ) * s;
        float hopY = sinf(animT * 3.14159265f) * HOP_H;

        /* --- 3D pass ---------------------------------------------------- */
        tiny3d_Clear(SKY_CLEAR, TINY3D_CLEAR_ALL);
        tiny3d_Project3D();

        MATRIX proj = MatrixProjPerspective(DEG2RAD(CAM_FOV), ASPECT, 0.1f, 1000.0f);
        tiny3d_SetProjectionMatrix(&proj);

        /* Third-person camera orbiting the cube. Target = cube's (smoothed)
         * position, so the camera glides after it. Bring the target to the
         * origin, yaw + pitch, then push the scene +Z behind the camera. The
         * camera tracks X/Z but not the hop, so it stays steady. */
        MATRIX mv = MatrixTranslation(-curX, -CAM_TARGET_Y, -curZ);
        mv = MatrixMultiply(mv, MatrixRotationY(camYaw));
        mv = MatrixMultiply(mv, MatrixRotationX(-camPitch));   /* negative = look DOWN here */
        mv = MatrixMultiply(mv, MatrixTranslation(0.0f, 0.0f, CAM_DIST));
        tiny3d_SetMatrixModelView(&mv);

        draw_board();
        /* Cube rests on the board (base at y=0) plus the hop arc. */
        draw_cube(curX, CUBE_HALF + hopY, curZ, CUBE_HALF);

        /* --- 2D HUD ----------------------------------------------------- */
        tiny3d_Project2D();
        reset_ttf_frame();
        display_ttf_string(40,  36, "PS3 3D Test - cube on chessboard", HUD_WHITE, 0, 18, 24);

        /* Current cell in chess notation: file a..h (cubeI), rank 1..8 (cubeJ). */
        char cell[16];
        snprintf(cell, sizeof(cell), "Cell: %c%d", 'a' + cubeI, cubeJ + 1);
        display_ttf_string(40, 64, cell, HUD_DIM, 0, 14, 20);

        display_ttf_string(40, 456, "D-pad: move   Stick: orbit camera   Start: exit",
                           HUD_DIM, 0, 14, 18);

        tiny3d_Flip();
    }

    return 0;
}
