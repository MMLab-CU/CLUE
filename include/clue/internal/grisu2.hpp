//  This is an implementation of Grisu2 algorithm
//
//  The implementation is a port of Loitsch's double-conversion
//  to C++11, with some minor optimization and tuning.
//
//  URL of double-conversion:
//
//      https://github.com/google/double-conversion
//
//

#ifndef CLUE_INTERNAL_GRISU2__
#define CLUE_INTERNAL_GRISU2__

#include <clue/internal/numfmt.hpp>

#define CLUE_UINT64_C(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))
#define GRISU2_ASSERT(cond) assert(cond)

namespace clue {
namespace grisu2 {

using ::std::uint64_t;
using ::std::uint32_t;

//=========================================================
//
//  Do-It-Yourself Floating Point
//
//   This "Do It Yourself Floating Point" class implements
//   a floating-point number with a uint64 significand and
//   an int exponent. Normalized DiyFp numbers will have
//   the most significant bit of the significand set.
//
//   Multiplication and Subtraction do not normalize their
//   results.DiyFp are not designed to contain special
//   doubles (NaN and Infinity).
//
//=========================================================

class DiyFp {
private:
    static constexpr uint64_t kUint64MSB = CLUE_UINT64_C(0x80000000, 00000000);
    uint64_t f_;
    int e_;

public:
    static const int kSignificandSize = 64;

    constexpr DiyFp() :
        f_(0), e_(0) {}

    constexpr DiyFp(uint64_t significand, int exponent) :
        f_(significand), e_(exponent) {}

    void Subtract(const DiyFp& other) {
        GRISU2_ASSERT(e_ == other.e_);
        GRISU2_ASSERT(f_ >= other.f_);
        f_ -= other.f_;
    }

    static DiyFp Minus(const DiyFp& a, const DiyFp& b) {
        DiyFp result = a;
        result.Subtract(b);
        return result;
    }

    void Multiply(const DiyFp& other);

    static DiyFp Times(const DiyFp& a, const DiyFp& b) {
        DiyFp result = a;
        result.Multiply(b);
        return result;
    }

    void Normalize() {
        GRISU2_ASSERT(f_ != 0);
        uint64_t significand = f_;
        int exponent = e_;

        const uint64_t k10MSBits = CLUE_UINT64_C(0xFFC00000, 00000000);
        while ((significand & k10MSBits) == 0) {
            significand <<= 10;
            exponent -= 10;
        }
        while ((significand & kUint64MSB) == 0) {
            significand <<= 1;
            exponent--;
        }
        f_ = significand;
        e_ = exponent;
    }

    static DiyFp Normalize(const DiyFp& a) {
        DiyFp result = a;
        result.Normalize();
        return result;
    }

    uint64_t f() const {
        return f_;
    }
    int e() const {
        return e_;
    }

    void set_f(uint64_t v) {
        f_ = v;
    }
    void set_e(int v) {
        e_ = v;
    }
};


class StringBuilder {

};


class DoubleToStringConverter {
public:
    static constexpr int kMaxFixedDigitsBeforePoint = 60;
    static constexpr int kMaxFixedDigitsAfterPoint = 60;
    static constexpr int kMaxExponentialDigits = 120;
    static constexpr int kMinPrecisionDigits = 1;
    static constexpr int kMaxPrecisionDigits = 120;

    enum Flags {
        NO_FLAGS = 0,
        EMIT_POSITIVE_EXPONENT_SIGN = 1,
        EMIT_TRAILING_DECIMAL_POINT = 2,
        EMIT_TRAILING_ZERO_AFTER_POINT = 4,
        UNIQUE_ZERO = 8
    };

    enum DtoaMode {
        // Produce the shortest correct representation.
        // For example the output of 0.299999999999999988897 is (the less accurate
        // but correct) 0.3.
        SHORTEST,
        // Same as SHORTEST, but for single-precision floats.
        SHORTEST_SINGLE,
        // Produce a fixed number of digits after the decimal point.
        // For instance fixed(0.1, 4) becomes 0.1000
        // If the input number is big, the output will be big.
        FIXED,
        // Fixed number of digits (independent of the decimal point).
        PRECISION
    };

    DoubleToStringConverter(int flags,
        const char* infinity_symbol,
        const char* nan_symbol,
        char exponent_character,
        int decimal_in_shortest_low,
        int decimal_in_shortest_high,
        int max_leading_padding_zeroes_in_precision_mode,
        int max_trailing_padding_zeroes_in_precision_mode)
    :
        flags_(flags),
        infinity_symbol_(infinity_symbol),
        nan_symbol_(nan_symbol),
        exponent_character_(exponent_character),
        decimal_in_shortest_low_(decimal_in_shortest_low),
        decimal_in_shortest_high_(decimal_in_shortest_high),
        max_leading_padding_zeroes_in_precision_mode_(
            max_leading_padding_zeroes_in_precision_mode),
        max_trailing_padding_zeroes_in_precision_mode_(
            max_trailing_padding_zeroes_in_precision_mode) {

        GRISU2_ASSERT(((flags & EMIT_TRAILING_DECIMAL_POINT) != 0) ||
            !((flags & EMIT_TRAILING_ZERO_AFTER_POINT) != 0));
    }

    static const DoubleToStringConverter& EcmaScriptConverter();

    bool ToShortest(double value, StringBuilder* result_builder) const {
        return ToShortestIeeeNumber(value, result_builder, SHORTEST);
    }

    // Same as ToShortest, but for single-precision floats.
    bool ToShortestSingle(float value, StringBuilder* result_builder) const {
        return ToShortestIeeeNumber(value, result_builder, SHORTEST_SINGLE);
    }

    bool ToFixed(double value, int requested_digits, StringBuilder* result_builder) const;

    bool ToExponential(double value, int requested_digits, StringBuilder* result_builder) const;

    bool ToPrecision(double value, int precision, StringBuilder* result_builder) const;

    static const int kBase10MaximalLength = 17;

    static void DoubleToAscii(double v,
                              DtoaMode mode,
                              int requested_digits,
                              char* buffer,
                              int buffer_length,
                              bool* sign,
                              int* length,
                              int* point);

private:
    // Implementation for ToShortest and ToShortestSingle.
    bool ToShortestIeeeNumber(double value,
        StringBuilder* result_builder,
        DtoaMode mode) const;

    bool HandleSpecialValues(double value, StringBuilder* result_builder) const;

    void CreateExponentialRepresentation(const char* decimal_digits,
        int length,
        int exponent,
        StringBuilder* result_builder) const;
    // Creates a decimal representation (i.e 1234.5678).
    void CreateDecimalRepresentation(const char* decimal_digits,
        int length,
        int decimal_point,
        int digits_after_point,
        StringBuilder* result_builder) const;

    const int flags_;
    const char* const infinity_symbol_;
    const char* const nan_symbol_;
    const char exponent_character_;
    const int decimal_in_shortest_low_;
    const int decimal_in_shortest_high_;
    const int max_leading_padding_zeroes_in_precision_mode_;
    const int max_trailing_padding_zeroes_in_precision_mode_;

    // DISALLOW_IMPLICIT_CONSTRUCTORS(DoubleToStringConverter);
};





} // end namespace grisu2
} // end namespace clue

#endif
