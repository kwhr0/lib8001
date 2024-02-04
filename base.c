#include "base.h"
#include "utils.h"
#include <string.h>

#define USKBD

#define KEYWAIT			20
#define KEYWAITREPEAT	3

__sfr __at 0x10 OUT10;
__sfr __at 0x30 OUT30;
__sfr __at 0x40 IO40;
__sfr __at 0x50 CRTC_PRM;
__sfr __at 0x51 CRTC_CMD;
__sfr __at 0x64 DMA_ADR2;
__sfr __at 0x65 DMA_CNT2;
__sfr __at 0x66 DMA_ADR3;
__sfr __at 0x68 DMA_CMD;

#define KEYN		4
#define VRAM		((u8 *)0xf300)
#define VRAM2		((u8 *)0xe700)
#define CLEARLINE	(VRAM + VRAM_XN * SCREEN_YN)

volatile extern u8 interruptCount, keyTarget, keyWait;

u8 *vram;
u8 width40, curAttr, isColorMode, last40, cursX, cursY;

static u8 screenXn, screenYn, isGraph, printer, frameCount;
static u8 keyN, lastKey, modifier;
static u8 *keyR, *keyW;
static u8 keyBuf[KEYN];
static u8 lastPort[11];

static IntFunc intFunc;

//// DISPLAY ////

u8 *vramLine(u8 y) {
	return y < SCREEN_YN ? vram + VRAM_XN * y : 0;
}

/*static*/inline void clearLine(void) {
	u8 i;
	u8 *p = CLEARLINE;
	for (i = 0; i <= SCREEN_XN; i++) *p++ = 0;
	if (isColorMode) {
		*p++ = 0;
		*p++ = width40 + 1;
	}
	else {
		*p++ = curAttr;
		*p++ = SCREEN_XN;
	}
	*p++ = curAttr;
	for (i = 2; i < ATTR_N; i++) {
		*p++ = SCREEN_XN;
		*p++ = curAttr;
	}
}

void cls(void) {
	u8 *p = vram;
	for (u8 i = 0; i < SCREEN_YN; i++) {
		memcpy(p, CLEARLINE, VRAM_XN);
		p += VRAM_XN;
	}
	cursX = cursY = 0;
}

void writeChar(u8 c) {
	u8 *p = vramLine(cursY);
	if (!p) return;
	u8 x = cursX + isColorMode;
	if (width40) x <<= 1;
	p[x] = c;
}

void newLine(void) {
	cursX = 0;
	if (++cursY >= screenYn) {
		--cursY;
		// separate for generating LDIR
		if (screenYn == 25) {
			memcpy(vram, vram + VRAM_XN, VRAM_XN * (25 - 1));
			memcpy(vram + VRAM_XN * (25 - 1), CLEARLINE, VRAM_XN);
		}
		else {
			memcpy(vram, vram + VRAM_XN, VRAM_XN * (20 - 1));
			memcpy(vram + VRAM_XN * (20 - 1), CLEARLINE, VRAM_XN);
		}
	}
}

static void insertAttr(u8 *p, u8 *lim, u8 ofs) {
	u8 *dp = lim, *sp = dp - ofs;
	while (sp > p) *--dp = *--sp;
}

