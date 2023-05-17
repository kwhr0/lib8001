#include "_time.h"

__sfr __at 0x10 OUT10;
__sfr __at 0x40 IO40;

extern u8 last40;

/*static*/inline u8 bcdtobin1(u8 d) {
	return (d >> 4) * 10 + (d & 15);
}

static u8 bintobcd1(u8 d) {
	u8 t, r = 0;
	do {
		t = d;
		d -= 10;
		r++;
	} while (d < t);
	return (r - 1 << 4) + d + 10;
}

void getTime(Time *time) {
	if (!time) return;
	u8 t[5];
	u8 *p = t;
	OUT10 = 3;
	IO40 = last40 | 2;
	OUT10 = 1;
	IO40 = last40 | 2;
	for (u8 i = 0; i < 40; i++) {
		*p = *p >> 1 | IO40 << 3 & 0x80;
		if ((i & 7) == 7) p++;
		IO40 = last40 | 4;
	}
	OUT10 = 3;
	IO40 = last40 | 2;
	IO40 = last40;
	time->month = *--p >> 4;
	time->day = bcdtobin1(*--p);
	time->hour = bcdtobin1(*--p);
	time->minute = bcdtobin1(*--p);
	time->second = bcdtobin1(*--p);
}

void setTime(Time *time) {
	if (!time) return;
	u8 i, d = 0;
	u8 t[5];
	u8 *p = t + 5;
	*--p = bintobcd1(time->month);
	*--p = bintobcd1(time->day);
	*--p = bintobcd1(time->hour);
	*--p = bintobcd1(time->minute);
	*--p = bintobcd1(time->second);
	OUT10 = 1;
	IO40 = last40 | 2;
	for (i = 0; i < 40; i++) {
		if (!(i & 7)) d = *p++;
		OUT10 = d << 3;
		IO40 = last40 | 4;
		d >>= 1;
	}
	OUT10 = 2;
	IO40 = last40 | 2;
	OUT10 = 3;
	IO40 = last40 | 2;
	IO40 = last40;
}
