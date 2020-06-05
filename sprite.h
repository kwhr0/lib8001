#include "types.h"

#define SF_LOOP			1
#define SF_PALINDROME		2
#define SF_ERASE_NO_ANIM	4
#define SF_HIDDEN	8
#define PS			4
#define AS			4

typedef struct {
	u8 n, pw, ph, w, h;
	u16 ofs[0];
} Pattern;

typedef struct Sprite {
	struct Sprite *next;
	u8 (*behavior)(struct Sprite *);
	Pattern *pat;
	u8 *curpat;
	s16 x, y;		// S11.4(PS)
	s8 speedX, speedY;	// S3.4(PS)
	u8 flags, attr, hitMask, prio;
	s8 insetLeft, insetRight, insetTop, insetBottom;
	s16 frame;		// S11.4(AS)
	s8 animSpeed;	// S3.4(AS)
} Sprite;

typedef struct {
	Sprite *free, *active;
	u16 vramofs;
	u8 xlim, ylim;
} SpriteContext;

void spriteContext(SpriteContext *c);
void spriteSetup(Sprite *s, u8 n, u8 stride);
void spriteView(u8 left, u8 top, u8 width, u8 height);
Sprite *spriteCreate(u8 prio, Pattern *anim);
void spriteAnim(Sprite *s, Pattern *anim);
u8 spriteVisible(Sprite *s);
u8 spriteCount(Pattern *anim);
Sprite *spriteHit(Sprite *s);
u8 spriteHits(Sprite *s, Sprite *result[], u8 n);
void spriteFrame(Sprite *s, s16 frame);
void spriteUpdate();

#define spriteInit()			(spriteContext(0))
#define spriteSetupArray(s)		(spriteSetup((Sprite *)s, sizeof(s) / sizeof(s[0]), sizeof(s[0])))
#define spriteHitArray(s, a)	(spriteHits((Sprite *)s, a, sizeof(a) / sizeof(a[0])))
#define spriteColor(s, c)	((s)->attr = (c) << 5 | 0x18)
