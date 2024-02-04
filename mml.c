#include "mml.h"
#include <ctype.h>

#define DEFAULT_BPM	120
#define OCTAVE		12
#define INTERVAL	17
#define ENV		0xe8
#define SLEEP		0xee
#define TERM		0xef

#ifdef __SDCC

#define TBASE		7500000
#define DESC_N		1

extern const u16 tonetable[];

void tone(u16 delta, u32 length);

#else	// __SDCC

#define TBASE		255000
#define DESC_N		4
#define NOISE	0xe0

static u8 *buf, *bufp, *lim;

static void putcmd(u8 v) {
	if (bufp && bufp < lim) *bufp++ = v;
}

static void putsleep(u16 v) {
	if (bufp && bufp < lim - 2) {
		*bufp++ = SLEEP;
		*bufp++ = v;
		*bufp++ = v >> 8;
	}
}

extern const u16 tonetable_dcsg[];

#endif	// __SDCC

typedef struct Desc {
	u8 *mml;
	u8 v, len, base, reg, valid;
	u16 wait;
} Desc;

static Desc desc[DESC_N];
static u32 tempo;

void mmlInit(void) {
	u8 reg = 0x80;
	for (Desc *p = desc; p < desc + DESC_N; p++, reg += 0x20) {
		p->mml = nil;
		p->base = 4 * OCTAVE;
		p->reg = reg;
		p->len = 4;
		p->v = reg | 0x10;
		p->valid = 0;
		p->wait = 0;
	}
	tempo = TBASE / DEFAULT_BPM;
}

static u8 *decimal(u8 *p, u8 *r) {
	if (isdigit(*p)) {
		*r = 0;
		do {
			*r *= 10;
			*r += *p++ - '0';
		} while (isdigit(*p));
	}
	return p;
}

static u32 lensub(Desc *d) {
	u8 l = d->len;
	d->mml = decimal(d->mml + 1, &l);
	if (!l) {
		d->mml--;
		return 0;
	}
	u32 t = tempo / l, t0 = t;
	while (*d->mml++ == '.') t += t0 >>= 1;
	d->mml -= 2;
	return t ? t : 1;
}

#ifdef __SDCC
#define waitsub(d)	lensub(d)
#else
#define waitsub(d)	((d)->wait = lensub(d))
#endif

static void mmlUpdate(Desc *d) {
	static const u8 cnv[] = { 9, 11, 0, 2, 4, 5, 7 };
	s8 oneshot = 0;
	while (d->mml && !d->wait) {
		u8 index, i;
#ifndef __SDCC
		u16 v;
#endif
		switch (*d->mml) {
			default:
				if (*d->mml < 'a') break;
#ifndef __SDCC
				if (d->reg != 0xe0) {
#endif
					if (*d->mml > 'g') break;
					index = d->base + cnv[*d->mml++ - 'a'] + oneshot;
					oneshot = 0;
					if (*d->mml == '+' || *d->mml == '#') index++;
					else if (*d->mml == '-') index--;
					else d->mml--;
#ifdef __SDCC
					tone(index < 128 - OCTAVE ? tonetable[index] : 0, waitsub(d));
#else
					waitsub(d);
					v = index < 128 - OCTAVE ? tonetable_dcsg[index] : 0;
					putcmd((v & 0xf) | d->reg);
					putcmd(v >> 4);
					putcmd(d->v);
					d->valid = 1;
				}
				else {
					if (*d->mml > 'h') break;
					putcmd(NOISE + *d->mml - 'a');
					putcmd(d->v);
					waitsub(d);
					d->valid = 1;
				}
#endif
				break;
#ifndef __SDCC
			case 'm':
				putcmd(ENV + (d->reg >> 5 & 3));
				v = lensub(d);
				if (v) v = (INTERVAL << 4) * (15 - (d->v & 15)) / v;
				putcmd(v);
				break;
#endif
			case 'r':
#ifdef __SDCC
				tone(0, waitsub(d));
#else
				waitsub(d);
				putcmd(0x1f | d->reg);
#endif
				break;
			case 'v':
				d->mml = decimal(d->mml + 1, &i) - 1;
#ifndef __SDCC
				d->v = d->reg | 15 - i | 16;
				if (!i) putcmd(d->v);
#endif
				break;
			case 'o':
//				d->mml++;
				if (isdigit(*++d->mml)) d->base = OCTAVE * (*d->mml - '0');
				break;
			case '>':
				if (d->base < 8 * OCTAVE) d->base += OCTAVE;
				break;
			case '<':
				if (d->base >= OCTAVE) d->base -= OCTAVE;
				break;
			case '~':
				oneshot = OCTAVE;
				break;
			case '_':
				oneshot = -OCTAVE;
				break;
			case 'l':
				d->mml = decimal(d->mml + 1, &d->len) - 1;
				break;
			case 't':
				i = 1;
				d->mml = decimal(d->mml + 1, &i) - 1;
				tempo = TBASE / i;
				break;
			case 0:
				d->mml = nil;
#ifndef __SDCC
				if (d->valid) putcmd(0x1f | d->reg);
#endif
				continue;
		}
		d->mml++;
	}
}

#ifndef __SDCC
static void mmlLoop(void) {
	Desc *p;
	for (p = desc; p < desc + DESC_N; p++) p->valid = 0;
	do {
		for (p = desc; p < desc + DESC_N; p++) mmlUpdate(p);
		u16 min = 0xffff;
		for (p = desc; p < desc + DESC_N; p++) 
			if (p->wait && min > p->wait) min = p->wait;
		if (min == 0xffff) break;
		putsleep(min);
		for (p = desc; p < desc + DESC_N; p++)
			if (p->wait) p->wait -= min;
	} while (1);
}
#endif

#ifdef __SDCC

void play(const char *mml) {
	desc[0].mml = (u8 *)mml;
	mmlUpdate(desc);
}

#else	// __SDCC

void play3(const char *mml1, const char *mml2, const char *mml3) {
	desc[0].mml = (u8 *)mml1;
	desc[1].mml = (u8 *)mml2;
	desc[2].mml = (u8 *)mml3;
	mmlLoop();
}

void play3n(const char *mml1, const char *mml2, const char *mml3, const char *mml_noise) {
	desc[0].mml = (u8 *)mml1;
	desc[1].mml = (u8 *)mml2;
	desc[2].mml = (u8 *)mml3;
	desc[3].mml = (u8 *)mml_noise;
	mmlLoop();
}

void mmlBeginCompile(u8 *p, u16 len) {
	buf = bufp = p;
	lim = buf + len - 1;
}

u16 mmlEndCompile(void) {
	*bufp++ = TERM;
	u16 r = bufp - buf;
	buf = bufp = lim = nil;
	return r;
}

#endif	// __SDCC
