#pragma once

#include <stdint.h>
#include <type_traits>
#include "stddef.h"
#include "gsl/gsl_fit.h"

namespace pgnano
{
// Perform exponentiation using only integer arithmetic
template <typename T>
inline T uint_pow(T base, T exp)
{
    T res = 1;
    for (T i = 0; i < exp; i++)
        res *= base;
    return res;
}

/*
    Fit to y = m*x+n
    Return 0 on success
    non-0 on failure
*/
inline int double_linear_lms(const double *X, const double *Y, const size_t vector_length, double *m, double *n)
{
    double discard_1, discard_2, discard_3, discard_4;
    return gsl_fit_linear(X, 1, Y, 1, vector_length, n, m, &discard_1, &discard_2, &discard_3, &discard_4);
}

template <typename Rep, uint8_t PrecisionBits>
class CustomFixedPoint
{
public:
    inline constexpr CustomFixedPoint<Rep, PrecisionBits> from_double(double x);
    inline constexpr CustomFixedPoint<Rep, PrecisionBits> operator+(const CustomFixedPoint<Rep, PrecisionBits> y)
    {
        return this->m_rep + y.m_rep;
    }
    inline constexpr CustomFixedPoint<Rep, PrecisionBits> operator*(const CustomFixedPoint<Rep, PrecisionBits> y)
    {
        return (this->m_rep >> HalfPrecisionBits) * (y >> HalfPrecisionBits);
    }
private:
    Rep m_rep;

    static constexpr bool left_shift_is_arithmetic_shift = (static_cast<Rep>(-4) >> 1) == static_cast<Rep>(-2);
    static_assert(left_shift_is_arithmetic_shift); // Only arithmetic shifts for left shifts are supported in this implementation. TODO: expand in order not to have this constraint
    static constexpr bool even_precision_bits = PrecisionBits % 2 == 0;
    static_assert(even_precision_bits); // Only even number of precision bits are allowed in this implementation. TODO: expand in order not to have this constraint 
    static constexpr uint8_t HalfPrecisionBits = PrecisionBits >> 1;
};




constexpr uint8_t PRECISION_BITS = 32;
typedef CustomFixedPoint<int64_t, PRECISION_BITS> fixed_point;

/*
    Fit to y = m*x+n
    Return 0 on success
    non-0 on failure
*/
inline int linear_lms(const double *X, const double *Y, const size_t vector_length, fixed_point *m, fixed_point *n)
{
    double d_m, d_n;
    auto ret_code = double_linear_lms(X,Y,vector_length,&d_m,&d_n);
    if (ret_code != 0)
        return ret_code;
    
}

};