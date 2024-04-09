#ifndef KD_HPP
#define KD_HPP


#include <arrow/buffer.h>

#include "compressor.h"
#include "decompressor.h"
#include "macros.h"
#include "header.h"
#include "BAM_handler.h"

#include "pod5_format/types.h"
#include "pod5_format/read_table_utils.h"
#include "pod5_format/pgnano/pgnano_writer_state.h"

#include "pod5_format/signal_compression.h"

#include <arrow/buffer.h>
#include <zstd.h>

#include "common.hpp"
#include "svb16.h"  

#include "pod5_format/pgnano/pgnano_utils.hpp"

#include <chrono>

#endif 

extern long full_size_keys;
extern long full_size_S;
extern long full_size_M;
extern long full_size_Llow;
extern long full_size_Lhigh;
extern long full_size_data;

extern long comp_size_keys;
extern long comp_size_S;
extern long comp_size_M;
extern long comp_size_Llow;
extern long comp_size_Lhigh;
extern long comp_size_data;

extern long total_samples;

extern double compression_time;
extern double decompression_time;

namespace svb16 {

struct buf_sizes_KD {
    size_t keys_sz, data_sz;
};


template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_KD encode_scalar_KD(
        Int16T const * in,
        uint8_t * SVB_RESTRICT keys,
        uint8_t * SVB_RESTRICT data,
        uint32_t count,
        Int16T prev = 0)
    {
        if (count == 0) {
            return {0,0};
        }

        uint8_t * keys_begin = keys;
        uint8_t * data_begin = data;

        uint8_t shift = 0;  
        uint8_t key_byte = 0;
        for (uint32_t c = 0; c < count; c++) {
            if (shift == 8) {
                shift = 0;
                *keys++ = key_byte;
                key_byte = 0;
            }

            uint16_t value;
            SVB16_IF_CONSTEXPR(UseDelta)
            {
                value = static_cast<uint16_t>(in[c]) - static_cast<uint16_t>(prev);
                SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_encode(value); }
                prev = in[c];
            }
            else SVB16_IF_CONSTEXPR(UseZigzag) {
                value = detail::zigzag_encode(static_cast<uint16_t>(in[c]));
            }
            else {
                value = static_cast<uint16_t>(in[c]);
            }

            if (value < (1 << 8)) {  
                *data = static_cast<uint8_t>(value);
                ++data;
            } else {                           
                std::memcpy(data, &value, 2);  // assumes little endian
                data += 2;
                key_byte |= 1 << shift;
            }

            shift += 1;
        }

        *keys++ = key_byte;  // write last key (increment needed)
        return {(size_t) (keys - keys_begin), (size_t) (data - data_begin)};
    }


template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_KD encode_KD(Int16T const * in, uint8_t * SVB_RESTRICT out, uint32_t count, Int16T prev = 0)
    {
        auto const keys = out;
        auto const data = keys + ::svb16_key_length(count);

        return encode_scalar_KD<Int16T, UseDelta, UseZigzag>(in, keys, data, count, prev);
    }

namespace detail{

inline uint16_t decode_data_KD(gsl::span<uint8_t const>::iterator & dataPtr, uint8_t code)
{
    uint16_t val;

    if (code == 0) {  // 1 byte
        val = (uint16_t)*dataPtr;
        dataPtr += 1;
    } else {  // 2 bytes
        val = 0;
        memcpy(&val, dataPtr, 2);  // assumes little endian
        dataPtr += 2;
    }

    return val;
}
}  // namespace detail

