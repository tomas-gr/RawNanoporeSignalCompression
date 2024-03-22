#pragma once

#include "common.hpp"
#include "decode_scalar.hpp"
#include "svb16.h"  // svb16_key_length
#ifdef SVB16_X64
#include "decode_x64.hpp"
#include "simd_detect_x64.hpp"
#endif

namespace svb16 {

// Required extra space after readable buffers passed in.
//
// Require 1 128 bit buffer beyond the end of all input readable buffers.
inline std::size_t decode_input_buffer_padding_byte_count()
{
//#ifdef SVB16_X64
//  return sizeof(__m128i);
//#else
    return 0;
//#endif
}

}  // namespace svb16