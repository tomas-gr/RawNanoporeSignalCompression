#pragma once

#include "common.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace svb16 {
  
struct buf_sizes_VBZ {
    size_t keys_sz, data_sz;
};

namespace detail {
inline uint16_t zigzag_encode(uint16_t val)
{
    return (val + val) ^ static_cast<uint16_t>(static_cast<int16_t>(val) >> 15);
}
}  // namespace detail


// ------------------------ FUNCIÓN ORIGINAL ------------------------ //    ------> Utilizada por el COMPRESOR KD
template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_VBZ encode_scalar(
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

    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = 0;
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keys++ = key_byte;
            key_byte = 0;
        }

        // Calcula VALUE:

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

        // Almacena BYTES en BUFFER

        if (value < (1 << 8)) {  // 1 byte
            *data = static_cast<uint8_t>(value);
            ++data;
        } else {                           // 2 bytes
            std::memcpy(data, &value, 2);  // assumes little endian
            data += 2;
            key_byte |= 1 << shift;
        }

        shift += 1;
    }

    *keys = key_byte;  // write last key (no increment needed)
    return {(size_t) (data - data_begin), (size_t) (keys - keys_begin)};
}


// ------------------------ FUNCIÓN MODIFICADA LOW_HIGH ------------------------ //
// template <typename Int16T, bool UseDelta, bool UseZigzag>
// uint8_t * encode_scalar_lh(
//     Int16T const * in,
//     uint8_t * SVB_RESTRICT keys,
//     uint8_t * SVB_RESTRICT data_low,
//     uint8_t * SVB_RESTRICT data_high,
//     uint32_t count,
//     Int16T prev = 0)
// {
    
//     if (count == 0) {
//         return data_low;
//     }

//     uint8_t shift = 0;  // cycles 0 through 7 then resets
//     uint8_t key_byte = 0;
   
//     for (uint32_t c = 0; c < count; c++) {
//         if (shift == 8) {
//             shift = 0;
//             *keys++ = key_byte;
//             key_byte = 0;
//         }

//         // Calcula VALUE:

//         uint16_t value;
//         SVB16_IF_CONSTEXPR(UseDelta)
//         {
//             // need to do the arithmetic in unsigned space so it wraps
//             value = static_cast<uint16_t>(in[c]) - static_cast<uint16_t>(prev);
//             SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_encode(value); }
//             prev = in[c];
//         }
//         else SVB16_IF_CONSTEXPR(UseZigzag) {
//             value = detail::zigzag_encode(static_cast<uint16_t>(in[c]));
//         }
//         else {
//             value = static_cast<uint16_t>(in[c]);
//         }

//         *data_low++ = static_cast<uint8_t>(value);

//         // Almacena BYTES en BUFFER
//         if (value >= (1 << 8)) {    // 2 bytes
//             *data_high++ = static_cast<uint8_t>(value >> 8);  // assumes little endian
//             key_byte |= 1 << shift;
//         }
//         shift += 1;
//     }

//     *keys = key_byte;  // write last key (no increment needed)
//     return data_high;
// }


// ------------------------ FUNCIÓN MODIFICADA NANO_01 ------------------------ //

// static uint8_t _encode_data_N01(uint32_t val,uint8_t** data, uint8_t* data_shift) {
//     uint8_t code;
//     // TODO hacer todo con la funcion write (emprolijar)
//     auto write_data = [&](uint32_t value, uint8_t bits) {

//         auto& current_shift = *data_shift;
//         for (std::size_t i = 0; i < bits / 4; ++i)
//         {
//             if (current_shift == 8)
//             {
//                 current_shift = 0;
//                 *data += 1;
//             }

//             auto val_masked = value & 0xf;
//             value >>= 4;

//             if (current_shift == 0)
//             {
//                 **data = 0;
//             }
//             **data |= val_masked << current_shift;
//             current_shift += 4;
//         }
//     };

//     write_data(val, 4);
//     code = 1;

//     return code;
// }




// template <typename Int16T, bool UseDelta, bool UseZigzag>
// buf_sizes_N01 encode_scalar_N01(
//     Int16T const * in,
//     uint8_t * SVB_RESTRICT keys,
//     uint8_t * data_S,
//     uint8_t * SVB_RESTRICT data_M,
//     uint8_t * SVB_RESTRICT data_L_low,
//     uint8_t * SVB_RESTRICT data_L_high,
//     uint32_t count,
//     Int16T prev = 0)
// {
    
//     if (count == 0) {
//         return {0, 0, 0 ,0, 0};
//     }
//     uint8_t * keys_begin = keys;
//     uint8_t * data_S_begin = data_S;
//     uint8_t * data_M_begin = data_M;
//     uint8_t * data_Llow_begin = data_L_low;
//     uint8_t * data_Lhigh_begin = data_L_high;

//     uint8_t S_shift = 0;
//     uint8_t shift = 0;  // cycles 0,2,4,6,8 then resets
//     uint8_t key_byte = 0;
   
//     for (uint32_t c = 0; c < count; c++) {
//         if (shift == 8) {
//             shift = 0;
//             *keys++ = key_byte;
//             key_byte = 0;
//         }

//         uint16_t value;
//         SVB16_IF_CONSTEXPR(UseDelta)
//         {
//             // need to do the arithmetic in unsigned space so it wraps
//             value = static_cast<uint16_t>(in[c]) - static_cast<uint16_t>(prev);
//             SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_encode(value); }
//             prev = in[c];
//         }
//         else SVB16_IF_CONSTEXPR(UseZigzag) {
//             value = detail::zigzag_encode(static_cast<uint16_t>(in[c]));
//         }
//         else {
//             value = static_cast<uint16_t>(in[c]);
//         }

//         uint8_t code = 0;
//         if (value == 0) {
//             code = 0;
//         } 
//         // 1/2 byte
//         else if (--value < 16) {
//         // if (value > 0 && (value = value - 1) < (1 << 4)){ 
            
//             // code = _encode_data_N01(value -1, &data_S, &S_shift);
//             if (S_shift == 8)
//             {
//                 S_shift = 0;
//                 data_S++;
//             }

//             auto val_masked = value & 0xf;

//             if (S_shift == 0)
//             {
//                 *data_S = 0;
//             }
//             *data_S |= val_masked << S_shift;
//             S_shift += 4;
//             code = 1;
//         }
//         // 1 byte
//         else if ((value -= 16) < 256){
//         // else if ((value = value - 16) < (1 << 8)){
//             *data_M++ = static_cast<uint8_t>(value);
//             code = 2;
//         }
//         // 2 bytes
//         else {
//             *data_L_low++ = static_cast<uint8_t>(value -= 256);
//             *data_L_high++ = static_cast<uint8_t>(value >> 8);
//             code = 3;
//         }

//         key_byte |= code << shift;
//         shift += 2;
//     }

//     if (S_shift != 0){
//         data_S += 1;
//     }
//     *keys++ = key_byte;  // write last key (no increment needed)

//     return {(size_t) (keys - keys_begin), (size_t) (data_S - data_S_begin), (size_t) (data_M - data_M_begin), (size_t) (data_L_low - data_Llow_begin),(size_t) (data_L_high - data_Lhigh_begin)};
// }



}  // namespace svb16