void setAttr(u8 x, u8 y, u8 attr) {
	if (!attrIsValid(attr) || isColorMode && !++x) return;
	if (width40) x <<= 1;
	if (x >= SCREEN_XN) return;
	u8 *a = vramLine(y);
	if (!a) return;
	u8 *p = a += SCREEN_XN, *lim = a + 2 * ATTR_N;
	if (isColorMode) p += 2;
	for (; p < lim && *p < x; p += 2) 
		;
	if (p >= lim) return;
	u8 d = width40 + 1;
	if (*p == x) {
		if (p[1] == attr) return;
		if (p > a && p[-1] == attr) 
			if (p >= lim - 2 || p[2] != x + d) *p += d;
			else {
				u8 *sp = p + (p[3] == attr ? 4 : 2);
				while (sp < lim) *p++ = *sp++;
				while (p < lim) {
					*p++ = SCREEN_XN;
					*p++ = attr;
				}
			}
		else {
			if (p < lim - 2 && p[2] != x + d) {
				insertAttr(p, lim, 2);
				p[2] = x + d;
			}
			p[1] = attr;
		}
	}
	else if (*p == x + d) {
		if (p[1] == attr) *p = x;
		else if (p[-1] != attr) {
			insertAttr(p, lim, 2);
			p[0] = x;
			p[1] = attr;
		}
	}
	else if (p > a && p[-1] != attr) {
		insertAttr(p, lim, 4);
		p[0] = x;
		p[1] = attr;
		if (p < lim - 3) {
			p[2] = x + d;
			p[3] = p[-1];
		}
	}
}

int putchar(int c) {
	if (printer) {
		OUT10 = c;
		IO40 = last40 & ~1;
		IO40 = last40;
	}
	else if (c == '\r') cursX = 0;
	else if (c == '\n') newLine();
	else {
		setAttr(cursX, cursY, curAttr & (isColorMode ? 0xef : 0x7f));
		writeChar(c);
		if (++cursX >= screenXn) newLine();
	}
	return 0;
}

void puts_n(char *p) {
	while (*p) putchar(*p++);
}

void locate(u8 x, u8 y) {
	cursX = x;
	cursY = y;
}

void color(s8 color) {
	curAttr = color >= 0 ? isColorMode ? (color << 1 | isGraph) << 4 | 8 : color & 7 | isGraph << 7 : ATTR_INVALID;
}

void cursorOn(void) {
	u8 x = cursX + isColorMode;
	if (width40) x <<= 1;
	CRTC_CMD = 0x81;
	CRTC_PRM = x;
	CRTC_PRM = cursY;
}

void cursorOff(void) {
	CRTC_CMD = 0x80;
}

void setPrinter(u8 f) {
	printer = f;
}

void intSet(IntFunc func) {
	intFunc = func;
}

u8 waitVSync(u8 frames) {
	u8 dif = interruptCount - frameCount;
	u16 t = (u16)frameCount + frames;
	u8 t1 = t, f;
	if (t1 >= frameCount)
		while ((f = interruptCount) >= frameCount && f < t1)
			intFunc();
	else
		while ((f = interruptCount) >= frameCount || f < t1)
			intFunc();
	frameCount = f;
	return dif;
}

static void setDMA(void) {
	DMA_CMD = 0xc4;
	DMA_ADR3 = (u16)vram & 0xff;
	DMA_ADR3 = (u16)vram >> 8;
}

u8 vramSwap(u8 frames) {
	u8 dif = waitVSync(frames);
	setDMA();
	vram = vram == VRAM ? VRAM2 : VRAM;
	return dif;
}

void vramSingle(void) {
	vram = VRAM;
	setDMA();
}

void setupScreen(u8 width, u8 height, u8 colorMode, u8 _color, u8 graph) {
	if (width != 40 && width != 80) return;
	if (height != 20 && height != 25) return;
	vram = VRAM;
	isColorMode = colorMode ? 1 : 0;
	isGraph = graph ? 1 : 0;
	width40 = width == 40;
	screenXn = width - isColorMode;
	screenYn = height;
	frameCount = interruptCount;
	cursorOff();
	color(_color);
	clearLine();
	cls();
	OUT30 = !colorMode << 1 | !width40;
	IO40 = last40 | 8;
	CRTC_CMD = 0;
	DMA_CMD = 0x80;
	DMA_ADR2 = (u16)VRAM & 0xff;
	DMA_ADR2 = (u16)VRAM >> 8;
	u16 l = SCREEN_XN * height - 1;
	DMA_CNT2 = l;
	DMA_CNT2 = l >> 8 | 0x80;
	CRTC_PRM = 0xce;
	if (height == 20) {
		CRTC_PRM = 0x93;
		CRTC_PRM = 0x69;
		CRTC_PRM = 0xbe;
	}
	else {
		CRTC_PRM = 0x98;
		CRTC_PRM = 0x67;
		CRTC_PRM = 0xde;
	}
	CRTC_PRM = isColorMode << 6 | 0x13;
	CRTC_CMD = 0x43;
	DMA_CMD = 0xc4;
	CRTC_CMD = 0x20;
	IO40 = last40;
}

