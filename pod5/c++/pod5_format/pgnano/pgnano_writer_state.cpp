#include "pgnano_writer_state.h"

pgnano::PGNanoWriterState::PGNanoWriterState()
{
    m_pore_type_server = std::make_unique<pgnano::PoreTypeServer>();
    m_bam_handler = std::make_unique<pgnano::BAMHandler>();
    m_levels_table = std::make_unique<pgnano::LevelTable>();
}