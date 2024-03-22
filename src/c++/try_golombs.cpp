#include "stdint.h"
#include "ic.h"
#include "simple_model.h"
#include "clr.h"
#include "zstd.h"

#include <iostream>

typedef struct {
    uint8_t N;
    uint16_t A;
} ContextClassStats;

typedef struct {
    uint16_t number_of_0;
    uint16_t binary_part;
    uint16_t k;
} golomb_data_t;

uint8_t get_k(ContextClassStats stats) {
    uint8_t k;
    for(k=0; (stats.N << k) < stats.A; k++);
    return k;
}

#define RESET_CONST 128

void update_context_class_stats(ContextClassStats * context_class, int16_t error){
    if(context_class->N == RESET_CONST) {
        context_class->N = context_class->N >> 1;
        context_class->A = context_class->A >> 1;
    }

    context_class->N += 1;
    context_class->A += error >= 0 ? error : (-1)*error;
}

size_t get_golomb_po2(uint8_t k, uint16_t mapped_error){
    golomb_data_t res;
    res.number_of_0 = mapped_error >> k;
    res.binary_part =  1 << k | (mapped_error & ((1<<k)-1));
    res.k = k;
    return res.number_of_0 + 1 + k;
}

template<typename T>
inline T to_bytes(T x)
{
    return (x / 8) + (x % 8 != 0 ? 1 : 0);
}

uint8_t bits_required(uint16_t x)
{
    uint8_t res = 16;
    for (; !(x & 0x8000); x <<= 1)
        res--;
    return res;
}

size_t compress_golomb(uint32_t samples, int16_t * data, uint8_t bit)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = ~low_mask;
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t* high_part = new uint16_t[samples];
    for (size_t i = 0; i < samples; i++)
    {
        high_part[i] = (data_uint[i] & high_mask) >> bit;
    }
    uint8_t* out_golomb_rice = new uint8_t[samples * 4];
    //size_t golomb_rice_bytes = bitrenc32(high_part, samples, out_golomb_rice);
    size_t golomb_rice_bits = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    for (size_t i = 0; i < samples; i++)
    {
        golomb_rice_bits += get_golomb_po2(get_k(_), high_part[i]);
        //FIXME: Less than 1 bit/symbol...
        update_context_class_stats(&_,high_part[i]);//FIXME: This is not the error
    }
    delete out_golomb_rice;
    delete high_part;
    return to_bytes(golomb_rice_bits) + to_bytes(bit * samples);
}

template<uint8_t bit>
size_t compress_golomb_range(uint32_t samples, int16_t * data)
{
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t* high_part = new uint16_t[samples];
    uint8_t* low_part = new uint8_t[samples];
    uint8_t* out = new uint8_t[samples * 4];
    SIMPLE_MODEL<(1 << bit)> m;
    uint16_t high_mask, low_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    RangeCoder rc;
    for (size_t i = 0; i < samples; i++)
    {
        high_part[i] = (data_uint[i] & high_mask) >> bit;
        low_part[i] = data_uint[i] & low_mask;
    }
    size_t golomb_rice_bits = 0;
    size_t rc_bytes = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    rc.output(out);
    rc.StartEncode();
    for (size_t i = 0; i < samples; i++)
    {
        golomb_rice_bits += get_golomb_po2(get_k(_), high_part[i]);
        update_context_class_stats(&_,high_part[i]);
        rc_bytes += m.encodeSymbolRegular(&rc,low_part[i]);
    }
    rc_bytes += rc.FinishEncode();
    delete high_part;
    delete low_part;
    return to_bytes(golomb_rice_bits) + rc_bytes;
}

size_t compress_golomb_zstd(uint32_t samples, int16_t * data)
{
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t* high_part = new uint16_t[samples];
    uint8_t* low_part = new uint8_t[samples];
    uint8_t* out = new uint8_t[ZSTD_compressBound(samples)];
    for (size_t i = 0; i < samples; i++)
    {
        high_part[i] = (data_uint[i] & 0xFFE0) >> 5;
        low_part[i] = data_uint[i] & 0x1F;
    }
    size_t golomb_rice_bits = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    for (size_t i = 0; i < samples; i++)
    {
        golomb_rice_bits += get_golomb_po2(get_k(_), high_part[i]);
        update_context_class_stats(&_,high_part[i]);
    }
    size_t zstd_bytes = ZSTD_compress(out, ZSTD_compressBound(samples), low_part, samples, 22);
    delete high_part;
    delete low_part;
    return to_bytes(golomb_rice_bits) + zstd_bytes;
}

#define MAX_RUN_LENGHT 15 //TODO: refactor to be able to inject this parameter and also to save bits if MAX_RUN_LENGHT < 256
size_t compress_golomb_RLE_uniform(uint32_t samples, int16_t * data, uint8_t bit)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t max_bits_packed = bits_required(max_value);
    uint8_t* high_part = new uint8_t[samples];
    for (size_t i = 0; i < samples; i++)
    {
        high_part[i] = (data_uint[i] & high_mask) >> bit;
    }
    uint8_t* out_golomb_rice = new uint8_t[samples * 2];
    size_t golomb_rice_bits = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    size_t i = 0;
    while (i < samples)
    {
        if (high_part[i] == 0)
        {
            uint8_t run_lenght = 0;
            while (high_part[i] == 0 && run_lenght < MAX_RUN_LENGHT)
            {
                run_lenght++;
                i++;
            }
            golomb_rice_bits += (max_bits_packed - bit) + 4;//FIXME: Incorrectly calculated
        }
        else 
        {
            golomb_rice_bits += get_golomb_po2(get_k(_), high_part[i]);//FIXME: GOLOMB QUE NO INCLUYA 0
            update_context_class_stats(&_,high_part[i]);
            i++;
        }
    }
    delete out_golomb_rice;
    delete high_part;
    return to_bytes(golomb_rice_bits) + to_bytes(bit * samples) + 1;   
}

