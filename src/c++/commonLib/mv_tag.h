#pragma once

#include <stdint.h>

namespace pgnano
{

struct MVTag
{
    MVTag(void* ptr)
    {

        stride = *((int8_t*)ptr);
        unary_coded_offsets = (int8_t*)ptr + 1;
    }
    int8_t stride;
    int8_t* unary_coded_offsets;
};

};