#pragma once

#include <stdint.h>

namespace pgnano { namespace standalone
{

int16_t constexpr inline predict_jump(int16_t previous_sample) noexcept
{
    return previous_sample;
}

int16_t constexpr inline predict_non_jump(int16_t previous_sample) noexcept
{
    return previous_sample;
}

}}