size_t compress_golomb_low(uint32_t samples, int16_t * data, uint8_t bit)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t max_bits_packed = bits_required(max_value);
    size_t golomb_rice_bits = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    for (size_t i = 0; i < samples; i++)
    {
        golomb_rice_bits += get_golomb_po2(get_k(_), data_uint[i] & low_mask);
        update_context_class_stats(&_,data_uint[i] & low_mask);
    }
    return to_bytes(golomb_rice_bits) + to_bytes((max_bits_packed - bit) * samples) + 1;
}

template<uint8_t bit>
size_t compress_golomb_low_range(uint32_t samples, int16_t * data)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint8_t* out = new uint8_t[samples * 4];
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t max_bits_packed = bits_required(max_value);
    SIMPLE_MODEL<(uint16_t(1) << (16 - bit))> m;
    RangeCoder rc;
    rc.output(out);
    rc.StartEncode();
    size_t golomb_rice_bits = 0;
    size_t range_bytes = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    for (size_t i = 0; i < samples; i++)
    {
        golomb_rice_bits += get_golomb_po2(get_k(_), data_uint[i] & low_mask);
        update_context_class_stats(&_,data_uint[i] & low_mask);
        range_bytes += m.encodeSymbolRegular(&rc,(data_uint[i] & high_mask) >> bit);
    }
    range_bytes += rc.FinishEncode();
    delete out;
    return to_bytes(golomb_rice_bits) + range_bytes;
}


#define RLE_MAX_RUN 32
size_t RLE_9_4_1(uint32_t samples, int16_t * data, uint8_t bit)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t max_bits_packed = bits_required(max_value);
    size_t rle_bits = 0;
    uint32_t i = 0;
    while (i < samples)
    {
        uint16_t run_lenght = 0;
        while (data[i] & high_mask == 0 && run_lenght < RLE_MAX_RUN)
        {
            run_lenght++;
            i++;
        }
        if (run_lenght == 0)
        {
            rle_bits += (max_bits_packed - bit);
            i++;
        }
        else
        {
            rle_bits += 4 + (max_bits_packed - bit);
        }
    }
    return to_bytes(rle_bits) + to_bytes(samples * bit);
}

size_t compress_zstd_high(uint32_t samples, int16_t * data, uint8_t bit)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t* high_data = new uint16_t[samples];
    uint8_t* out = new uint8_t[samples * 4];
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t max_bits_packed = bits_required(max_value);
    size_t golomb_rice_bits = 0;
    size_t zstd_bytes = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    for (size_t i = 0; i < samples; i++)
    {
        golomb_rice_bits += get_golomb_po2(get_k(_), data_uint[i] & low_mask);
        update_context_class_stats(&_,data_uint[i] & low_mask);
        high_data[i] = (data_uint[i] & high_mask) >> bit;
    }

    zstd_bytes = ZSTD_compress(out, ZSTD_compressBound(samples), high_data, samples, 22);
    delete out;
    delete high_data;

    return to_bytes(golomb_rice_bits) + zstd_bytes;
}

#define MAX_RUN_LENGHT 15 //TODO: refactor to be able to inject this parameter and also to save bits if MAX_RUN_LENGHT < 256
template<uint8_t bit>
size_t compress_golomb_RLE_range(uint32_t samples, int16_t * data, uint8_t _bit_discard)
{
    uint16_t low_mask, high_mask;
    low_mask = (1 << bit) - 1;
    high_mask = (~low_mask);
    uint16_t* data_uint = reinterpret_cast<uint16_t*>(data);
    uint16_t max_value = bitz16(data_uint, samples, nullptr, 0);
    uint16_t max_bits_packed = bits_required(max_value);
    uint16_t* high_part = new uint16_t[samples * 4];
    uint8_t* out = new uint8_t[samples * 4];
    for (size_t i = 0; i < samples; i++)
    {
        high_part[i] = (data_uint[i] & high_mask) >> bit;
    }
    uint8_t* out_golomb_rice = new uint8_t[samples * 2];
    size_t golomb_rice_bits = 0;
    ContextClassStats _;
    _.A = 8;
    _.N = 1;
    size_t i = 0;
    size_t low_part_bytes = 0;
    RangeCoder rc_rle;
    SIMPLE_MODEL<(1 << 4)> m_rle;
    rc_rle.output(out);
    rc_rle.StartEncode();

    while (i < samples)
    {
        if (high_part[i] == 0)
        {
            uint8_t run_lenght = 0;
            while (high_part[i] == 0 && run_lenght < MAX_RUN_LENGHT)
            {
                run_lenght++;
                i++;
            }
            golomb_rice_bits += (max_bits_packed - bit) + m_rle.encodeSymbolRegular(&rc_rle, run_lenght);//FIXME: Incorrectly calculated
        }
        else 
        {
            golomb_rice_bits += get_golomb_po2(get_k(_), high_part[i]);//FIXME: GOLOMB QUE NO INCLUYA 0
            update_context_class_stats(&_,high_part[i]);
            i++;
        }
    }
    RangeCoder rc;
    SIMPLE_MODEL<(1 << bit)> m;
    rc.output(out);
    rc.StartEncode();
    for (uint32_t i = 0; i < samples; i++)
    {
        low_part_bytes += m.encodeSymbolRegular(&rc, data_uint[i] & low_mask);
    }
    low_part_bytes += rc.FinishEncode();

    delete out_golomb_rice;
    delete high_part;
    delete out;
    return to_bytes(golomb_rice_bits) + low_part_bytes + 1;   
}