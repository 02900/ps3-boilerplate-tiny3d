/*
 * Minimal MikMod audio for the boilerplate — a looping tune + one SFX, all
 * SYNTHESIZED in code (no asset files, so it's license-safe to ship in a
 * template). Backend-independent: identical in the tiny3d / rsxgl / raylib repos.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void audio_init(void);      /* once at startup (after the PS3 stack is up)  */
void audio_update(void);    /* every frame — drives MikMod's software mixer */
void audio_shutdown(void);  /* on exit                                      */
void audio_play_blip(void); /* short SFX (played on each bounce)            */

#ifdef __cplusplus
}
#endif
