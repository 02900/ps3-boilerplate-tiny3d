/*
 * PS3 3D Test - spinning cube
 *
 * A minimal *real-3D* sandbox for PS3 homebrew (PSL1GHT + Tiny3D). Unlike the
 * sibling ports (ki-blast-arena, ps3-mega-mario) which only use Tiny3D's 2D
 * mode (tiny3d_Project2D), this draws a perspective scene:
 *
 *   - a perspective projection (MatrixProjPerspective)
 *   - a model/view transform rebuilt every frame (rotation + push-back)
 *   - a colour cube (6 quads) drawn with the depth buffer enabled
 *   - a 2D text HUD overlaid afterwards (Project2D)
 *
 * Use it as a starting point to experiment with the 3D pipeline: textures,
 * lighting (tiny3d_SetLight*), meshes, cameras, etc.
 *
 * Controls (pad on port 0):
 *   Left stick / D-pad ... orbit the cube (pitch / yaw)
 *   Cross (X) ............ hold to pause the auto-spin
 *   Start ................ quit to the XMB
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

/* Colours are RGBA (0xRRGGBBAA) for vertex/ttf; tiny3d_Clear wants 0xAARRGGBB. */
#define SKY_CLEAR     0xff0E1A2E
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

/* Emit one quad face: 4 corners (CCW), single flat colour. */
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

/* A unit cube spanning [-1,1] on each axis, one colour per face. */
static void draw_cube(void)
{
    tiny3d_SetPolygon(TINY3D_QUADS);

    /* +Z front  */ face(-1,-1, 1,  1,-1, 1,  1, 1, 1, -1, 1, 1, 0xE6483AFF); /* red    */
    /* -Z back   */ face( 1,-1,-1, -1,-1,-1, -1, 1,-1,  1, 1,-1, 0xF5A623FF); /* orange */
    /* +X right  */ face( 1,-1, 1,  1,-1,-1,  1, 1,-1,  1, 1, 1, 0x4AC94AFF); /* green  */
    /* -X left   */ face(-1,-1,-1, -1,-1, 1, -1, 1, 1, -1, 1,-1, 0x3FA9F5FF); /* blue   */
    /* +Y top    */ face(-1, 1, 1,  1, 1, 1,  1, 1,-1, -1, 1,-1, 0xF5D23FFF); /* yellow */
    /* -Y bottom */ face(-1,-1,-1,  1,-1,-1,  1,-1, 1, -1,-1, 1, 0xB45FF5FF); /* purple */

    tiny3d_End();
}

int main(void)
{
    /* Auto-spin plus user orbit; both feed the model rotation each frame. */
    float yaw = 0.6f, pitch = 0.5f;

    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL);
    init_screen();
    ioPadInit(7);

    while (running) {
        sysUtilCheckCallback();

        /* --- input ------------------------------------------------------ */
        int paused = 0;
        ioPadGetInfo(&pad_info);
        if (pad_info.status[0]) {
            ioPadGetData(0, &pad_data);

            if (pad_data.BTN_START) running = 0;
            paused = pad_data.BTN_CROSS;

            /* D-pad orbits in fixed steps. */
            if (pad_data.BTN_LEFT)  yaw   -= 0.05f;
            if (pad_data.BTN_RIGHT) yaw   += 0.05f;
            if (pad_data.BTN_UP)    pitch -= 0.05f;
            if (pad_data.BTN_DOWN)  pitch += 0.05f;

            /* Left stick orbits proportionally (0..255, centred at 128). */
            float lx = (pad_data.ANA_L_H - 128) / 128.0f;
            float ly = (pad_data.ANA_L_V - 128) / 128.0f;
            if (lx < -0.2f || lx > 0.2f) yaw   += lx * 0.06f;
            if (ly < -0.2f || ly > 0.2f) pitch += ly * 0.06f;
        }
        if (!paused) yaw += 0.012f;   /* gentle idle rotation */

        /* --- 3D pass ---------------------------------------------------- */
        tiny3d_Clear(SKY_CLEAR, TINY3D_CLEAR_ALL);
        tiny3d_Project3D();

        MATRIX proj = MatrixProjPerspective(DEG2RAD(60.0f), ASPECT, 0.1f, 1000.0f);
        tiny3d_SetProjectionMatrix(&proj);

        /* model/view: push 4.5 units down -Z FIRST, then rotate in place.
         * Tiny3D's MatrixMultiply(old,new) composes so the rightmost factor
         * hits the vertex first, so translation must seed the matrix and the
         * rotations follow — otherwise the cube ORBITS the camera (radius 4.5)
         * instead of spinning, swinging out of the frustum most of the time. */
        MATRIX mv = MatrixTranslation(0.0f, 0.0f, -4.5f);
        mv = MatrixMultiply(mv, MatrixRotationX(pitch));
        mv = MatrixMultiply(mv, MatrixRotationY(yaw));
        tiny3d_SetMatrixModelView(&mv);

        draw_cube();

        /* --- 2D HUD ----------------------------------------------------- */
        tiny3d_Project2D();
        reset_ttf_frame();
        display_ttf_string(40,  36, "PS3 3D Test - spinning cube", HUD_WHITE, 0, 18, 24);
        display_ttf_string(40, 456, "Stick / D-pad: orbit   X: pause spin   Start: exit",
                           HUD_DIM, 0, 14, 18);

        tiny3d_Flip();
    }

    return 0;
}
