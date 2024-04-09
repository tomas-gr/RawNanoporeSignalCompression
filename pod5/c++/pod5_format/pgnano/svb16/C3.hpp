#ifndef LL_LH_HPP
#define LL_LH_HPP

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

#include "pod5_format/pgnano/pgnano_utils.hpp"

#include <chrono>

#include <arrow/buffer.h>
#include <zstd.h>

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

//extern double decompression_time;

namespace svb16 {

struct buf_sizes_ll_lh {
    size_t keys_sz, data_ll_sz, data_lh_sz, data_h_sz;
};

template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_ll_lh encode_scalar_ll_lh(
    Int16T const * in,
    uint8_t * SVB_RESTRICT keys_buf,
    uint8_t * SVB_RESTRICT data_ll_buf,
    uint8_t * SVB_RESTRICT data_lh_buf,
    uint8_t * SVB_RESTRICT data_h_buf,
    uint32_t count,
    Int16T prev = 0)
{
    
    if (count == 0) {
        return {0, 0, 0 ,0};
    }
    uint8_t * keys_buf_ini = keys_buf;
    uint8_t * data_ll_ini = data_ll_buf;
    uint8_t * data_lh_ini = data_lh_buf;
    uint8_t * data_h_ini = data_h_buf;

    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = 0;
   
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keys_buf++ = key_byte;
            key_byte = 0;
        }

        uint16_t value;
        SVB16_IF_CONSTEXPR(UseDelta)
        {
            // need to do the arithmetic in unsigned space so it wraps
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

        if (value >= (1 << 8)) {    // 2 bytes
            *data_lh_buf++ = static_cast<uint8_t>(value);
            *data_h_buf++ = static_cast<uint8_t>(value >> 8);
            key_byte |= 1 << shift;
        } else {
            *data_ll_buf++ = static_cast<uint8_t>(value);
        }
        shift += 1;
    }

    *keys_buf++ = key_byte;  // write last key (no increment needed)

