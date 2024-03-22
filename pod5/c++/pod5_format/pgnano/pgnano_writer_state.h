#pragma once

#include <memory>
#include "pore_type_server.h"
#include "BAM_handler.h"
#include "level_table.h"

namespace pgnano
{

class PGNanoWriterState
{
public:
    PGNanoWriterState();
    //PGNanoWriterState(const PGNanoWriterState &) = delete;
    //PGNanoWriterState& operator= (const PGNanoWriterState &) = delete;
    //PGNanoWriterState& operator= (PGNanoWriterState &&) = default;

    // FIXME: DON'T MAKE THESE MEMBERS PUBLIC
    std::unique_ptr<PoreTypeServer> m_pore_type_server;
    std::unique_ptr<BAMHandler> m_bam_handler;
    std::unique_ptr<LevelTable> m_levels_table;
private:
};

};