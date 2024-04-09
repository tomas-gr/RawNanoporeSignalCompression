#ifndef NANO_01_HPP
#define NANO_01_HPP

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

#include <arrow/buffer.h>
#include <zstd.h>

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

extern long number_small;
extern long number_medium;
extern long number_large;

extern long total_samples;

extern double compression_time;
extern double decompression_time;


namespace svb16 {

struct buf_sizes_N01 {
    size_t keys_sz, data_S_sz, data_M_sz, data_Llow_sz, data_Lhigh_sz;
};

template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_N01 encode_scalar_N01(
    Int16T const * in,
    uint8_t * SVB_RESTRICT keys,
    uint8_t * data_S,
    uint8_t * SVB_RESTRICT data_M,
    uint8_t * SVB_RESTRICT data_L_low,
    uint8_t * SVB_RESTRICT data_L_high,
    uint32_t count,
    Int16T prev = 0)
{
    
    if (count == 0) {
        return {0, 0, 0 ,0, 0};
    }
    uint8_t * keys_begin = keys;
    uint8_t * data_S_begin = data_S;
    uint8_t * data_M_begin = data_M;
    uint8_t * data_Llow_begin = data_L_low;
    uint8_t * data_Lhigh_begin = data_L_high;

    uint8_t S_shift = 0;
    uint8_t shift = 0;  // cycles 0,2,4,6,8 then resets
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

        uint8_t code = 0;
        // 0 byte
        if (value == 0) {
            code = 0;
        } 
        // 1/2 byte
        else if (--value < 16) {
            number_small++;
            if (S_shift == 8)
            {
                S_shift = 0;
                data_S++;
            }

            auto val_masked = value & 0xf;

            if (S_shift == 0)
            {
                *data_S = 0;
            }
            *data_S |= val_masked << S_shift;
            S_shift += 4;
            code = 1;
        }
        // 1 byte
        else if ((value -= 16) < 256){
            number_medium++;
            *data_M++ = static_cast<uint8_t>(value);
            code = 2;
        }
        // 2 bytes
        else {
            number_large++;
            *data_L_low++ = static_cast<uint8_t>(value -= 256);
            *data_L_high++ = static_cast<uint8_t>(value >> 8);
            code = 3;
        }

        key_byte |= code << shift;
        shift += 2;
    }

    if (S_shift != 0){
        data_S += 1;
    }
    *keys++ = key_byte;  // write last key (no increment needed)

    return {(size_t) (keys - keys_begin), (size_t) (data_S - data_S_begin), (size_t) (data_M - data_M_begin), (size_t) (data_L_low - data_Llow_begin),(size_t) (data_L_high - data_Lhigh_begin)};
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_N01 encode_N01(Int16T const * in,
              uint8_t * SVB_RESTRICT keys,
              uint8_t * SVB_RESTRICT data_S,
              uint8_t * SVB_RESTRICT data_M,
              uint8_t * SVB_RESTRICT data_L_low,
              uint8_t * SVB_RESTRICT data_L_high,
              uint32_t count,
              Int16T prev = 0)
{

// #ifdef SVB16_X64
//     // if (has_ssse3()) {
//     //     return encode_sse<Int16T, UseDelta, UseZigzag>(in, keys, data, count, prev) - out;
//     // }
// #endif
    return encode_scalar_N01<Int16T, UseDelta, UseZigzag>(in, keys, data_S, data_M, data_L_low, data_L_high, count, prev);
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
uint8_t const * decode_scalar_N01(
    gsl::span<Int16T> out_span,
    gsl::span<uint8_t const> keys_span,
    gsl::span<uint8_t const> data_S_span,
    gsl::span<uint8_t const> data_M_span,    
    gsl::span<uint8_t const> data_L_low_span,    
    gsl::span<uint8_t const> data_L_high_span,
    Int16T prev = 0)
{
    auto const count = out_span.size();
    if (count == 0) {
        return data_L_high_span.begin();
    }

    auto out = out_span.begin();
    auto keys = keys_span.begin();
    auto data_S = data_S_span.begin();
    auto data_M = data_M_span.begin();
    auto data_L_low = data_L_low_span.begin();
    auto data_L_high = data_L_high_span.begin();

    uint8_t S_shift = 0;
    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = *keys++;
    // need to do the arithmetic in unsigned space so it wraps
    auto u_prev = static_cast<uint16_t>(prev);
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            key_byte = *keys++;
        }
        
        uint16_t value;
        uint8_t code = (key_byte >> shift) & 0x3;

        if (code == 0) { // 0 bytes
            value = 0;
        } else if (code == 1) { // 1/2 byte
            if (S_shift == 8)
            {
                S_shift = 0;
                data_S++;
            }
            if (S_shift == 0)
            {
                value = *data_S & 0xf;
            } else {
                value = *data_S >> 4;
            }
            S_shift += 4;
            value += 1;
        } else if (code == 2) { // 1 byte
            value = static_cast<uint8_t>(*data_M++) + 17;
        } else if (code == 3) { // 2 bytes
            value = (*data_L_high++ << 8) + *data_L_low++ + 273;
        } else {
            assert(0 && "Unknown code");
            value = 0;
        }

        SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_decode(value);}
        SVB16_IF_CONSTEXPR(UseDelta)
        {
            value += u_prev;
            u_prev = value;
        }
        *out++ = static_cast<Int16T>(value);
        shift += 2;
    }

    if (S_shift != 0)
    {
        data_S += 1;
    }

    assert(out == out_span.end());
    assert(keys == keys_span.end());
    assert(data_S <= data_S_span.end());
    assert(data_M <= data_M_span.end());    
    assert(data_L_low <= data_L_low_span.end());
    assert(data_L_high <= data_L_high_span.end());
    
    return data_L_high;
}


