#include "types.h"

#define graphInit()		(setClr(0))

void setClr(u8);
void point(u8 x, u8 y);
void line(u8 x0, u8 y0, u8 x1, u8 y1);
void line_c(u8 x0, u8 y0, u8 x1, u8 y1, u8 c);
void box(u8 x0, u8 y0, u8 x1, u8 y1);
void box_c(u8 x0, u8 y0, u8 x1, u8 y1, u8 c);
void boxfill(u8 x0, u8 y0, u8 x1, u8 y1);
void boxfill_c(u8 x0, u8 y0, u8 x1, u8 y1, u8 c);
void circle(u8 x0, u8 y0, u8 r);
void circle_c(u8 x0, u8 y0, u8 r, u8 c);
