#include "types.h"

typedef struct Time {
	u8 month, day, hour, minute, second;
} Time;

void getTime(Time *time);
void setTime(Time *time);
