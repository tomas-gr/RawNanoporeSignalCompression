#include "codecs.h"

namespace pgnano
{

uint16_t signed_encode(int16_t x)
{
    if (x >= 0)
        return x << 1;
    else
        return ((uint16_t)(-x) << 1) - 1;
}

int16_t signed_decode(uint16_t x)
{
    if (x & 0x1)
        return -((x + 1) >> 1);
    else
        return x >> 1;
}

}