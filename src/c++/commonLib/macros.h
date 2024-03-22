#pragma once

#define ASSING_FIRST_BYTE(lvalue, xvalue) \
lvalue = xvalue & 0xFF

#define ASSIGN_SECOND_BYTE(lvalue, xvalue) \
lvalue = (xvalue & 0xFF00) >> 8

#define BREAK_INTO_BYTES(first_byte, second_byte, xvalue) \
ASSING_FIRST_BYTE(first_byte, xvalue); \
ASSIGN_SECOND_BYTE(second_byte, xvalue);

#define SWAP_LITTLE_ENDIAN(x,y) \
x ^= y; \
y ^= x; \
x ^= y;
