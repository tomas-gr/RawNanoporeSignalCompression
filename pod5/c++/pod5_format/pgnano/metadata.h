#pragma once

#include "pod5_format/read_table_utils.h"
#include "known_pore_types.h"

namespace pgnano
{

struct Metadata
{
public:
    pod5::ReadData m_read_data;
    size_t samples;
    pgnano::PGNANO_PORE_TYPE pore_type = pgnano::PGNANO_PORE_TYPE::UNKNOWN;
};

}