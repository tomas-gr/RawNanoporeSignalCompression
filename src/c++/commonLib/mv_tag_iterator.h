#pragma once

#include "bit_stream_iterator.h"

namespace pgnano
{

class MVTagIterator
{
public:
    MVTagIterator(const uint8_t* mv_tag, unsigned int mv_tag_byte_length)
    : m_stride(*mv_tag),
    m_reads_until_next_bit(m_stride),
    m_bitstream_iterator(++mv_tag, --mv_tag_byte_length)
    {}
    // Advance iterator and return whether we are at the end of the iterator
    // Returns true if there are still more elements to process
    inline constexpr bool next() noexcept
    {
        return true;
    }
    // Should we switch nucleotides?
    // Does not advance the iterator
    inline constexpr bool is_jump() const noexcept { return true; };
private:
    uint_fast8_t m_stride;
    uint_fast8_t m_reads_until_next_bit;
    BitStreamIterator m_bitstream_iterator;
};

};
