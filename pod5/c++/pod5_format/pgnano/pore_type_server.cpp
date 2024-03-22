#include "pore_type_server.h"

namespace pgnano
{

void pgnano::PoreTypeServer::put_pore_type(pod5::PoreDictionaryIndex idx, const std::string & pore_description) 
{
    PGNANO_PORE_TYPE pore_type = m_parser.parse_pore_type(pore_description);
    std::lock_guard<std::mutex> lk(m_idx_map_mtx);
    m_idx_map.insert({idx,pore_type});
}

pgnano::PGNANO_PORE_TYPE pgnano::PoreTypeServer::get_pore_type(pod5::PoreDictionaryIndex idx) 
{
    // FIXME: Program will terminate if the user did not add the pore type first.
    std::lock_guard<std::mutex> lk(m_idx_map_mtx);
    return m_idx_map.at(idx);
}

};