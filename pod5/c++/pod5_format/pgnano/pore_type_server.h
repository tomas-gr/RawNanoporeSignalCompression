#pragma once

#include <unordered_map>
#include <mutex>

#include "pore_type_parser.h"
#include "pod5_format/read_table_utils.h"

namespace pgnano
{

class PoreTypeServer
{
public:
    void put_pore_type(pod5::PoreDictionaryIndex idx, std::string const & pore_description);
    PGNANO_PORE_TYPE get_pore_type(pod5::PoreDictionaryIndex idx);

private:
    std::unordered_map<pod5::PoreDictionaryIndex, PGNANO_PORE_TYPE> m_idx_map;//TODO: podes hacerlo con un Array y evitar locks. Si los indices no estan ordenados entonces no funciona
    std::mutex m_idx_map_mtx; //TODO: readers writers lock
    PoreTypeParser m_parser;
};

};