#include "pgnano_reader_state.h"

pgnano::PGNanoReaderState::PGNanoReaderState()
{
    m_bam_handler = std::make_unique<BAMHandler>();
    m_levels_table = std::make_unique<LevelTable>();
}