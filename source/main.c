/*
 * PS3 Tiny3D Boilerplate — 2D sprite + audio
 *
 * A minimal "hello, game" for PS3 homebrew with Tiny3D + ya2d: load a PNG sprite,
 * move it with the pad, bounce it off the screen edges (a blip plays on each
 * bounce) while a short synthesized tune loops. ya2d handles the PNG decode and
 * the 2D textured quad; Tiny3D provides the frame + 2D projection.
 *
 * The sprite (data/sprite.png) is embedded at build time (bin2o -> sprite_png);
 * the audio is synthesized in source/audio.c (no asset files).
 *
 * Controls (gamepad 0): D-pad / left stick nudge the sprite · Start exits.
 *
 * The previous 3D chessboard sample lives on the `3d-chessboard` branch.
 */
#include <ppu-types.h>
#include <io/pad.h>
#include <sysutil/sysutil.h>

#include <tiny3d.h>
#include <ya2d/ya2d.h>

#include "audio.h"

/* Embedded sprite (bin2o from data/sprite.png). */
extern const unsigned char sprite_png[];
extern const unsigned int  sprite_png_size;

#define SCREEN_W   848                 /* Tiny3D's fixed 2D canvas (stretched to 16:9) */
#define SCREEN_H   512
#define SKY_CLEAR  0xff0E1A2E          /* tiny3d_Clear wants 0xAARRGGBB */
#define MAX_SPEED  9.0f

static int     running = 1;
static padInfo pad_info;
static padData pad_data;

static void sys_callback(u64 status, u64 param, void *userdata)
{
	(void)param; (void)userdata;
	if (status == SYSUTIL_EXIT_GAME) running = 0;
}

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }

int main(void)
{
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL);

	tiny3d_Init(1024 * 1024);
	ya2d_init();
	audio_init();
	ioPadInit(7);

	ya2d_Texture *sprite = ya2d_loadPNGfromBuffer((void *)sprite_png, sprite_png_size);
	int sw = sprite ? sprite->imageWidth  : 48;
	int sh = sprite ? sprite->imageHeight : 48;

	float x = (SCREEN_W - sw) * 0.5f, y = (SCREEN_H - sh) * 0.5f;
	float vx = 3.0f, vy = 2.2f;   /* starts moving so it bounces on its own */

	while (running) {
		sysUtilCheckCallback();

		int up = 0, dn = 0, lf = 0, rt = 0;
		float lx = 0, ly = 0;
		ioPadGetInfo(&pad_info);
		if (pad_info.status[0]) {
			ioPadGetData(0, &pad_data);
			if (pad_data.BTN_START) running = 0;
			up = pad_data.BTN_UP;   dn = pad_data.BTN_DOWN;
			lf = pad_data.BTN_LEFT; rt = pad_data.BTN_RIGHT;
			lx = (pad_data.ANA_L_H - 128) / 128.0f;
			ly = (pad_data.ANA_L_V - 128) / 128.0f;
		}

		/* nudge the velocity with the D-pad and the left stick */
		if (lf) vx -= 0.3f; if (rt) vx += 0.3f;
		if (up) vy -= 0.3f; if (dn) vy += 0.3f;
		if (lx < -0.2f || lx > 0.2f) vx += lx * 0.4f;
		if (ly < -0.2f || ly > 0.2f) vy += ly * 0.4f;
		vx = clampf(vx, -MAX_SPEED, MAX_SPEED);
		vy = clampf(vy, -MAX_SPEED, MAX_SPEED);

		x += vx; y += vy;

		/* bounce off the four edges; blip on each */
		if (x < 0)              { x = 0;              vx = -vx; audio_play_blip(); }
		if (x > SCREEN_W - sw)  { x = SCREEN_W - sw;  vx = -vx; audio_play_blip(); }
		if (y < 0)              { y = 0;              vy = -vy; audio_play_blip(); }
		if (y > SCREEN_H - sh)  { y = SCREEN_H - sh;  vy = -vy; audio_play_blip(); }

		tiny3d_Clear(SKY_CLEAR, TINY3D_CLEAR_ALL);
		tiny3d_Project2D();
		if (sprite) ya2d_drawTexture(sprite, x, y);
		tiny3d_Flip();

		audio_update();
	}

	audio_shutdown();
	if (sprite) ya2d_freeTexture(sprite);
	return 0;
}