template <typename Int16T, bool UseDelta, bool UseZigzag>
size_t decode_N01(gsl::span<Int16T> out, gsl::span<uint8_t const> in, uint32_t dS, uint32_t dM, uint32_t dLl, Int16T prev = 0)
{   
    auto keys_length = ::svb16_key_length_2bit(out.size());
    auto const keys = in.subspan(0, keys_length);
    auto const data_S = in.subspan(keys_length);
    auto const data_M = in.subspan(keys_length + dS);
    auto const data_L_low = in.subspan(keys_length + dS + dM);
    auto const data_L_high = in.subspan(keys_length + dS + dM + dLl);
#ifdef SVB16_X64
    // if (has_ssse3()) {
    //     return encode_sse<Int16T, UseDelta, UseZigzag>(in, keys, data, count, prev) - out;
    // }
#endif
    return decode_scalar_N01<Int16T, UseDelta, UseZigzag>(out, keys, data_S, data_M, data_L_low, data_L_high, prev) - in.begin();
}

} // namespace svb16


namespace pgnano{

arrow::Result<std::size_t> compress_signal_N01(   // COMPRESOR NANO_01 //
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    gsl::span<std::uint8_t> const & destination,
    pod5::ReadData const & read_data,
    bool is_last_batch)
{
    auto t1 = std::chrono::steady_clock::now();

    // --------------- First compress the samples using svb --------------- //
    
    ARROW_ASSIGN_OR_RAISE(auto keys, arrow::AllocateResizableBuffer(svb16_key_length_2bit(samples.size()), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_S, arrow::AllocateResizableBuffer(samples.size(), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_M, arrow::AllocateResizableBuffer(samples.size(), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_L_low, arrow::AllocateResizableBuffer(samples.size(), pool));
    ARROW_ASSIGN_OR_RAISE(auto data_L_high, arrow::AllocateResizableBuffer(samples.size(), pool));

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;
    
    auto const res_sizes = svb16::encode_N01<int16_t, UseDelta, UseZigzag>(
        samples.data(), 
        keys->mutable_data(),
        data_S->mutable_data(),
        data_M->mutable_data(),
        data_L_low->mutable_data(),
        data_L_high->mutable_data(),
        samples.size());
    
    ARROW_RETURN_NOT_OK(keys->Resize(res_sizes.keys_sz));
    ARROW_RETURN_NOT_OK(data_S->Resize(res_sizes.data_S_sz));
    ARROW_RETURN_NOT_OK(data_M->Resize(res_sizes.data_M_sz));
    ARROW_RETURN_NOT_OK(data_L_low->Resize(res_sizes.data_Llow_sz));
    ARROW_RETURN_NOT_OK(data_L_high->Resize(res_sizes.data_Lhigh_sz));


    full_size_keys += res_sizes.keys_sz;
    full_size_S += res_sizes.data_S_sz;
    full_size_M += res_sizes.data_M_sz;
    full_size_Llow += res_sizes.data_Llow_sz;
    full_size_Lhigh += res_sizes.data_Lhigh_sz;

    total_samples += samples.size();


    // --------------- Now compress the keys using zstd --------------- //
    size_t const zstd_keys_max_size = ZSTD_compressBound(keys->size());
    if (ZSTD_isError(zstd_keys_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for keys");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_keys_result = arrow::AllocateResizableBuffer(zstd_keys_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_keys, compressed_keys_result);

    size_t const compressed_keys_size = ZSTD_compress(
        compressed_keys->mutable_data(), compressed_keys->size(),
        keys->mutable_data(), keys->size(), 1);
    if (ZSTD_isError(compressed_keys_size)) {
        return pod5::Status::Invalid("Failed to compress keys");
    }

    // --------------- Now compress the data_S using zstd --------------- //
    // find max size
    size_t const zstd_data_S_max_size = ZSTD_compressBound(data_S->size());
    if (ZSTD_isError(zstd_data_S_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data_S");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_S_result = arrow::AllocateResizableBuffer(zstd_data_S_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_S, compressed_data_S_result);
 
    // Do the compression
    size_t const compressed_data_S_size = ZSTD_compress(
        compressed_data_S->mutable_data(), compressed_data_S->size(),
        data_S->data(), data_S->size(), 1);
    if (ZSTD_isError(compressed_data_S_size)) {
        return pod5::Status::Invalid("Failed to compress data_S");
    }

    // --------------- Now compress the data_M using zstd --------------- //
    size_t const zstd_data_M_max_size = ZSTD_compressBound(data_M->size());
    if (ZSTD_isError(zstd_data_M_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data_M");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_M_result = arrow::AllocateResizableBuffer(zstd_data_M_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_M, compressed_data_M_result);
 
    size_t const compressed_data_M_size = ZSTD_compress(
        compressed_data_M->mutable_data(), compressed_data_M->size(),
        data_M->data(), data_M->size(), 1);
    if (ZSTD_isError(compressed_data_M_size)) {
        return pod5::Status::Invalid("Failed to compress data_M");
    }

    // --------------- Now compress the data_L_low using zstd --------------- //
    size_t const zstd_data_Llow_max_size = ZSTD_compressBound(data_L_low->size());
    if (ZSTD_isError(zstd_data_Llow_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data_L_low");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_Llow_result = arrow::AllocateResizableBuffer(zstd_data_Llow_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_Llow, compressed_data_Llow_result);
 
    size_t const compressed_data_Llow_size = ZSTD_compress(
        compressed_data_Llow->mutable_data(), compressed_data_Llow->size(),
        data_L_low->data(), data_L_low->size(), 1);
    if (ZSTD_isError(compressed_data_Llow_size)) {
        return pod5::Status::Invalid("Failed to compress data_L_low");
    }

    // --------------- Now compress the data_L_high using zstd --------------- //
    size_t const zstd_data_Lhigh_max_size = ZSTD_compressBound(data_L_high->size());
    if (ZSTD_isError(zstd_data_Lhigh_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data_L_high");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_Lhigh_result = arrow::AllocateResizableBuffer(zstd_data_Lhigh_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_Lhigh, compressed_data_Lhigh_result);
 
    size_t const compressed_data_Lhigh_size = ZSTD_compress(
        compressed_data_Lhigh->mutable_data(), compressed_data_Lhigh->size(),
        data_L_high->data(), data_L_high->size(), 1);
    if (ZSTD_isError(compressed_data_Lhigh_size)) {
        return pod5::Status::Invalid("Failed to compress data_L_high");
    }

    auto write_ptr = destination.data();

    // Copy the size of compressed keys at the beginning of the destination buffer
    unsigned long total_comp_size = sizeof(compressed_keys_size) + compressed_keys_size + sizeof(compressed_data_S_size) + compressed_data_S_size 
        + sizeof(compressed_data_M_size) + compressed_data_M_size + sizeof(compressed_data_Llow_size) + compressed_data_Llow_size + compressed_data_Lhigh_size;
    // Assert there is enough room for memcpy
    if (destination.size() < total_comp_size)
    {
        // Print destination size and required size
        printf("Destination size: %lu\n", destination.size());
        printf("Required size: %lu\n", total_comp_size);
        
        return pod5::Status::Invalid("Not enough space in destination buffer");
    }

    std::memcpy(write_ptr, &compressed_keys_size, sizeof(compressed_keys_size));
    write_ptr += sizeof(compressed_keys_size);
    
    // Copy compressed keys after the size in the destination buffer
    std::memcpy(write_ptr, compressed_keys->data(), compressed_keys_size);
    write_ptr += compressed_keys_size;

    // Copy the size of compressed data_S
    std::memcpy(write_ptr, &compressed_data_S_size, sizeof(compressed_data_S_size));
    write_ptr += sizeof(compressed_data_S_size);

    // Copy compressed data_S 
    std::memcpy(write_ptr, compressed_data_S->data(), compressed_data_S_size);
    write_ptr += compressed_data_S_size;

    // Copy the size of compressed data_M
    std::memcpy(write_ptr, &compressed_data_M_size, sizeof(compressed_data_M_size));
    write_ptr += sizeof(compressed_data_M_size);

    // Copy compressed data_M
    std::memcpy(write_ptr, compressed_data_M->data(), compressed_data_M_size);
    write_ptr += compressed_data_M_size;

    // Copy the size of compressed data_L_low
    std::memcpy(write_ptr, &compressed_data_Llow_size, sizeof(compressed_data_Llow_size));
    write_ptr += sizeof(compressed_data_Llow_size);
    
    // Copy compressed data_L_low
    std::memcpy(write_ptr, compressed_data_Llow->data(), compressed_data_Llow_size);
    write_ptr += compressed_data_Llow_size;

    // Copy compressed data_L_high
    std::memcpy(write_ptr, compressed_data_Lhigh->data(), compressed_data_Lhigh_size);
    write_ptr += compressed_data_Lhigh_size;

    auto t2 = std::chrono::steady_clock::now();
    compression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    
    comp_size_keys += sizeof(compressed_keys_size) + compressed_keys_size;
    comp_size_S += sizeof(compressed_data_S_size) + compressed_data_S_size;
    comp_size_M += sizeof(compressed_data_M_size) + compressed_data_M_size;
    comp_size_Llow += sizeof(compressed_data_Llow_size) + compressed_data_Llow_size;
    comp_size_Lhigh += compressed_data_Lhigh_size;

    return total_comp_size;
}

// DECOMPRESOR N01 //   ---->   FALTA AGREGAR LOS OTROS DECOMPRESORES 
pod5::Status decompress_signal_N01(
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

    // Se calculan los tamaños de keys descomprimidos:
    unsigned long long const decompressed_zstd_keys_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_keys_size);
    if (ZSTD_isError(decompressed_zstd_keys_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_keys_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_keys_size),
            ")");
    }
    read_ptr += compressed_keys_size;

    // Extract the size of compressed data_S    
    std::size_t compressed_data_S_size;
    std::memcpy(&compressed_data_S_size, read_ptr, sizeof(compressed_data_S_size));
    read_ptr += sizeof(compressed_data_S_size);

    // Se calculan los tamaños de data_S descomprimidos:
    unsigned long long const decompressed_zstd_data_S_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_data_S_size);
    if (ZSTD_isError(decompressed_zstd_data_S_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_S_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_S_size),
            ")");
    }
    read_ptr += compressed_data_S_size;
    
    // Extract the size of compressed data_M    
    std::size_t compressed_data_M_size;
    std::memcpy(&compressed_data_M_size, read_ptr, sizeof(compressed_data_M_size));
    read_ptr += sizeof(compressed_data_M_size);

    // Se calculan los tamaños de data_M descomprimidos:
    unsigned long long const decompressed_zstd_data_M_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_data_M_size);
    if (ZSTD_isError(decompressed_zstd_data_M_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_M_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_M_size),
            ")");
    }
    read_ptr += compressed_data_M_size;

    // Extract the size of compressed data_low_high    
    std::size_t compressed_data_L_low_size;
    std::memcpy(&compressed_data_L_low_size, read_ptr, sizeof(compressed_data_L_low_size));
    read_ptr += sizeof(compressed_data_L_low_size);

    // Se calculan los tamaños de data_low_high descomprimidos:
    unsigned long long const decompressed_zstd_data_L_low_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_data_L_low_size);
    if (ZSTD_isError(decompressed_zstd_data_L_low_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_L_low_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_L_low_size),
            ")");
    }
    read_ptr += compressed_data_L_low_size;

    // Size of compressed data_high
    size_t compressed_data_L_high_size = compressed_bytes.end() - read_ptr; 
    
    // Se calculan los tamaños de data_high descomprimidos:
    unsigned long long const decompressed_zstd_data_L_high_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_data_L_high_size);
    if (ZSTD_isError(decompressed_zstd_data_L_high_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_data_L_high_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_L_high_size),
            ")");
    }

    // Calculo el allocation_padding y creo el buffer intermediate
    auto allocation_padding = svb16::decode_input_buffer_padding_byte_count();
    ARROW_ASSIGN_OR_RAISE(
        auto intermediate,
        arrow::AllocateResizableBuffer( decompressed_zstd_keys_size +
                                        decompressed_zstd_data_S_size +
                                        decompressed_zstd_data_M_size +
                                        decompressed_zstd_data_L_low_size +
                                        decompressed_zstd_data_L_high_size +
                                        allocation_padding, pool));
    
    read_ptr = compressed_bytes.data() + sizeof(compressed_keys_size);
    auto write_ptr = intermediate->mutable_data();
    // Descomprimo Keys
    size_t const decompress_res_keys = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_keys_size,
        read_ptr,
        compressed_keys_size);
    if (ZSTD_isError(decompress_res_keys)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_keys,
            " ",
            ZSTD_getErrorName(decompress_res_keys),
            ")");
    }    
    read_ptr += compressed_keys_size + sizeof(compressed_data_S_size);
    write_ptr += decompressed_zstd_keys_size;
    // Descomprimo Data_low_low
    size_t const decompress_res_data_S = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_data_S_size,
        read_ptr,
        compressed_data_S_size);
    if (ZSTD_isError(decompress_res_data_S)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_S,
            " ",
            ZSTD_getErrorName(decompress_res_data_S),
            ")");
    }
    read_ptr += compressed_data_S_size + sizeof(compressed_data_M_size);
    write_ptr += decompressed_zstd_data_S_size;

