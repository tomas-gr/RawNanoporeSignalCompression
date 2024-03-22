#ifndef PGNANO_UTILS_HPP
#define PGNANO_UTILS_HPP

#include "pod5_format/pgnano/svb16/svb16.h"
#include <zstd.h>


#endif  // PGNANO_UTILS_HPP

namespace pgnano {

// std::size_t compressed_signal_max_size(std::size_t sample_count)
// {
//     auto const max_svb_size = svb16_max_encoded_length(sample_count);
//     auto const zstd_compressed_max_size = ZSTD_compressBound(max_svb_size);
//     return zstd_compressed_max_size;
// }

} // namespace pgnano

namespace svb16 {
namespace detail {

// inline uint16_t zigzag_encode(uint16_t val)
// {
//     return (val + val) ^ static_cast<uint16_t>(static_cast<int16_t>(val) >> 15);
// }

// inline uint16_t zigzag_decode(uint16_t val)
// {
//     return (val >> 1) ^ static_cast<uint16_t>(0 - (val & 1));
// }

}  // namespace detail

} // namespace svb16