#include "types.h"

void sleep(u16 ms);
u8 inp(u8 port);
u8 getR();
u8 enterCritical();
void exitCritical(u8 last);
void idle();
