/*
 * Minimal MikMod audio — a looping tune + a "blip" SFX, both SYNTHESIZED as
 * 16-bit mono PCM in code and handed to MikMod through an in-memory MREADER.
 * No audio assets on disk, so this is safe to ship in a template. Init is
 * defensive: any failure leaves audio_ok = 0 and every call becomes a no-op.
 *
 * (Adapted from ps3-mega-mario/source/audio.c, trimmed to two original sounds.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ppu-types.h>
#include <mikmod.h>

#include "audio.h"

#define SR 22050   /* synthesis sample rate */

static int     audio_ok = 0;
static SAMPLE *s_blip = NULL, *s_music = NULL;
static SBYTE   music_voice = -1;

/* ---- in-memory MREADER (lets MikMod parse a WAV from a RAM buffer) -------- */
typedef struct { MREADER core; const unsigned char *data; long size, pos; } MemReader;

static BOOL mr_eof(MREADER *r)  { MemReader *m = (MemReader *)r; return m->pos >= m->size; }
static long mr_tell(MREADER *r) { return ((MemReader *)r)->pos; }
static int  mr_get(MREADER *r)
{
	MemReader *m = (MemReader *)r;
	return (m->pos >= m->size) ? EOF : m->data[m->pos++];
}
static BOOL mr_read(MREADER *r, void *dst, size_t n)
{
	MemReader *m = (MemReader *)r;
	long rem = m->size - m->pos;
	if ((long)n > rem) { if (rem > 0) { memcpy(dst, m->data + m->pos, rem); m->pos += rem; } return 0; }
	memcpy(dst, m->data + m->pos, n); m->pos += n; return 1;
}
static BOOL mr_seek(MREADER *r, long off, int whence)
{
	MemReader *m = (MemReader *)r;
	long base = (whence == SEEK_SET) ? 0 : (whence == SEEK_CUR) ? m->pos : m->size;
	long np = base + off;
	if (np < 0 || np > m->size) return -1;
	m->pos = np; return 0;
}
static SAMPLE *load_wav(const void *data, long size)
{
	MemReader mr;
	mr.core.Seek = mr_seek; mr.core.Tell = mr_tell; mr.core.Read = mr_read;
	mr.core.Get = mr_get;   mr.core.Eof = mr_eof;
	mr.data = (const unsigned char *)data; mr.size = size; mr.pos = 0;
	return Sample_LoadGeneric(&mr.core);
}

/* ---- PCM synthesis ------------------------------------------------------- */
/* Append a square-wave segment sweeping f0->f1 over ms at amp; f0<=0 => silence.
 * Short attack/release ramps avoid clicks; phase carries across calls. */
static int seg(SWORD *buf, int cap, int pos, double f0, double f1,
               int ms, int amp, double *phase)
{
	int n = (int)((long)ms * SR / 1000); if (n < 1) n = 1;
	int atk = SR * 3 / 1000, rel = SR * 10 / 1000;
	for (int i = 0; i < n && pos < cap; i++, pos++) {
		if (f0 <= 0 && f1 <= 0) { buf[pos] = 0; continue; }
		double frac = (double)i / n;
		double f = f0 + (f1 - f0) * frac;
		*phase += f / SR;
		if (*phase >= 1.0) *phase -= (double)(long)*phase;
		double e = 1.0;
		if (i < atk)          e = (double)i / atk;
		else if (i > n - rel) e = (double)(n - i) / rel;
		double v = (*phase < 0.5) ? amp : -amp;
		int s = (int)(v * e);
		buf[pos] = (SWORD)(s > 32767 ? 32767 : s < -32768 ? -32768 : s);
	}
	return pos;
}

static void put_le32(unsigned char *p, u32 v) { p[0]=v; p[1]=v>>8; p[2]=v>>16; p[3]=v>>24; }
static void put_le16(unsigned char *p, u16 v) { p[0]=v; p[1]=v>>8; }

/* Wrap PCM (n mono 16-bit samples) in a little-endian WAV and load it. */
static SAMPLE *load_pcm(const SWORD *pcm, int n)
{
	long data = (long)n * 2, total = 44 + data;
	unsigned char *w = (unsigned char *)malloc(total);
	if (!w) return NULL;
	memcpy(w, "RIFF", 4);       put_le32(w + 4, (u32)(36 + data));
	memcpy(w + 8, "WAVE", 4);
	memcpy(w + 12, "fmt ", 4);  put_le32(w + 16, 16);
	put_le16(w + 20, 1);        put_le16(w + 22, 1);          /* PCM, mono   */
	put_le32(w + 24, SR);       put_le32(w + 28, SR * 2);     /* rate, byterate */
	put_le16(w + 32, 2);        put_le16(w + 34, 16);         /* block, bits */
	memcpy(w + 36, "data", 4);  put_le32(w + 40, (u32)data);
	for (int i = 0; i < n; i++) put_le16(w + 44 + i * 2, (u16)pcm[i]);
	SAMPLE *s = load_wav(w, total);
	free(w);
	return s;
}

static SWORD g_scratch[SR * 3];   /* up to 3 s */

static SAMPLE *synth_blip(void)   /* a quick upward chirp */
{
	double ph = 0; int n = seg(g_scratch, SR * 3, 0, 440, 990, 90, 8000, &ph);
	return load_pcm(g_scratch, n);
}
static SAMPLE *synth_music(void)  /* a short cheerful C-major square-wave loop */
{
	static const int mf[16] = { 523, 659, 784, 1047, 784, 659, 587, 523,
	                            587, 659, 698, 784, 659, 523, 587, 0 };
	static const int ml[16] = { 150,150,150,150,150,150,150,300,
	                            150,150,150,150,150,150,300,150 };
	double ph = 0; int n = 0;
	for (int k = 0; k < 16; k++) {
		n = seg(g_scratch, SR * 3, n, mf[k], mf[k], ml[k] - 18, 5200, &ph);
		n = seg(g_scratch, SR * 3, n, 0, 0, 18, 0, &ph);   /* tiny gap */
	}
	return load_pcm(g_scratch, n);
}

/* ---- public API ---------------------------------------------------------- */
void audio_init(void)
{
	if (audio_ok) return;

	MikMod_RegisterAllDrivers();
	MikMod_RegisterAllLoaders();

	md_mode = DMODE_STEREO | DMODE_16BITS | DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
	md_mixfreq = 48000;

	if (MikMod_Init("")) return;                       /* silent on failure */
	MikMod_SetNumVoices(0, 8);
	if (MikMod_EnableOutput()) { MikMod_Exit(); return; }

	s_blip  = synth_blip();
	s_music = synth_music();
	if (s_music) {
		s_music->flags    |= SF_LOOP;
		s_music->loopstart = 0;
		s_music->loopend   = s_music->length;
		music_voice = Sample_Play(s_music, 0, SFX_CRITICAL);      /* won't be stolen */
		if (music_voice >= 0) Voice_SetVolume(music_voice, 96);   /* duck under SFX  */
	}

	audio_ok = 1;
}

void audio_update(void)   { if (audio_ok) MikMod_Update(); }

void audio_shutdown(void)
{
	if (!audio_ok) return;
	audio_ok = 0;
	if (music_voice >= 0) Voice_Stop(music_voice);
	if (s_music) Sample_Free(s_music);
	if (s_blip)  Sample_Free(s_blip);
	MikMod_DisableOutput();
	MikMod_Exit();
}

void audio_play_blip(void) { if (audio_ok && s_blip) Sample_Play(s_blip, 0, 0); }
