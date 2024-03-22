#pragma once

#include <stdint.h>
#include <memory>

namespace pgnano
{

class BitStreamIterator
{
public:
    BitStreamIterator(const uint8_t* mv_tag, unsigned int mv_tag_byte_length)
    {

    }

    // Advance iterator and return whether we are at the end of the iterator
    // Returns true if there are still more elements to process
    inline bool next() noexcept
    {
        return true;
    }
    // Query the current value without advancing the iterator
    inline bool peek() const noexcept { return true; }
private:
//    std::unique_ptr<uint8_t[]> m_bit_stream;
//    uint8_t *m_current_ptr, *m_end_ptr;
//    static constexpr uint_fast8_t mask = 0x80;
//    uint_fast8_t m_current_buffer;
//    uint_fast8_t m_bits_in_buffer;
};

};

/*
TODO: Increase the buffer size...
TODO: use AVX-512 and memory prefetching to improve performance
*/