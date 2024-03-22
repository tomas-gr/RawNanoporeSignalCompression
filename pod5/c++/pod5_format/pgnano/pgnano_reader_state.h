#pragma once

#include <memory>
#include "BAM_handler.h"
#include "level_table.h"

namespace pgnano
{

class PGNanoReaderState
{
public:
    PGNanoReaderState();
    std::unique_ptr<BAMHandler> m_bam_handler;
    std::unique_ptr<LevelTable> m_levels_table;
private:
};

};