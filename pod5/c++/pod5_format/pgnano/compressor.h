#pragma once

#include <stdint.h>
#include <atomic>
#include <mutex> // Instead of using a mutex just add a pgnano_init()

#include "pod5_format/types.h"
#include "pod5_format/read_table_utils.h"

#include "model.h"
#include "metadata.h"
#include "header.h"
#include "default_histogram.h"
#include "pgnano_writer_state.h"

namespace pgnano
{

constexpr size_t overflow_buffer_bytes = 8 * 2 ; // How much extra bytes are needed to avoid overflow when detecting that a signal must be sent uncompressed. Need to account for 2 compressions (higher order byte and lower order byte)

struct CompressionStats
{
public:
    std::uint_fast64_t bytes_written, total_sample_size;
};

struct CompressionResult
{
    size_t bytes_written;
    bool is_raw;
};

class Compressor
{
public:
    //Compressor() : m_high_byte_ctx_class(high_byte_histogram), m_low_byte_ctx_class(low_byte_histogram) {};
    void reset();
    size_t compress(pod5::ReadData const & read_data, std::size_t sample_count, int16_t const * samples, uint8_t* const & dest, pgnano::PGNanoWriterState & state);
    static inline std::size_t compressed_signal_max_size(std::size_t sample_count, pod5::ReadData read_data) { 
        long size = sample_count*sizeof(int16_t) + header_size + pgnano::overflow_buffer_bytes;
        if (size > 1024)
            return size;
        else
            return 1024; 
    }
    static inline void update_compression_stats(std::uint_fast64_t bytes, std::uint_fast64_t sample_size) 
    {
        static bool flag = false;
        static std::mutex mtx;
        if (!flag)
        {
            mtx.lock();
            if (!flag)
            {
                Compressor::bytes_written = 0;
                Compressor::total_sample_size = 0;
                flag = true;
            }
            mtx.unlock();
        }
        Compressor::bytes_written += bytes;
        Compressor::total_sample_size += sample_size;
    }
    static inline CompressionStats get_compression_stats() { return {Compressor::bytes_written, Compressor::total_sample_size}; }
private:
    static inline std::size_t send_raw_threshold(std::size_t sample_count, pod5::ReadData read_data) { return sample_count*sizeof(int16_t); }
    void compress_metadata(pgnano::Metadata const & metadata, uint8_t* const & dest);
    void compress_header(pgnano::Header const & header, uint8_t* const & dest);
    CompressionResult compress_signal(pod5::ReadData const & read_data, std::size_t sample_count, int16_t const * samples, uint8_t* const & dest);
    RangeCoder m_range_coder;
    Model m_high_byte_ctx_class;
    Model m_low_byte_ctx_class_zero, m_low_byte_ctx_class_non_zero;
    static std::atomic<std::uint_fast64_t> bytes_written;
    static std::atomic<std::uint_fast64_t> total_sample_size;
};

}