template <typename Int16T, bool UseDelta, bool UseZigzag>
uint8_t const * decode_scalar_KD(
    gsl::span<Int16T> out_span,
    gsl::span<uint8_t const> keys_span,
    gsl::span<uint8_t const> data_span,
    Int16T prev = 0)
{
    auto const count = out_span.size();
    if (count == 0) {
        return data_span.begin();
    }

    auto out = out_span.begin();
    auto keys = keys_span.begin();
    auto data = data_span.begin();

    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = *keys++;
    // need to do the arithmetic in unsigned space so it wraps
    auto u_prev = static_cast<uint16_t>(prev);
    for (uint32_t c = 0; c < count; c++, shift++) {
        if (shift == 8) {
            shift = 0;
            key_byte = *keys++;
        }
        uint16_t value = detail::decode_data_KD(data, (key_byte >> shift) & 0x01);
        SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_decode(value); }
        SVB16_IF_CONSTEXPR(UseDelta)
        {
            value += u_prev;
            u_prev = value;
        }
        *out++ = static_cast<Int16T>(value);
    }

    assert(out == out_span.end());
    assert(keys == keys_span.end());
    assert(data <= data_span.end());
    return data;
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
size_t decode_KD(gsl::span<Int16T> out, gsl::span<uint8_t const> in, uint32_t d, Int16T prev = 0)
{
    auto keys_length = ::svb16_key_length(out.size());
    auto const keys = in.subspan(0, keys_length);
    auto const data = in.subspan(keys_length);

#ifdef SVB16_X64
    //if (has_sse4_1()) {
    //    return decode_sse<Int16T, UseDelta, UseZigzag>(out, keys, data, prev) - in.begin();
    //}
#endif
    return decode_scalar_KD<Int16T, UseDelta, UseZigzag>(out, keys,data, prev) - in.begin();
}


} // namespace svb16


