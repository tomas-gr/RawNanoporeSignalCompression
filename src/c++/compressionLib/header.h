#pragma once

#include <stdint.h>

namespace pgnano { namespace standalone 
{
constexpr size_t header_bytes = sizeof(uint32_t) + sizeof(uint8_t);
constexpr size_t header_words = (header_bytes >> 1) + (header_bytes & 1);

class Header
{
public:
    uint32_t sample_count;    
    uint8_t is_raw;
};

}};