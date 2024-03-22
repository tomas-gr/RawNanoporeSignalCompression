#include "model.h"

namespace pgnano 
{
    void Model::reset() { m_model.reset(); };
    void Model::reset(const std::array<uint16_t, MAX_SYMBOLS> & precalc_histogram) { m_model.reset(); };
    size_t Model::encode_symbol(RangeCoder *rc, uint16_t sym) { return m_model.encodeSymbolRegular(rc, sym); }
    uint16_t Model::decode_symbol(RangeCoder *rc) { return m_model.decodeSymbolRegular(rc); }
    void Model::encode_binary(RangeCoder *rc, uint8_t sym) { return m_model.encodeSymbolBinary(rc, sym); }
    uint8_t Model::decode_binary(RangeCoder *rc) { return m_model.decodeSymbolBinary(rc); }
};