namespace pgnano{

arrow::Result<std::size_t> compress_signal_KD(  // COMPRESOR KD //
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    gsl::span<std::uint8_t> const & destination,
    pod5::ReadData const & read_data,
    bool is_last_batch)
{

    auto t1 = std::chrono::steady_clock::now();
    
    // --------------- First compress the samples using svb --------------- //
    auto const max_size = svb16_max_encoded_length(samples.size());
    ARROW_ASSIGN_OR_RAISE(auto intermediate, arrow::AllocateResizableBuffer(max_size, pool));

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto const encoded_sz = svb16::encode_KD<int16_t, UseDelta, UseZigzag>(
        samples.data(), intermediate->mutable_data(), samples.size());
    auto const encoded_count = encoded_sz.keys_sz + encoded_sz.data_sz;
    ARROW_RETURN_NOT_OK(intermediate->Resize(encoded_count));

    full_size_keys += encoded_sz.keys_sz;
    full_size_data += encoded_sz.data_sz;

    total_samples += samples.size();

    // --------------- Separate keys and data --------------- //
    auto const keys_size = ::svb16_key_length(samples.size());
    auto keys_span = gsl::span<uint8_t>(intermediate->mutable_data(), keys_size);
    auto data_span = gsl::span<uint8_t>(intermediate->mutable_data() + keys_size, encoded_count - keys_size);

    // --------------- Now compress the keys using zstd --------------- //
    size_t const zstd_keys_max_size = ZSTD_compressBound(keys_span.size());
    if (ZSTD_isError(zstd_keys_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for keys");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_keys_result = arrow::AllocateResizableBuffer(zstd_keys_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_keys, compressed_keys_result);

    size_t const compressed_keys_size = ZSTD_compress(
        compressed_keys->mutable_data(), compressed_keys->size(),
        keys_span.data(), keys_span.size(), 1);
    if (ZSTD_isError(compressed_keys_size)) {
        return pod5::Status::Invalid("Failed to compress keys");
    }

    // --------------- Now compress the data using zstd --------------- //
    size_t const zstd_data_max_size = ZSTD_compressBound(data_span.size());
    if (ZSTD_isError(zstd_data_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_result = arrow::AllocateResizableBuffer(zstd_data_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data, compressed_data_result);
 
    size_t const compressed_data_size = ZSTD_compress(
        compressed_data->mutable_data(), compressed_data->size(),
        data_span.data(), data_span.size(), 1);
    if (ZSTD_isError(compressed_data_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

    // Copy the size of compressed keys at the beginning of the destination buffer
    std::memcpy(destination.data(), &compressed_keys_size, sizeof(compressed_keys_size));

    // Copy compressed keys after the size in the destination buffer
    std::memcpy(destination.data() + sizeof(compressed_keys_size), compressed_keys->data(), compressed_keys_size);

    // Copy compressed data after the compressed keys in the destination buffer
    std::memcpy(destination.data() + sizeof(compressed_keys_size) + compressed_keys_size, compressed_data->data(), compressed_data_size);

    auto t2 = std::chrono::steady_clock::now();

    compression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();

    comp_size_keys += sizeof(compressed_keys_size) + compressed_keys_size;
    comp_size_data += compressed_data_size;

    return compressed_keys_size + compressed_data_size + sizeof(compressed_keys_size);
}



pod5::Status decompress_signal_KD(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination,
    pgnano::PGNanoReaderState & state)
{

    auto t1d = std::chrono::steady_clock::now();

    auto read_ptr = compressed_bytes.data();

    // Extract the size of compressed keys from the beginning of the compressed_bytes    
    std::size_t compressed_keys_size;
    std::memcpy(&compressed_keys_size, read_ptr, sizeof(compressed_keys_size));
    read_ptr += sizeof(compressed_keys_size);

    unsigned long long const decompressed_zstd_keys_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_keys_size);
    if (ZSTD_isError(decompressed_zstd_keys_size)) {
        return pod5::Status::Invalid(
            "Keys Input data not compressed by zstd: (",
            decompressed_zstd_keys_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_keys_size),
            ")");
    }
    read_ptr += compressed_keys_size;

    // Extract the size of compressed data
    std::size_t compressed_data_size = compressed_bytes.end() - read_ptr; 


    unsigned long long const decompressed_zstd_data_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_data_size);
    if (ZSTD_isError(decompressed_zstd_data_size)) {
        return pod5::Status::Invalid(
            "data Input data not compressed by zstd: (",
            decompressed_zstd_data_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_size),
            ")");
    }

    auto allocation_padding = svb16::decode_input_buffer_padding_byte_count();
    ARROW_ASSIGN_OR_RAISE(
        auto intermediate,
        arrow::AllocateResizableBuffer(decompressed_zstd_keys_size + 
                                    decompressed_zstd_data_size + 
                                    allocation_padding, pool));
    
    size_t const decompress_res_keys = ZSTD_decompress(
        intermediate->mutable_data(),
        decompressed_zstd_keys_size,
        compressed_bytes.data() + sizeof(compressed_keys_size),
        compressed_keys_size);
    if (ZSTD_isError(decompress_res_keys)) {
        return pod5::Status::Invalid(
            "keys Input data failed to decompress using zstd: (",
            decompress_res_keys,
            " ",
            ZSTD_getErrorName(decompress_res_keys),
            ")");
    }    
    
    size_t const decompress_res_data = ZSTD_decompress(
        intermediate->mutable_data() + decompressed_zstd_keys_size,
        decompressed_zstd_data_size,
        compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size,
        compressed_data_size);
    if (ZSTD_isError(decompress_res_data)) {
        return pod5::Status::Invalid(
            "data Input data failed to decompress using zstd: (",
            decompress_res_data,
            " ",
            ZSTD_getErrorName(decompress_res_data),
            ")");
    }  

    // Now decompress the data using svb:
    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto consumed_count = svb16::decode_KD<int16_t, UseDelta, UseZigzag>(destination, gsl::make_span(intermediate->data(), 
        intermediate->size()), decompress_res_data, 0);
    if ((consumed_count + allocation_padding) != (std::size_t)intermediate->size()) {
        return pod5::Status::Invalid("Remaining data at end of signal buffer");
    }

    auto t2d = std::chrono::steady_clock::now();
    
    decompression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2d - t1d).count();

    return pod5::Status::OK();
}


} // namespace pgnano
