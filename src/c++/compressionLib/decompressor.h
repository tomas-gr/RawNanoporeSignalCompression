#pragma once

#include <string>
#include <stdint.h>
#include <boost/uuid/uuid.hpp>

#include "header.h"
#include "model.h"
#include "clr.h"

namespace pgnano { namespace standalone 
{

class Decompressor
{
public:
    Decompressor(const std::string & bam_filename, const std::string & levels_filename);
    ~Decompressor();
    Decompressor(Decompressor &&) = delete;
    Decompressor(const Decompressor &) = delete;
    Decompressor & operator=(const Decompressor &) = delete;

    uint32_t retrieve_sample_count(const int16_t * in) noexcept;
    // Buffer assignment is user's responsibility
    void decompress(const boost::uuids::uuid & read_id, int16_t * in, int16_t * out);
private:
    void decompress(const boost::uuids::uuid & read_id, uint8_t * in, int16_t * out);
    void decompress_header(const uint8_t * in, Header & h);
    void decompress_signal(const boost::uuids::uuid  & read_id, uint8_t * in, int16_t * out, const Header & h);
    uint32_t retrieve_sample_count(const uint8_t * in) noexcept;
    void reset_models();

    RangeCoder m_rc;
    Model m_first_byte_model_zero, m_first_byte_model_non_zero, m_second_byte_model;
};
   
}};