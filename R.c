#include "R.h"
#include <stdlib.h>

u16 R(u16 n) {
	return (long)n * rand() >> 15;
}
