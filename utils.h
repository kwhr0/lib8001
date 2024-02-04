#include "types.h"

void sleep(u16 ms) CC0;
u8 inp(u8 port) CC0;
u8 getR(void) CC0;
u8 enterCritical(void) CC0;
void exitCritical(u8 last) CC0;
void idle(void);
