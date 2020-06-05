#include "beep.h"
#include "utils.h"

__sfr __at 0x40 IO40;

extern u8 last40;

void beep(s16 len) {
	if (len < 0) IO40 = last40 |= 0x20;
	else if (!len) IO40 = last40 &= ~0x20;
	else {
		IO40 = last40 ^= 0x20;
		sleep(len);
		IO40 = last40 ^= 0x20;
	}
}
