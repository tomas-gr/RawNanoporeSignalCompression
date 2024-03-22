#pragma once

#include <string>
#include <stdint.h>
#include <boost/uuid/uuid.hpp>

#include "clr.h"
#include "model.h"
#include "header.h"
#include "level_table.h"
#include "BAM_handler.h"


namespace pgnano { namespace standalone 
{

struct CompressionResult
{
    uint64_t words_written;
    bool is_raw;
};

struct IntermediateCompressionResult
{
    uint64_t bytes_written;
    bool is_raw;
};

constexpr size_t overflow_buffer_bytes = 8 * 2 ; // How much extra bytes are needed to avoid overflow when detecting that a signal must be sent uncompressed. Need to account for 2 compressions (higher order byte and lower order byte)
class Compressor
{

public:
    Compressor(const std::string & bam_filepath, const std::string & levels_filepath);
    ~Compressor();
    Compressor(Compressor &&) = delete;
    Compressor(const Compressor &) = delete;
    Compressor & operator=(const Compressor &) = delete;

    // Return the maximum number of bytes needed to compress the requested amount of samples
    static inline constexpr std::size_t compressed_signal_max_size(uint32_t sample_count) noexcept { size_t x = sample_count*sizeof(int16_t) + header_words * sizeof(uint16_t) + overflow_buffer_bytes; return x & 0x1 ? ++x : x; } // FIXME: must always be an even number
    // Returns the number of 16 bits words written
    // Buffer assignment is user's responsibility
    size_t compress(const boost::uuids::uuid & read_id, int16_t * in_ptr, uint32_t sample_count, int16_t * out_ptr);
    inline constexpr uint64_t samples_processed() const noexcept { return m_samples_processed; }
    inline constexpr uint64_t signal_16_bit_words_written() const noexcept { return m_signal_16_bit_words_written; }
private:
    static inline std::size_t send_raw_threshold(uint32_t sample_count) { return sample_count*sizeof(int16_t); }
    
    // Initializes encoding
    void start_encode(uint8_t * out_ptr);
    // Returns bytes written to finish decoding
    size_t finish_encode();
    void reset_models();

    size_t compress(const boost::uuids::uuid & read_id, int16_t * in_ptr, uint32_t sample_count, uint8_t * out_ptr);
    void compress_header(const Header & h, uint8_t * out_ptr);
    CompressionResult compress_signal(const boost::uuids::uuid & read_id, int16_t * in_ptr, uint32_t sample_count, uint8_t * out_ptr);
    IntermediateCompressionResult compress_signal_chunk_arithmetic(int16_t *in_ptr, uint32_t samples_in_chunk, uint8_t * out_ptr, size_t abort_threshold);
    void gather_lms_stats(const boost::uuids::uuid & read_id, int16_t *in_ptr, uint32_t samples_in_chunk);

    uint_fast64_t m_samples_processed;
    uint_fast64_t m_signal_16_bit_words_written; // Does not include header
    Model m_first_byte_model_zero, m_first_byte_model_non_zero, m_second_byte_model;
    RangeCoder m_rc;
    //BAMHandler m_bam_handler;
    //LevelTable m_level_table;
};

}};