//// KEYBOARD ////

static void clrKeyBuf(void) {
	keyR = keyW = keyBuf;
	keyN = 0;
}

u8 keyPress(void) {
	static const u8 normal[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '*', '+', '=', ',', '.', 13,
#ifdef USKBD
		'[', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
		'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
		'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
		'x', 'y', 'z', ']', '?', '\\', '=', '-',
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 39, ';', ',', '.', '/', 0,
#else
		'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
		'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
		'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
		'x', 'y', 'z', '[', '\\', ']', '^', '-',
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', ';', ':', ',', '.', '/', 0,
#endif
		12, 30, 28, 8, 0, 0, 0, 0,
		3, 0, 0, 0, 0, 0, ' ', 27,
		31, 29 // extended cursor key
	};
	static const u8 shift[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '*', '+', '=', ',', '.', 13,
#ifdef USKBD
		'{', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
		'X', 'Y', 'Z', '}', 0, '|', '+', '_',
		')', '!', '@', '#', '$', '%', '^', '&',
		'*', '(', 34, ':', '<', '>', '?', '_',
#else
		0, 'A', 'B', 'C', 'D', 'E', 'F', 'G',
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
		'X', 'Y', 'Z', 0, 0, 0, 0, '=',
		0, '!', 34, '#', '$', '%', '&', 39,
		'(', ')', '*', '+', '<', '>', '?', '_',
#endif
		11, 31, 29, 8, 0, 0, 0, 0,
		3, 0, 0, 0, 0, 0, ' ', 27,
	};
	static const u8 kana_normal[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '*', '+', '=', ',', '.', 13,
		0xde, 0xc1, 0xba, 0xbf, 0xbc, 0xb2, 0xca, 0xb7,
		0xb8, 0xc6, 0xcf, 0xc9, 0xd8, 0xd3, 0xd0, 0xd7,
		0xbe, 0xc0, 0xbd, 0xc4, 0xb6, 0xc5, 0xcb, 0xc3,
		0xbb, 0xdd, 0xc2, 0xdf, 0xb0, 0xd1, 0xcd, 0xce,
		0xdc, 0xc7, 0xcc, 0xb1, 0xb3, 0xb4, 0xb5, 0xd4,
		0xd5, 0xd6, 0xb9, 0xda, 0xc8, 0xd9, 0xd2, 0xdb,
		12, 30, 28, 8, 0, 0, 0, 0,
		3, 0, 0, 0, 0, 0, ' ', 27,
	};
	static const u8 kana_shift[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '*', '+', '=', ',', '.', 13,
		0xde, 0xc1, 0xba, 0xbf, 0xbc, 0xb2, 0xca, 0xb7,
		0xb8, 0xc6, 0xcf, 0xc9, 0xd8, 0xd3, 0xd0, 0xd7,
		0xbe, 0xc0, 0xbd, 0xc4, 0xb6, 0xc5, 0xcb, 0xc3,
		0xbb, 0xdd, 0xaf, 0xdf, 0xb0, 0xd1, 0xcd, 0xce,
		0xa6, 0xc7, 0xcc, 0xa7, 0xa9, 0xaa, 0xab, 0xac,
		0xad, 0xae, 0xb9, 0xda, 0xc8, 0xd9, 0xd2, 0xdb,
		12, 30, 28, 8, 0, 0, 0, 0,
		3, 0, 0, 0, 0, 0, ' ', 27,
	};
	static const u8 graph[] = {
		0x9a, 0x93, 0x8f, 0x92, 0xe1, 0xe2, 0xe3, 0x98,
		0x91, 0x99, 0x95, 0xe0, 0x96, 0x90, 0x9b, 0x96,
		0x8a, 0x9e, 0x84, 0x82, 0xe6, 0xe4, 0xe7, 0xec,
		0xed, 0xe8, 0xea, 0xeb, 0x8e, 0x86, 0x85, 0xe9,
		0x8d, 0x9c, 0xe5, 0x9f, 0xee, 0xf0, 0x83, 0x9d,
		0x81, 0xef, 0x80, 0x00, 0xf1, 0x00, 0x8b, 0x8c,
		0xf7, 0x00, 0x00, 0x00, 0x00, 0xf2, 0xf3, 0xf4,
		0xf5, 0xf6, 0x94, 0x89, 0x87, 0x88, 0x97, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	static const u8 *tbl[] = {
		normal, shift, kana_normal, kana_shift, graph, graph, graph, graph
	};
	u8 b, c, i, j, m = 0, d = inp(8);
	if (!(d & 0x40)) m |= 1;
	if (!(d & 0x20)) m |= 2;
	if (!(d & 0x10)) m |= 4;
	if (modifier != m) {
		modifier = m;
		clrKeyBuf();
	}
	for (i = 0; i < 11; i++) {
		d = inp(i);
		b = lastPort[i];
		for (m = ~d & b, j = 0; m; m >>= 1, j++)
			if (m & 1 && keyN < KEYN && (c = tbl[modifier][i << 3 | j])) {
				keyN++;
				*keyW++ = c;
				if (keyW >= keyBuf + KEYN) keyW = keyBuf;
			}
		for (m = d & ~b, j = 0; m; m >>= 1, j++)
			if (m & 1 && (c = tbl[modifier][i << 3 | j])) {
				u8 k, l;
				for (k = keyN; k > 0; k--)
					if (*keyR == c) {
						keyN--;
						u8 *sp = keyR + 1, *dp = keyR;
						for (l = k; l > 1; l--) {
							if (sp >= keyBuf + KEYN) sp = keyBuf;
							if (dp >= keyBuf + KEYN) dp = keyBuf;
							*dp++ = *sp++;
						}
						if (--keyW < keyBuf) keyW = keyBuf + KEYN - 1;
					}
			}
		lastPort[i] = d;
	}
	if (!keyN) c = 0;
	else {
		c = *keyR;
		if (keyN > 1) {
			if (++keyR >= keyBuf + KEYN) keyR = keyBuf;
			keyN--;
		}
	}
	return c;
}

u8 keyDown(u8 repeat) {
	u8 l, c = keyPress();
	if (c != lastKey) {
		lastKey = c;
		l = enterCritical();
		keyTarget = interruptCount + KEYWAIT;
		keyWait = 1;
		exitCritical(l);
		return c;
	}
	else if (repeat && !keyWait) {
		l = enterCritical();
		keyTarget = interruptCount + KEYWAITREPEAT;
		keyWait = 1;
		exitCritical(l);
		return c;
	}
	lastKey = c;
	return 0;
}

void input(u8 *buf, u16 len) {
	u8 *p = buf, *lim = buf + len - 1;
	while (1) {
		cursorOn();
		u8 c = keyDown(1);
		if (c == 8) {
			if (p > buf) {
				*--p = 0;
				if (--cursX == 0xff) {
					cursX = screenXn - 1;
					if (--cursY == 0xff) cursY = 0;
				}
				writeChar(0);
			}
		}
		else if (c == 13) {
			*p = 0;
			newLine();
			cursorOff();
			return;
		}
		else if (c >= ' ' && p < lim && !(cursX >= screenXn - 1 && cursY >= screenYn - 1)) {
			*p++ = c;
			putchar(c);
		}
	}
}

//// INIT ////

void baseInit(void) {
	memset(lastPort, 0xff, sizeof(lastPort));
	lastKey = modifier = 0;
	IO40 = last40 = 1;
	printer = 0;
	intFunc = idle;
	clrKeyBuf();
	setupScreen(40, 20, 0, 0, 0);
}