    // Descomprimo Data_low_high
    size_t const decompress_res_data_M = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_data_M_size,
        read_ptr,
        compressed_data_M_size);
    if (ZSTD_isError(decompress_res_data_M)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_M,
            " ",
            ZSTD_getErrorName(decompress_res_data_M),
            ")");
    }
    read_ptr += compressed_data_M_size + sizeof(compressed_data_L_low_size);
    write_ptr += decompressed_zstd_data_M_size;  
    
    // Descomprimo Data_high
    size_t const decompress_res_data_L_low = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_data_L_low_size,
        read_ptr,
        compressed_data_L_low_size);
    if (ZSTD_isError(decompress_res_data_L_low)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_L_low,
            " ",
            ZSTD_getErrorName(decompress_res_data_L_low),
            ")");
    } 
    read_ptr += compressed_data_L_low_size;
    write_ptr += decompressed_zstd_data_L_low_size;

    // Descomprimo Data_high
    size_t const decompress_res_data_L_high = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_data_L_high_size,
        read_ptr,
        compressed_data_L_high_size);
    if (ZSTD_isError(decompress_res_data_L_high)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res_data_L_high,
            " ",
            ZSTD_getErrorName(decompress_res_data_L_high),
            ")");
    }

    // Now decompress the data using svb:
    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto consumed_count = svb16::decode_N01<int16_t, UseDelta, UseZigzag>(destination, gsl::make_span(intermediate->data(), 
        intermediate->size()), decompress_res_data_S, decompress_res_data_M, decompress_res_data_L_low, 0);
    if ((consumed_count + allocation_padding) != (std::size_t)intermediate->size()) {
        return pod5::Status::Invalid("Remaining data at end of signal buffer");
    }

    auto t2d = std::chrono::steady_clock::now();
    decompression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2d - t1d).count();
    
    return pod5::Status::OK();
}

} // namespace pgnano
