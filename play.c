#include "play.h"
#include "utils.h"

#define PLAYER_N	8

__sfr __at 0x90 SN;

typedef struct {
	u8 *buf, *p;
	u16 wait;
	u8 ch, flags;
	u8 r[16];
	u8 env[4];
} Player;

Player player[PLAYER_N];
static u8 mute;

extern void (*interruptHandler)(void);
extern const u8 music[];

void playInit(void) {
	playStopAll();
	mute = 0;
}

#if 1
void playRestore(Player *c, u8 restore) CC0;
void playStopCh(u8 ch) CC0;
void playInterrupt(void);
#else
#define INTERVAL	17
#define ENV		0xe8
#define SLEEP		0xee
#define TERM		0xef
static void playRestore(Player *c, u8 restore) {
	u8 *p = c->r;
	for (u8 m = 1; m < 16; p += 4, m <<= 1)
		if (restore & c->ch & m) {
			SN = p[0];
			SN = p[1];
			SN = p[2];
		}
}
static void playStopCh(u8 ch) {
	for (u8 m = 1, d = 0x9f; m < 16; m <<= 1, d += 0x20)
		if (ch & m) SN = d;
}
static void playInterrupt(void) {
	u8 restore = 0, higher_ch = 0;
	for (Player *c = player + PLAYER_N - 1; c >= player; c--) {
		if (!c->p) continue;
		if (restore) playRestore(c, restore);
		if (c->wait) {
			s16 t = c->wait - INTERVAL;
			if (t < 0) t = 0;
			c->wait = t;
			for (u8 i = 0, j = 2, b = 0x90, m = 1; i < 4; i++, j += 4, b += 0x20, m <<= 1) {
				u16 t = c->r[j + 1] + c->env[i];
				if (t > 0xff) t = 0xff;
				c->r[j + 1] = t;
				t = b | t >> 4;
				c->r[j] = t;
				if (!(higher_ch & m)) SN = t;
			}
		}
		while (c->p && !c->wait) {
			u8 *p = c->p;
			u8 last = 0, valid = 0;
			while (*p != SLEEP && *p != TERM) {
				if (*p >= ENV && *p < ENV + 4) {
					c->env[*p - ENV] = p[1];
					p += 2;
				}
				else {
					u8 d = *p++;
					u8 sel = d >> 3 & 0xe;
					if (d & 0x80) {
						c->r[sel] = d;
						last = sel;
						u8 m = 1 << (sel >> 2);
						c->ch |= m;
						valid = !(higher_ch & m);
						if (d & 0x10) c->r[sel | 1] = d << 4;
					}
					else c->r[last | 1] = d;
					if (valid) SN = d;
				}
			}
			if (*p == SLEEP) {
				c->wait = p[1] | p[2] << 8;
				p += 3;
			}
			else if (c->flags & PF_LOOP) p = c->buf;
			else {
				p = nil;
				restore |= ~higher_ch & c->ch;
				c->ch = 0;
			}
			c->p = p;
		}
		higher_ch |= c->ch;
		restore &= ~c->ch;
	}
	playStopCh(restore);
}
#endif

void playStart(u8 index, u16 ofs, u8 f) {
	if (mute || index >= PLAYER_N) return;
	Player *p = &player[index];
	u8 l = enterCritical();
	if (p->p) playStop(index);
	exitCritical(l);
	p->buf = (u8 *)&music[ofs];
	p->ch = 0;
	p->flags = f;
	p->wait = 0;
	for (u8 i = 0; i < 16; i += 4) {
		p->r[i] = 0x8f | i << 3;
		p->r[i + 1] = 0;
		p->r[i + 2] = 0x9f | i << 3;
		p->r[i + 3] = 0xff;
	}
	for (u8 i = 0; i < 4; i++) p->env[i] = 0;
	p->p = p->buf; // enable channel
	interruptHandler = playInterrupt;
}

void playStop(u8 index) {
	if (index >= PLAYER_N) return;
	Player *p = &player[index];
	u8 restore = 0, higher_ch = 0;
	u8 l = enterCritical();
	for (Player *c = player + PLAYER_N - 1; c >= player; c--)
		if (c->p) {
			playRestore(c, restore);
			if (c == p) {
				p->p = nil;
				restore = ~higher_ch & c->ch;
			}
			else {
				higher_ch |= c->ch;
				restore &= ~c->ch;
			}
		}
	exitCritical(l);
	playStopCh(restore);
}

void playStopAll(void) {
	u8 l = enterCritical();
	for (Player *c = player; c < player + PLAYER_N; c++) c->p = nil;
	exitCritical(l);
	SN = 0x9f;
	SN = 0xbf;
	SN = 0xdf;
	SN = 0xff;
	interruptHandler = nil;
}

void playMute(u8 f) {
	mute = f;
	if (f) playStopAll();
}

u16 playing(u8 index) {
	return index < PLAYER_N && player[index].p != nil ? player[index].buf - music : 0;
}
