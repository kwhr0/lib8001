#include "sprite.h"
#include "base.h"

extern u8 bitmap[];

static SpriteContext defaultCtx;
static SpriteContext *ctx = &defaultCtx;

void spriteContext(SpriteContext *c) {
	ctx = c ? c : &defaultCtx;
}

void spriteSetup(Sprite *s, u8 n, u8 stride) {
	if (!s) return;
	ctx->free = s;
	ctx->active = nil;
	for (u8 i = 0; i < n - 1; i++) {
		Sprite *next = (Sprite *)((u8 *)s + stride);
		s->next = next;
		s = next;
	}
	s->next = nil;
	ctx->vramofs = isColorMode;
	ctx->xlim = 80 - isColorMode;
	ctx->ylim = 25;
}

void spriteView(u8 left, u8 top, u8 width, u8 height) {
	left += isColorMode;
	if (left > 79) left = 79;
	if (top > 24) top = 24;
	if (left + width > 80) width = 80 - left;
	if (top + height > 25) height = 25 - top;
	ctx->vramofs = 120 * top + left;
	ctx->xlim = width;
	ctx->ylim = height;
}

void spriteAnim(Sprite *s, Pattern *pat) {
	if (!s) return;
	s->pat = pat;
	s->curpat = bitmap + pat->ofs[0];
	s->frame = 0;
	s->animSpeed = 1 << AS;
}

Sprite *spriteCreate(u8 prio, Pattern *pat) {
	Sprite *s = ctx->free;
	if (!s) return nil;
	s->behavior = nil;
	s->x = s->y = s->speedX = s->speedY = 0;
	s->insetLeft = s->insetRight = s->insetTop = s->insetBottom = 0;
	s->prio = prio;
	s->attr = ATTR_INVALID;
	s->flags = s->hitMask = 0;
	spriteAnim(s, pat);
	ctx->free = s->next;
	Sprite *s0 = nil, *s1 = ctx->active;
	for (; s1 && s1->prio <= s->prio; s0 = s1, s1 = s1->next)
		;
	if (s0) {
		s->next = s0->next;
		s0->next = s;
	}
	else {
		s->next = ctx->active;
		ctx->active = s;
	}
	return s;
}

u8 spriteVisible(Sprite *s) {
	if (!s) return 0;
	s16 x = s->x >> PS, y = s->y >> PS;
	return x > -s->pat->pw && x < 160 - isColorMode && y > -s->pat->ph && y < 100;
}

u8 spriteHits(Sprite *s, Sprite *result[], u8 n) {
	if (!s) return 0;
	s16 x0 = s->x >> PS, y0 = s->y >> PS;
	s16 x1 = x0 + s->pat->pw, y1 = y0 + s->pat->ph;
	x0 += s->insetLeft;
	x1 -= s->insetRight;
	y0 += s->insetTop;
	y1 -= s->insetBottom;
	u8 c = 0;
	for (Sprite *s1 = ctx->active; s1 && c < n; s1 = s1->next) {
		if (s == s1 || !(s->hitMask & s1->hitMask)) continue;
		s16 x10 = s1->x >> PS, y10 = s1->y >> PS;
		s16 x11 = x10 + s1->pat->pw, y11 = y10 + s1->pat->ph;
		x10 += s1->insetLeft;
		x11 -= s1->insetRight;
		y10 += s1->insetTop;
		y11 -= s1->insetBottom;
		if (x0 <= x11 && x1 >= x10 && y0 <= y11 && y1 >= y10) result[c++] = s1;
	}
	return c;
}

Sprite *spriteHit(Sprite *s) {
	Sprite *a[1];
	return spriteHits(s, a, 1) ? *a : nil;
}

u8 spriteCount(Pattern *pat) {
	u8 n = 0;
	for (Sprite *s = ctx->active; s; s = s->next) n += !pat || s->pat == pat;
	return n;
}

void spriteFrame(Sprite *s, s16 frame) {
	if (!s) return;
	if (frame < 0) frame = 0;
	else if (frame >= s->pat->n << AS) frame = (s->pat->n << AS) - 1;
	s->frame = frame;
	s->curpat = bitmap + s->pat->ofs[frame >> AS];
}

void spriteDraw(u8 *src, u8 *dst, u8 x, u8 y, u8 xcount, u8 ycount, u8 xlimit, u8 ylimit) CC0;

void spriteUpdate(void) {
	Sprite *s = ctx->active, *s0 = nil;
	while (s) {
		if (s->behavior ? s->behavior(s) : 
			(s->animSpeed || !(s->flags & SF_ERASE_NO_ANIM)) && spriteVisible(s)) {
			if (!(s->flags & SF_HIDDEN)) {
				spriteDraw(s->curpat, vram + ctx->vramofs, 
					s->x >> PS, s->y >> PS, s->pat->w, s->pat->h, 
					ctx->xlim, ctx->ylim);
				if (attrIsValid(s->attr)) {
					s16 x, y, x0 = s->x >> PS + 1, y0 = s->y >> PS + 2;
					for (y = y0; y < y0 + s->pat->h; y++)
						for (x = x0; x < x0 + s->pat->w; x++)
							setAttr(x, y, s->attr);
				}
			}
			s->x += s->speedX;
			s->y += s->speedY;
			s16 f = s->frame + s->animSpeed;
			u8 frameN = s->pat->n;
			if (s->animSpeed > 0) {
				if (f >= frameN << AS) 
					if (s->flags & SF_PALINDROME) {
						s->animSpeed = -s->animSpeed;
						f += s->animSpeed << 1;
					}
					else if (s->flags & SF_LOOP) f -= frameN << AS;
					else {
						f = (frameN << AS) - 1;
						s->animSpeed = 0;
					}
			}
			else if (f < 0) 
				if (s->flags & SF_LOOP)
					if (s->flags & SF_PALINDROME) {
						s->animSpeed = -s->animSpeed;
						f += s->animSpeed << 1;
					}
					else f += frameN << AS;
				else {
					f = 0;
					s->animSpeed = 0;
				}
			spriteFrame(s, f);
			s0 = s;
			s = s->next;
		}
		else {
			if (s0) s0->next = s->next;
			else ctx->active = s->next;
			s->next = ctx->free;
			ctx->free = s;
			s = s0 ? s0->next : ctx->active;
		}
	}
}
