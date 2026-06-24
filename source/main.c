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
 *   Left stick ........... orbit the camera around the cube (yaw / pitch)
 *   Start ................ quit to the XMB
 *
 * Stage 2 will add D-pad movement of the cube across the cells (the camera
 * follows because its target is the cube's position).
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

    /* Cube position on the board, in cells (start near the centre). */
    int cubeI = 4, cubeJ = 4;

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

            /* Left stick orbits the camera (0..255, centred at 128). */
            float lx = (pad_data.ANA_L_H - 128) / 128.0f;
            float ly = (pad_data.ANA_L_V - 128) / 128.0f;
            if (lx < -0.2f || lx > 0.2f) camYaw   += lx * 0.04f;
            if (ly < -0.2f || ly > 0.2f) camPitch += ly * 0.03f;
        }

        /* Keep the look-down angle sane (just above ground to near top-down). */
        if (camPitch < 0.1f) camPitch = 0.1f;
        if (camPitch > 1.3f) camPitch = 1.3f;

        /* --- 3D pass ---------------------------------------------------- */
        tiny3d_Clear(SKY_CLEAR, TINY3D_CLEAR_ALL);
        tiny3d_Project3D();

        MATRIX proj = MatrixProjPerspective(DEG2RAD(60.0f), ASPECT, 0.1f, 1000.0f);
        tiny3d_SetProjectionMatrix(&proj);

        /* Third-person camera orbiting the cube. Target = cube position (so the
         * camera follows it). Bring the target to the origin, yaw + pitch, then
         * push the whole scene +Z so the camera sits CAM_DIST behind the cube. */
        float tx = cell_center(cubeI), tz = cell_center(cubeJ);
        MATRIX mv = MatrixTranslation(-tx, -CAM_TARGET_Y, -tz);
        mv = MatrixMultiply(mv, MatrixRotationY(camYaw));
        mv = MatrixMultiply(mv, MatrixRotationX(-camPitch));   /* negative = look DOWN here */
        mv = MatrixMultiply(mv, MatrixTranslation(0.0f, 0.0f, CAM_DIST));
        tiny3d_SetMatrixModelView(&mv);

        draw_board();
        /* Cube rests on the board: centre y = CUBE_HALF so its base is at y=0. */
        draw_cube(tx, CUBE_HALF, tz, CUBE_HALF);

        /* --- 2D HUD ----------------------------------------------------- */
        tiny3d_Project2D();
        reset_ttf_frame();
        display_ttf_string(40,  36, "PS3 3D Test - cube on chessboard", HUD_WHITE, 0, 18, 24);
        display_ttf_string(40, 456, "Stick: orbit camera (3rd person)   Start: exit",
                           HUD_DIM, 0, 14, 18);

        tiny3d_Flip();
    }

    return 0;
}