    return {(size_t) (keys_buf - keys_buf_ini), (size_t) (data_ll_buf - data_ll_ini), (size_t) (data_lh_buf - data_lh_ini), (size_t) (data_h_buf - data_h_ini)};
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_ll_lh encode_ll_lh(Int16T const * in,
              uint8_t * SVB_RESTRICT keys_buf,
              uint8_t * SVB_RESTRICT data_ll_buf,
              uint8_t * SVB_RESTRICT data_lh_buf,
              uint8_t * SVB_RESTRICT data_h_buf,
              uint32_t count,
              Int16T prev = 0)
{

#ifdef SVB16_X64
    // if (has_ssse3()) {
    //     return encode_sse<Int16T, UseDelta, UseZigzag>(in, keys, data, count, prev) - out;
    // }
#endif
    return encode_scalar_ll_lh<Int16T, UseDelta, UseZigzag>(in, keys_buf, data_ll_buf, data_lh_buf, data_h_buf, count, prev);
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
uint8_t const * decode_scalar_ll_lh(
    gsl::span<Int16T> out_span,
    gsl::span<uint8_t const> keys_span,
    gsl::span<uint8_t const> data_ll_span,
    gsl::span<uint8_t const> data_lh_span,    
    gsl::span<uint8_t const> data_h_span,
    Int16T prev = 0)
{
    auto const count = out_span.size();
    if (count == 0) {
        return data_ll_span.begin();
    }

    auto out = out_span.begin();
    auto keys = keys_span.begin();
    auto data_ll = data_ll_span.begin();
    auto data_lh = data_lh_span.begin();
    auto data_h = data_h_span.begin();

    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = *keys++;
    // need to do the arithmetic in unsigned space so it wraps
    auto u_prev = static_cast<uint16_t>(prev);
    
    for (uint32_t c = 0; c < count; c++, shift++) {
        if (shift == 8) {
            shift = 0;
            key_byte = *keys++;
        }
        
        uint16_t value = 0;
        if (((key_byte >> shift) & 0x01) == 1) {  // 2 bytes
            value = (uint16_t)*data_lh++;
            value |= ((uint16_t)*data_h++ << 8);
        } 
        else {
            value = (uint16_t)*data_ll++;
        }
        // uint16_t value = detail::decode_data(data_ll, data_lh, data_h, (key_byte >> shift) & 0x01);
        
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
    assert(data_ll <= data_ll_span.end());
    assert(data_lh <= data_lh_span.end());    
    assert(data_h <= data_h_span.end());
    
    return data_h;
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
size_t decode_ll_lh(gsl::span<Int16T> out, gsl::span<uint8_t const> in, uint32_t dll, uint32_t dlh, Int16T prev = 0)
{
    auto keys_length = ::svb16_key_length(out.size());
    auto const keys = in.subspan(0, keys_length);
    auto const data_ll = in.subspan(keys_length);
    auto const data_lh = in.subspan(keys_length + dll);
    auto const data_h = in.subspan(keys_length + dll + dlh);

#ifdef SVB16_X64
    //if (has_sse4_1()) {
    //    return decode_sse<Int16T, UseDelta, UseZigzag>(out, keys, data, prev) - in.begin();
    //}
#endif
    return decode_scalar_ll_lh<Int16T, UseDelta, UseZigzag>(out, keys, data_ll, data_lh, data_h, prev) - in.begin();
}

} // namespace svb16

namespace pgnano {
    
    arrow::Result<std::size_t> compress_signal_ll_lh(   // COMPRESOR LL_LH //
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    gsl::span<std::uint8_t> const & destination,
    pod5::ReadData const & read_data,
    bool is_last_batch)
{

    auto t1 = std::chrono::steady_clock::now();

    ARROW_ASSIGN_OR_RAISE(auto keys_buf, arrow::AllocateResizableBuffer(svb16_key_length(samples.size()), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_ll_buf, arrow::AllocateResizableBuffer(samples.size(), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_lh_buf, arrow::AllocateResizableBuffer(samples.size(), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_h_buf, arrow::AllocateResizableBuffer(samples.size(), pool));

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;
    
    auto const res_sizes = svb16::encode_ll_lh<int16_t, UseDelta, UseZigzag>(
        samples.data(), 
        keys_buf->mutable_data(),
        data_ll_buf->mutable_data(),
        data_lh_buf->mutable_data(),
        data_h_buf->mutable_data(),
        samples.size());
    
    ARROW_RETURN_NOT_OK(keys_buf->Resize(res_sizes.keys_sz));
    ARROW_RETURN_NOT_OK(data_ll_buf->Resize(res_sizes.data_ll_sz));
    ARROW_RETURN_NOT_OK(data_lh_buf->Resize(res_sizes.data_lh_sz));
    ARROW_RETURN_NOT_OK(data_h_buf->Resize(res_sizes.data_h_sz));

    full_size_keys += res_sizes.keys_sz;
    full_size_M += res_sizes.data_ll_sz;
    full_size_Llow += res_sizes.data_lh_sz;
    full_size_Lhigh += res_sizes.data_h_sz;

    total_samples += samples.size();

    size_t const zstd_keys_max_size = ZSTD_compressBound(keys_buf->size());
    if (ZSTD_isError(zstd_keys_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for keys");
    }

    auto compressed_keys_result = arrow::AllocateResizableBuffer(zstd_keys_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_keys, compressed_keys_result);

    size_t const compressed_keys_size = ZSTD_compress(
        compressed_keys->mutable_data(), compressed_keys->size(),
        keys_buf->mutable_data(), keys_buf->size(), 1);
    if (ZSTD_isError(compressed_keys_size)) {
        return pod5::Status::Invalid("Failed to compress keys");
    }

    size_t const zstd_data_ll_max_size = ZSTD_compressBound(data_ll_buf->size());
    if (ZSTD_isError(zstd_data_ll_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    auto compressed_data_ll_result = arrow::AllocateResizableBuffer(zstd_data_ll_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_ll, compressed_data_ll_result);
 
    size_t const compressed_data_ll_size = ZSTD_compress(
        compressed_data_ll->mutable_data(), compressed_data_ll->size(),
        data_ll_buf->data(), data_ll_buf->size(), 1);
    if (ZSTD_isError(compressed_data_ll_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

    size_t const zstd_data_lh_max_size = ZSTD_compressBound(data_lh_buf->size());
    if (ZSTD_isError(zstd_data_lh_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    auto compressed_data_lh_result = arrow::AllocateResizableBuffer(zstd_data_lh_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_lh, compressed_data_lh_result);
 
    size_t const compressed_data_lh_size = ZSTD_compress(
        compressed_data_lh->mutable_data(), compressed_data_lh->size(),
        data_lh_buf->data(), data_lh_buf->size(), 1);
    if (ZSTD_isError(compressed_data_lh_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

    size_t const zstd_data_h_max_size = ZSTD_compressBound(data_h_buf->size());
    if (ZSTD_isError(zstd_data_h_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    auto compressed_data_h_result = arrow::AllocateResizableBuffer(zstd_data_h_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_h, compressed_data_h_result);
 
    size_t const compressed_data_h_size = ZSTD_compress(
        compressed_data_h->mutable_data(), compressed_data_h->size(),
        data_h_buf->data(), data_h_buf->size(), 1);
    if (ZSTD_isError(compressed_data_h_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

    // Initialize write_ptr at the beginning of the destination buffer
    auto write_ptr = destination.data();

    // Copy the size of compressed keys at the beginning of the destination buffer
    std::memcpy(write_ptr, &compressed_keys_size, sizeof(compressed_keys_size));
    write_ptr += sizeof(compressed_keys_size);

    // Copy compressed keys after the size in the destination buffer
    std::memcpy(write_ptr, compressed_keys->data(), compressed_keys_size);
    write_ptr += compressed_keys_size;

    // Copy the size of compressed data_low_low
    std::memcpy(write_ptr, &compressed_data_ll_size, sizeof(compressed_data_ll_size));
    write_ptr += sizeof(compressed_data_ll_size);

    // Copy compressed data_low_low 
    std::memcpy(write_ptr, compressed_data_ll->data(), compressed_data_ll_size);
    write_ptr += compressed_data_ll_size;

    // Copy the size of compressed data_low_high
    std::memcpy(write_ptr, &compressed_data_lh_size, sizeof(compressed_data_lh_size));
    write_ptr += sizeof(compressed_data_lh_size);

    // Copy compressed data_low_high
    std::memcpy(write_ptr, compressed_data_lh->data(), compressed_data_lh_size);
    write_ptr += compressed_data_lh_size;

    // Copy compressed data_high 
    std::memcpy(write_ptr, compressed_data_h->data(), compressed_data_h_size);

    auto t2 = std::chrono::steady_clock::now();

    compression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();

    write_ptr += compressed_data_h_size;

    unsigned long compr_size = write_ptr - destination.data();

    comp_size_keys += sizeof(compressed_keys_size) + compressed_keys_size;
    comp_size_M += sizeof(compressed_data_ll_size) + compressed_data_ll_size;
    comp_size_Llow += sizeof(compressed_data_lh_size) + compressed_data_lh_size;
    comp_size_Lhigh += compressed_data_h_size;

    return compr_size;
}

pod5::Status decompress_signal_ll_lh(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination,
    pgnano::PGNanoReaderState & state)
{
    
    auto t1d = std::chrono::steady_clock::now();
    
    std::size_t compressed_keys_size;
    std::memcpy(&compressed_keys_size, compressed_bytes.data(), sizeof(compressed_keys_size));

    unsigned long long const decompressed_zstd_keys_size =
        ZSTD_getFrameContentSize(compressed_bytes.data() + sizeof(compressed_keys_size), compressed_keys_size);
    if (ZSTD_isError(decompressed_zstd_keys_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_keys_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_keys_size),
            ")");
    }
    
    std::size_t compressed_data_ll_size;
    std::memcpy(&compressed_data_ll_size, compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size, 
        sizeof(compressed_data_ll_size));

    unsigned long long const decompressed_zstd_data_ll_size =
        ZSTD_getFrameContentSize(compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size + sizeof(compressed_data_ll_size), 
            compressed_data_ll_size);
    if (ZSTD_isError(decompressed_zstd_data_ll_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_ll_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_ll_size),
            ")");
    }
    
    std::size_t compressed_data_lh_size;
    std::memcpy(&compressed_data_lh_size, compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size + sizeof(compressed_data_ll_size) 
        + compressed_data_ll_size, sizeof(compressed_data_lh_size));

    unsigned long long const decompressed_zstd_data_lh_size =
        ZSTD_getFrameContentSize(compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size 
            + sizeof(compressed_data_ll_size) + compressed_data_ll_size + sizeof(compressed_data_lh_size), compressed_data_lh_size);
    if (ZSTD_isError(decompressed_zstd_data_lh_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_lh_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_lh_size),
            ")");
    }

    size_t compressed_data_h_size = 
        compressed_bytes.size() - sizeof(compressed_keys_size) - compressed_keys_size - sizeof(compressed_data_ll_size) 
            - compressed_data_ll_size - sizeof(compressed_data_lh_size) - compressed_data_lh_size;
    
    unsigned long long const decompressed_zstd_data_h_size =
        ZSTD_getFrameContentSize(compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size 
                + sizeof(compressed_data_ll_size) + compressed_data_ll_size + sizeof(compressed_data_lh_size) + compressed_data_lh_size, compressed_data_h_size);
    if (ZSTD_isError(decompressed_zstd_data_h_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_h_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_h_size),
            ")");
    }

    auto allocation_padding = svb16::decode_input_buffer_padding_byte_count();
    ARROW_ASSIGN_OR_RAISE(
        auto intermediate,
        arrow::AllocateResizableBuffer(decompressed_zstd_keys_size + decompressed_zstd_data_ll_size + decompressed_zstd_data_lh_size
            + decompressed_zstd_data_h_size + allocation_padding, pool));
    
    size_t const decompress_res_keys = ZSTD_decompress(
        intermediate->mutable_data(),
        decompressed_zstd_keys_size,
        compressed_bytes.data() + sizeof(compressed_keys_size),
        compressed_keys_size);
    if (ZSTD_isError(decompress_res_keys)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_keys,
            " ",
            ZSTD_getErrorName(decompress_res_keys),
            ")");
    }    
    
    size_t const decompress_res_data_ll = ZSTD_decompress(
        intermediate->mutable_data() + decompressed_zstd_keys_size,
        decompressed_zstd_data_ll_size,
        compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size + sizeof(compressed_data_ll_size),
        compressed_data_ll_size);
    if (ZSTD_isError(decompress_res_data_ll)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_ll,
            " ",
            ZSTD_getErrorName(decompress_res_data_ll),
            ")");
    }  

    size_t const decompress_res_data_lh = ZSTD_decompress(
        intermediate->mutable_data() + decompressed_zstd_keys_size + decompressed_zstd_data_ll_size,
        decompressed_zstd_data_lh_size,
        compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size + sizeof(compressed_data_ll_size) 
            + compressed_data_ll_size + sizeof(compressed_data_lh_size), compressed_data_lh_size);
    if (ZSTD_isError(decompress_res_data_lh)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_lh,
            " ",
            ZSTD_getErrorName(decompress_res_data_lh),
            ")");
    }  
    
    size_t const decompress_res_data_h = ZSTD_decompress(
        intermediate->mutable_data() + decompressed_zstd_keys_size + decompressed_zstd_data_ll_size + decompressed_zstd_data_lh_size,
        decompressed_zstd_data_h_size,
        compressed_bytes.data() + sizeof(compressed_keys_size) + compressed_keys_size + sizeof(compressed_data_ll_size) + compressed_data_ll_size 
            + sizeof(compressed_data_lh_size) + compressed_data_lh_size, compressed_data_h_size);
    if (ZSTD_isError(decompress_res_data_h)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_h,
            " ",
            ZSTD_getErrorName(decompress_res_data_h),
            ")");
    }  

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto consumed_count = svb16::decode_ll_lh<int16_t, UseDelta, UseZigzag>(destination, gsl::make_span(intermediate->data(), 
        intermediate->size()), decompress_res_data_ll, decompress_res_data_lh, 0);
    if ((consumed_count + allocation_padding) != (std::size_t)intermediate->size()) {
        return pod5::Status::Invalid("Remaining data at end of signal buffer");
    }

    auto t2d = std::chrono::steady_clock::now();

    decompression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2d - t1d).count();

    return pod5::Status::OK();
}

}   // namespace pgnano
