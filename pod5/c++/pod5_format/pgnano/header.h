#pragma once

#include "metadata.h"

namespace pgnano
{

struct Header
{
public:
    pgnano::Metadata metadata;
    bool is_raw;
};

constexpr size_t header_size = sizeof(size_t) + sizeof(uint8_t) + sizeof(uint8_t);

}