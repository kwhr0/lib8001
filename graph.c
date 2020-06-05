#include "graph.h"
#include "base.h"

typedef void (*PointFunc)(u8 x, u8 y);

static u8 clr, chr;

void setClr(u8 f) {
	clr = f;
}

void point(u8 x, u8 y) {
	u8 cy = y >> 2;
	u8 *p = vramLine(cy);
	if (!p) return;
	u8 cx = x >> 1;
	setAttr(cx, cy, curAttr | (isColorMode ? 0x10 : 0x80));
	cx += isColorMode;
	if (width40) cx <<= 1;
	if (cx < 80) {
		u8 d = 1 << ((x & 1) << 2 | y & 3);
		if (clr) p[cx] &= ~d;
		else p[cx] |= d;
	}
}

static void point2(u8 x, u8 y) {
	point(x >> 1, y);
}

static void point_c(u8 x, u8 y) {
	u8 *p = vramLine(y);
	if (!p) return;
	setAttr(x, y, curAttr & (isColorMode ? 0xef : 0x7f));
	x += isColorMode;
	if (width40) x <<= 1;
	if (x < 80) p[x] = chr;
}

static void point_c2(u8 x, u8 y) {
	point_c(x, y >> 1);
}

static void linesub(u8 x0, u8 y0, u8 x1, u8 y1, PointFunc func) {
	u8 x = x0, y = y0, dx, dy, f;
	s8 sx, sy;
	s16 d;
	if (x0 < x1) {
		dx = x1 - x0;
		sx = 1;
	}
	else {
		dx = x0 - x1;
		sx = -1;
	}
	if (y0 < y1) {
		dy = y1 - y0;
		sy = 1;
	}
	else {
		dy = y0 - y1;
		sy = -1;
	}
	if (dx > dy) {
		d = dx >> 1;
		do {
			func(x, y);
			if ((d += dy) >= dx) {
				d -= dx;
				y += sy;
			}
			f = x != x1;
			x += sx;
		} while(f);
	}
	else {
		d = dy >> 1;
		do {
			func(x, y);
			if ((d += dx) >= dy) {
				d -= dy;
				x += sx;
			}
			f = y != y1;
			y += sy;
		} while (f);
	}
}

void line(u8 x0, u8 y0, u8 x1, u8 y1) {
	linesub(x0, y0, x1, y1, point);
}

void line_c(u8 x0, u8 y0, u8 x1, u8 y1, u8 c) {
	chr = c;
	linesub(x0, y0, x1, y1, point_c);
}

void boxfill(u8 x0, u8 y0, u8 x1, u8 y1) {
	static const u8 ma0[] = { 0xff, 0xee, 0xcc, 0x88 };
	static const u8 ma1[] = { 0x11, 0x33, 0x77, 0xff };
	u8 x, y, t, attr = curAttr | (isColorMode ? 0x10 : 0x80);
	if (x0 > x1) {
		t = x0;
		x0 = x1;
		x1 = t;
	}
	if (y0 > y1) {
		t = y0;
		y0 = y1;
		y1 = t;
	}
	for (y = y0 & 0xfc; y <= (y1 & 0xfc); y += 4) {
		u8 cy = y >> 2, m0 = 0xff;
		u8 *p = vramLine(cy);
		if (!p) continue;
		t = y0 - y;
		if (t < 4) m0 &= ma0[t];
		t = y1 - y;
		if (t < 4) m0 &= ma1[t];
		for (x = x0 & 0xfe; x <= (x1 & 0xfe); x += 2) {
			u8 cx = x >> 1, m = m0;
			if (x == x0 - 1) m &= 0xf0;
			else if (x == x1) m &= 0xf;
			setAttr(cx, cy, attr);
			cx += isColorMode;
			if (width40) cx <<= 1;
			if (cx < 80)
				if (clr) p[cx] &= ~m;
				else p[cx] |= m;
		}
	}
}

void boxfill_c(u8 x0, u8 y0, u8 x1, u8 y1, u8 c) {
	u8 x, y, t;
	chr = c;
	if (x0 > x1) {
		t = x0;
		x0 = x1;
		x1 = t;
	}
	if (y0 > y1) {
		t = y0;
		y0 = y1;
		y1 = t;
	}
	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++)
			point_c(x, y);
}

void box(u8 x0, u8 y0, u8 x1, u8 y1) {
	boxfill(x0, y0, x1, y0);
	boxfill(x0, y1, x1, y1);
	boxfill(x0, y0, x0, y1);
	boxfill(x1, y0, x1, y1);
}

void box_c(u8 x0, u8 y0, u8 x1, u8 y1, u8 c) {
	chr = c;
	linesub(x0, y0, x1, y0, point_c);
	linesub(x0, y1, x1, y1, point_c);
	linesub(x0, y0, x0, y1, point_c);
	linesub(x1, y0, x1, y1, point_c);
}

static void circlesub(u8 x0, u8 y0, u8 r, PointFunc func) {
	if (!r) return;
	u8 x = r, y = 0;
	s16 f = (-r << 1) + 3;
	while (x >= y) {
		func(x0 + x, y0 + y);
		func(x0 - x, y0 + y);
		func(x0 + x, y0 - y);
		func(x0 - x, y0 - y);
		func(x0 + y, y0 + x);
		func(x0 - y, y0 + x);
		func(x0 + y, y0 - x);
		func(x0 - y, y0 - x);
		if (f >= 0) {
			x--;
			f -= x << 2;
		}
		y++;
		f += (y << 2) + 2;
	}
}

void circle(u8 x0, u8 y0, u8 r) {
	if (width40) circlesub((x0 << 1) + 1, y0, r, point2);
	else circlesub(x0, y0, r, point);
}

void circle_c(u8 x0, u8 y0, u8 r, u8 c) {
	chr = c;
	if (width40) circlesub(x0, y0, r, point_c);
	else circlesub(x0, (y0 << 1) + 1, r, point_c2);
}
