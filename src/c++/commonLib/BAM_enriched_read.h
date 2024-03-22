#pragma once

#include <memory>
#include "concepts.h"

namespace pgnano 
{

class BAMEnrichedRead
{
public:
    BAMEnrichedRead(const uint8_t* mv_tag, const uint8_t* sequence, int sequence_byte_length, unsigned int mv_tag_byte_length);

    ~BAMEnrichedRead() {};

    inline bool has_mv_tag() const noexcept { return m_mv_tag != nullptr; }
    inline bool has_sequence() const noexcept { return m_sequence != nullptr; }
    inline bool is_complete() const noexcept { return has_mv_tag() && is_complete(); }

    bool next();//TODO: iterator_ended?
    bool is_jump();
    PackedNucleotide current_nucleotide();

private:
    std::unique_ptr<uint8_t[]> m_mv_tag;//TODO: encapsular en clase "Iterador"
    std::unique_ptr<uint8_t[]> m_sequence;
    uint8_t m_stride;
    uint8_t* m_mv_tag_ptr;
    uint8_t* m_sequence_ptr;
};

};
/*
FIXME: mv_tag is a int8_t ptr, not a uint8_t ptr
*/