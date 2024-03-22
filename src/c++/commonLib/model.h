#pragma once

#include <stdint.h>

#include "simple_model.h"

constexpr unsigned MAX_SYMBOLS = 1 << 8;

namespace pgnano 
{

class Model
{
public:
    Model() {};
    Model(const std::array<uint16_t, MAX_SYMBOLS> & precalc_histogram): m_model(precalc_histogram) {};
    void reset();
    void reset(const std::array<uint16_t, MAX_SYMBOLS> & precalc_histogram);
    size_t encode_symbol(RangeCoder *rc, uint16_t sym);
    uint16_t decode_symbol(RangeCoder *rc);
    void encode_binary(RangeCoder *rc, uint8_t sym);
    uint8_t decode_binary(RangeCoder *rc);
private:
    SIMPLE_MODEL<MAX_SYMBOLS> m_model;
};

}