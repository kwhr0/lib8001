#include "types.h"

#define SCREEN_XN		80
#define SCREEN_YN		25
#define ATTR_N			20
#define VRAM_XN			(SCREEN_XN + 2 * ATTR_N)
#define ATTR_INVALID	0xff
#define attrIsValid(a)	(((a) | 0x90) != ATTR_INVALID)

typedef void (*IntFunc)(void);

void baseInit(void);
void setupScreen(u8 width, u8 height, u8 colorMode, u8 color, u8 graph);
void cls(void);
void color(s8 color);
void locate(u8 x, u8 y);
void setPrinter(u8 f);
int putchar(int c);
void puts_n(char *p);

u8 *vramLine(u8 y);
void setAttr(u8 x, u8 y, u8 attr);
void cursorOn(void);
void cursorOff(void);
void intSet(IntFunc func);
u8 waitVSync(u8 frames);
u8 vramSwap(u8 frames);
void vramSingle(void);

u8 keyPress(void);
u8 keyDown(u8 repeat);
void input(u8 *buf, u16 len);

extern u8 *vram;
extern u8 width40, curAttr, isColorMode, last40, cursX, cursY;
