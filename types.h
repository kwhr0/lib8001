#pragma once

#ifndef nil
#define nil	0
#endif

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

#ifdef __SDCC
__sfr __at 0 KEY0;
__sfr __at 1 KEY1;
__sfr __at 2 KEY2;
__sfr __at 3 KEY3;
__sfr __at 4 KEY4;
__sfr __at 5 KEY5;
__sfr __at 6 KEY6;
__sfr __at 7 KEY7;
__sfr __at 8 KEY8;
__sfr __at 9 KEY9;
#endif
