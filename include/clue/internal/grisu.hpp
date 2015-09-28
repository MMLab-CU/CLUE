// This is an implementation of Milo's optimized Grisu algorithm.
//
// The implementation is adapted from RapidJSON by Milo Yip:
//
//    https://github.com/miloyip/rapidjson
//


#ifndef CLUE_INTERNAL_GRISU__
#define CLUE_INTERNAL_GRISU__

#include <clue/internal/numfmt.hpp>

#define CLUE_UINT64_HL(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))

namespace clue {
namespace grisu_impl {

//===============================================
//
//   class DiyFp
//
//===============================================

using ::std::uint32_t;
using ::std::uint64_t;

struct DiyFp {
    // fields

    uint64_t f;
    int e;

    // constructors
    constexpr DiyFp() noexcept :
        f(0), e(0) {}

    constexpr DiyFp(uint64_t fp, int exp) noexcept :
        f(fp), e(exp) {}

    explicit DiyFp(double d) noexcept {
        union {
            double d;
            uint64_t u64;
        } u = { d };

        int biased_e = static_cast<int>((u.u64 & kDpExponentMask) >> kDpSignificandSize);
        uint64_t significand = (u.u64 & kDpSignificandMask);
        if (biased_e != 0) {
            f = significand + kDpHiddenBit;
            e = biased_e - kDpExponentBias;
        }
        else {
            f = significand;
            e = kDpMinExponent + 1;
        }
    }

    // operations

    constexpr DiyFp operator-(const DiyFp& rhs) const {
        return DiyFp(f - rhs.f, e);
    }

    //
    // platform-dependent optimized multiplication
    //
    DiyFp operator*(const DiyFp& rhs) const {
#if defined(_MSC_VER) && defined(_M_AMD64)
        uint64_t h;
        uint64_t l = _umul128(f, rhs.f, &h);
        if (l & (uint64_t(1) << 63)) // rounding
            h++;
        return DiyFp(h, e + rhs.e + 64);
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__x86_64__)
        __extension__ typedef unsigned __int128 uint128;
        uint128 p = static_cast<uint128>(f) * static_cast<uint128>(rhs.f);
        uint64_t h = static_cast<uint64_t>(p >> 64);
        uint64_t l = static_cast<uint64_t>(p);
        if (l & (uint64_t(1) << 63)) // rounding
            h++;
        return DiyFp(h, e + rhs.e + 64);
#else
        const uint64_t M32 = 0xFFFFFFFF;
        const uint64_t a = f >> 32;
        const uint64_t b = f & M32;
        const uint64_t c = rhs.f >> 32;
        const uint64_t d = rhs.f & M32;
        const uint64_t ac = a * c;
        const uint64_t bc = b * c;
        const uint64_t ad = a * d;
        const uint64_t bd = b * d;
        uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32);
        tmp += 1U << 31;  /// mult_round
        return DiyFp(ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), e + rhs.e + 64);
#endif
    }

    DiyFp Normalize() const {
#if defined(_MSC_VER) && defined(_M_AMD64)
        unsigned long index;
        _BitScanReverse64(&index, f);
        return DiyFp(f << (63 - index), e - (63 - index));
#elif defined(__GNUC__) && __GNUC__ >= 4
        int s = __builtin_clzll(f);
        return DiyFp(f << s, e - s);
#else
        DiyFp res = *this;
        while (!(res.f & (static_cast<uint64_t>(1) << 63))) {
            res.f <<= 1;
            res.e--;
        }
        return res;
#endif
    }

    DiyFp NormalizeBoundary() const {
        DiyFp res = *this;
        while (!(res.f & (kDpHiddenBit << 1))) {
            res.f <<= 1;
            res.e--;
        }
        res.f <<= (kDiySignificandSize - kDpSignificandSize - 2);
        res.e = res.e - (kDiySignificandSize - kDpSignificandSize - 2);
        return res;
    }

    void NormalizedBoundaries(DiyFp* minus, DiyFp* plus) const {
        DiyFp pl = DiyFp((f << 1) + 1, e - 1).NormalizeBoundary();
        DiyFp mi = (f == kDpHiddenBit) ? DiyFp((f << 2) - 1, e - 2) : DiyFp((f << 1) - 1, e - 1);
        mi.f <<= mi.e - pl.e;
        mi.e = pl.e;
        *plus = pl;
        *minus = mi;
    }

    double ToDouble() const {
        union {
            double d;
            uint64_t u64;
        }u;
        const uint64_t be = (e == kDpDenormalExponent && (f & kDpHiddenBit) == 0) ? 0 :
            static_cast<uint64_t>(e + kDpExponentBias);
        u.u64 = (f & kDpSignificandMask) | (be << kDpSignificandSize);
        return u.d;
    }

    static constexpr int kDiySignificandSize = 64;
    static constexpr int kDpSignificandSize = 52;
    static constexpr int kDpExponentBias = 0x3FF + kDpSignificandSize;
    static constexpr int kDpMaxExponent = 0x7FF - kDpExponentBias;
    static constexpr int kDpMinExponent = -kDpExponentBias;
    static constexpr int kDpDenormalExponent = -kDpExponentBias + 1;
    static constexpr uint64_t kDpExponentMask = CLUE_UINT64_HL(0x7FF00000, 0x00000000);
    static constexpr uint64_t kDpSignificandMask = CLUE_UINT64_HL(0x000FFFFF, 0xFFFFFFFF);
    static constexpr uint64_t kDpHiddenBit = CLUE_UINT64_HL(0x00100000, 0x00000000);

}; // end class DiyFp


inline DiyFp GetCachedPowerByIndex(size_t index) {
    // 10^-348, 10^-340, ..., 10^340
    static const uint64_t kCachedPowers_F[] = {
        CLUE_UINT64_HL(0xfa8fd5a0, 0x081c0288), CLUE_UINT64_HL(0xbaaee17f, 0xa23ebf76),
        CLUE_UINT64_HL(0x8b16fb20, 0x3055ac76), CLUE_UINT64_HL(0xcf42894a, 0x5dce35ea),
        CLUE_UINT64_HL(0x9a6bb0aa, 0x55653b2d), CLUE_UINT64_HL(0xe61acf03, 0x3d1a45df),
        CLUE_UINT64_HL(0xab70fe17, 0xc79ac6ca), CLUE_UINT64_HL(0xff77b1fc, 0xbebcdc4f),
        CLUE_UINT64_HL(0xbe5691ef, 0x416bd60c), CLUE_UINT64_HL(0x8dd01fad, 0x907ffc3c),
        CLUE_UINT64_HL(0xd3515c28, 0x31559a83), CLUE_UINT64_HL(0x9d71ac8f, 0xada6c9b5),
        CLUE_UINT64_HL(0xea9c2277, 0x23ee8bcb), CLUE_UINT64_HL(0xaecc4991, 0x4078536d),
        CLUE_UINT64_HL(0x823c1279, 0x5db6ce57), CLUE_UINT64_HL(0xc2109436, 0x4dfb5637),
        CLUE_UINT64_HL(0x9096ea6f, 0x3848984f), CLUE_UINT64_HL(0xd77485cb, 0x25823ac7),
        CLUE_UINT64_HL(0xa086cfcd, 0x97bf97f4), CLUE_UINT64_HL(0xef340a98, 0x172aace5),
        CLUE_UINT64_HL(0xb23867fb, 0x2a35b28e), CLUE_UINT64_HL(0x84c8d4df, 0xd2c63f3b),
        CLUE_UINT64_HL(0xc5dd4427, 0x1ad3cdba), CLUE_UINT64_HL(0x936b9fce, 0xbb25c996),
        CLUE_UINT64_HL(0xdbac6c24, 0x7d62a584), CLUE_UINT64_HL(0xa3ab6658, 0x0d5fdaf6),
        CLUE_UINT64_HL(0xf3e2f893, 0xdec3f126), CLUE_UINT64_HL(0xb5b5ada8, 0xaaff80b8),
        CLUE_UINT64_HL(0x87625f05, 0x6c7c4a8b), CLUE_UINT64_HL(0xc9bcff60, 0x34c13053),
        CLUE_UINT64_HL(0x964e858c, 0x91ba2655), CLUE_UINT64_HL(0xdff97724, 0x70297ebd),
        CLUE_UINT64_HL(0xa6dfbd9f, 0xb8e5b88f), CLUE_UINT64_HL(0xf8a95fcf, 0x88747d94),
        CLUE_UINT64_HL(0xb9447093, 0x8fa89bcf), CLUE_UINT64_HL(0x8a08f0f8, 0xbf0f156b),
        CLUE_UINT64_HL(0xcdb02555, 0x653131b6), CLUE_UINT64_HL(0x993fe2c6, 0xd07b7fac),
        CLUE_UINT64_HL(0xe45c10c4, 0x2a2b3b06), CLUE_UINT64_HL(0xaa242499, 0x697392d3),
        CLUE_UINT64_HL(0xfd87b5f2, 0x8300ca0e), CLUE_UINT64_HL(0xbce50864, 0x92111aeb),
        CLUE_UINT64_HL(0x8cbccc09, 0x6f5088cc), CLUE_UINT64_HL(0xd1b71758, 0xe219652c),
        CLUE_UINT64_HL(0x9c400000, 0x00000000), CLUE_UINT64_HL(0xe8d4a510, 0x00000000),
        CLUE_UINT64_HL(0xad78ebc5, 0xac620000), CLUE_UINT64_HL(0x813f3978, 0xf8940984),
        CLUE_UINT64_HL(0xc097ce7b, 0xc90715b3), CLUE_UINT64_HL(0x8f7e32ce, 0x7bea5c70),
        CLUE_UINT64_HL(0xd5d238a4, 0xabe98068), CLUE_UINT64_HL(0x9f4f2726, 0x179a2245),
        CLUE_UINT64_HL(0xed63a231, 0xd4c4fb27), CLUE_UINT64_HL(0xb0de6538, 0x8cc8ada8),
        CLUE_UINT64_HL(0x83c7088e, 0x1aab65db), CLUE_UINT64_HL(0xc45d1df9, 0x42711d9a),
        CLUE_UINT64_HL(0x924d692c, 0xa61be758), CLUE_UINT64_HL(0xda01ee64, 0x1a708dea),
        CLUE_UINT64_HL(0xa26da399, 0x9aef774a), CLUE_UINT64_HL(0xf209787b, 0xb47d6b85),
        CLUE_UINT64_HL(0xb454e4a1, 0x79dd1877), CLUE_UINT64_HL(0x865b8692, 0x5b9bc5c2),
        CLUE_UINT64_HL(0xc83553c5, 0xc8965d3d), CLUE_UINT64_HL(0x952ab45c, 0xfa97a0b3),
        CLUE_UINT64_HL(0xde469fbd, 0x99a05fe3), CLUE_UINT64_HL(0xa59bc234, 0xdb398c25),
        CLUE_UINT64_HL(0xf6c69a72, 0xa3989f5c), CLUE_UINT64_HL(0xb7dcbf53, 0x54e9bece),
        CLUE_UINT64_HL(0x88fcf317, 0xf22241e2), CLUE_UINT64_HL(0xcc20ce9b, 0xd35c78a5),
        CLUE_UINT64_HL(0x98165af3, 0x7b2153df), CLUE_UINT64_HL(0xe2a0b5dc, 0x971f303a),
        CLUE_UINT64_HL(0xa8d9d153, 0x5ce3b396), CLUE_UINT64_HL(0xfb9b7cd9, 0xa4a7443c),
        CLUE_UINT64_HL(0xbb764c4c, 0xa7a44410), CLUE_UINT64_HL(0x8bab8eef, 0xb6409c1a),
        CLUE_UINT64_HL(0xd01fef10, 0xa657842c), CLUE_UINT64_HL(0x9b10a4e5, 0xe9913129),
        CLUE_UINT64_HL(0xe7109bfb, 0xa19c0c9d), CLUE_UINT64_HL(0xac2820d9, 0x623bf429),
        CLUE_UINT64_HL(0x80444b5e, 0x7aa7cf85), CLUE_UINT64_HL(0xbf21e440, 0x03acdd2d),
        CLUE_UINT64_HL(0x8e679c2f, 0x5e44ff8f), CLUE_UINT64_HL(0xd433179d, 0x9c8cb841),
        CLUE_UINT64_HL(0x9e19db92, 0xb4e31ba9), CLUE_UINT64_HL(0xeb96bf6e, 0xbadf77d9),
        CLUE_UINT64_HL(0xaf87023b, 0x9bf0ee6b)
    };
    static const int16_t kCachedPowers_E[] = {
        -1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007,  -980,
        -954,  -927,  -901,  -874,  -847,  -821,  -794,  -768,  -741,  -715,
        -688,  -661,  -635,  -608,  -582,  -555,  -529,  -502,  -475,  -449,
        -422,  -396,  -369,  -343,  -316,  -289,  -263,  -236,  -210,  -183,
        -157,  -130,  -103,   -77,   -50,   -24,     3,    30,    56,    83,
        109,   136,   162,   189,   216,   242,   269,   295,   322,   348,
        375,   402,   428,   455,   481,   508,   534,   561,   588,   614,
        641,   667,   694,   720,   747,   774,   800,   827,   853,   880,
        907,   933,   960,   986,  1013,  1039,  1066
    };
    return DiyFp(kCachedPowers_F[index], kCachedPowers_E[index]);
}

inline DiyFp GetCachedPower(int e, int* K) {
    double dk = (-61 - e) * 0.30102999566398114 + 347;
    int k = static_cast<int>(dk);
    if (dk - k > 0.0)
        k++;

    unsigned index = static_cast<unsigned>((k >> 3) + 1);
    *K = -(-348 + static_cast<int>(index << 3));

    return GetCachedPowerByIndex(index);
}

inline DiyFp GetCachedPower10(int exp, int *outExp) {
     unsigned index = (static_cast<unsigned>(exp) + 348u) / 8u;
     *outExp = -348 + static_cast<int>(index) * 8;
     return GetCachedPowerByIndex(index);
}


//===============================================
//
//   Grisu: double -> string
//
//===============================================

inline void GrisuRound(char* buffer, int len, uint64_t delta, uint64_t rest, uint64_t ten_kappa, uint64_t wp_w) {
    while (rest < wp_w && delta - rest >= ten_kappa &&
           (rest + ten_kappa < wp_w ||  /// closer
            wp_w - rest > rest + ten_kappa - wp_w)) {
        buffer[len - 1]--;
        rest += ten_kappa;
    }
}

inline unsigned CountDecimalDigit32(uint32_t n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;  // won't reach 10 digitis in DigitGen
    return 9;
}

inline void DigitGen(const DiyFp& W, const DiyFp& Mp, uint64_t delta, char* buffer, int* len, int* K) {
    static const uint32_t kPow10[] = {
        1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
    };
    const DiyFp one(uint64_t(1) << -Mp.e, Mp.e);
    const DiyFp wp_w = Mp - W;
    uint32_t p1 = static_cast<uint32_t>(Mp.f >> -one.e);
    uint64_t p2 = Mp.f & (one.f - 1);
    unsigned kappa = CountDecimalDigit32(p1); // kappa in [0, 9]
    *len = 0;

    while (kappa > 0) {
        uint32_t d = 0;
        switch (kappa) {
            case  9: d = p1 /  100000000; p1 %=  100000000; break;
            case  8: d = p1 /   10000000; p1 %=   10000000; break;
            case  7: d = p1 /    1000000; p1 %=    1000000; break;
            case  6: d = p1 /     100000; p1 %=     100000; break;
            case  5: d = p1 /      10000; p1 %=      10000; break;
            case  4: d = p1 /       1000; p1 %=       1000; break;
            case  3: d = p1 /        100; p1 %=        100; break;
            case  2: d = p1 /         10; p1 %=         10; break;
            case  1: d = p1;              p1 =           0; break;
            default:;
        }
        if (d || *len)
            buffer[(*len)++] = static_cast<char>('0' + static_cast<char>(d));
        kappa--;
        uint64_t tmp = (static_cast<uint64_t>(p1) << -one.e) + p2;
        if (tmp <= delta) {
            *K += kappa;
            GrisuRound(buffer, *len, delta, tmp,
                static_cast<uint64_t>(kPow10[kappa]) << -one.e, wp_w.f);
            return;
        }
    }

    // kappa = 0
    for (;;) {
        p2 *= 10;
        delta *= 10;
        char d = static_cast<char>(p2 >> -one.e);
        if (d || *len)
            buffer[(*len)++] = static_cast<char>('0' + d);
        p2 &= one.f - 1;
        kappa--;
        if (p2 < delta) {
            *K += kappa;
            GrisuRound(buffer, *len, delta, p2,
                one.f,
                wp_w.f * kPow10[-static_cast<int>(kappa)]);
            return;
        }
    }
}

inline void Grisu2(double value, char* buffer, int* length, int* K) {
    const DiyFp v(value);
    DiyFp w_m, w_p;
    v.NormalizedBoundaries(&w_m, &w_p);

    const DiyFp c_mk = GetCachedPower(w_p.e, K);
    const DiyFp W = v.Normalize() * c_mk;
    DiyFp Wp = w_p * c_mk;
    DiyFp Wm = w_m * c_mk;
    Wm.f++;
    Wp.f--;
    DigitGen(W, Wp, Wp.f - Wm.f, buffer, length, K);
}

inline const char* GetDigitsLut() {
    static const char cDigitsLut[200] = {
        '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
        '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
        '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
        '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
        '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
        '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
        '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
        '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
        '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
        '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
    };
    return cDigitsLut;
}

inline char* WriteExponent(int K, char* buffer) {
    if (K < 0) {
        *buffer++ = '-';
        K = -K;
    }
    if (K >= 100) {
        *buffer++ = static_cast<char>('0' + static_cast<char>(K / 100));
        K %= 100;
        const char* d = GetDigitsLut() + K * 2;
        *buffer++ = d[0];
        *buffer++ = d[1];
    }
    else if (K >= 10) {
        const char* d = GetDigitsLut() + K * 2;
        *buffer++ = d[0];
        *buffer++ = d[1];
    }
    else
        *buffer++ = static_cast<char>('0' + static_cast<char>(K));

    *buffer = '\0';
    return buffer;
}

inline char* Prettify(char* buffer, int length, int k) {
    const int kk = length + k;  // 10^(kk-1) <= v < 10^kk

    if (length <= kk && kk <= 21) {
        // 1234e7 -> 12340000000
        for (int i = length; i < kk; i++)
            buffer[i] = '0';
        buffer[kk] = '.';
        buffer[kk + 1] = '0';
        return &buffer[kk + 2];
    }
    else if (0 < kk && kk <= 21) {
        // 1234e-2 -> 12.34
        ::std::memmove(&buffer[kk + 1], &buffer[kk], static_cast<size_t>(length - kk));
        buffer[kk] = '.';
        return &buffer[length + 1];
    }
    else if (-6 < kk && kk <= 0) {
        // 1234e-6 -> 0.001234
        const int offset = 2 - kk;
        ::std::memmove(&buffer[offset], &buffer[0], static_cast<size_t>(length));
        buffer[0] = '0';
        buffer[1] = '.';
        for (int i = 2; i < offset; i++)
            buffer[i] = '0';
        return &buffer[length + offset];
    }
    else if (length == 1) {
        // 1e30
        buffer[1] = 'e';
        return WriteExponent(kk - 1, &buffer[2]);
    }
    else {
        // 1234e30 -> 1.234e33
        ::std::memmove(&buffer[2], &buffer[1], static_cast<size_t>(length - 1));
        buffer[1] = '.';
        buffer[length + 1] = 'e';
        return WriteExponent(kk - 1, &buffer[0 + length + 2]);
    }
}

inline void Grisu_DtoA(double value, char* buffer) {
    if (::std::isfinite(value)) {
        if (value == 0.0) {  // 0.0 or -0.0
            if (::std::signbit(value)) *buffer++ = '-';
            buffer[0] = '0';
            buffer[1] = '.';
            buffer[2] = '0';
        } else {
            if (value < 0) {
                *buffer++ = '-';
                value = -value;
            }
            int length, K;
            Grisu2(value, buffer, &length, &K);
            Prettify(buffer, length, K);
        }
    } else if (::std::isinf(value) ) {
        static char infsrc[4] = {'I', 'n', 'f', '\0'};
        if (::std::signbit(value)) *buffer++ = '-';
        ::std::memcpy(buffer, infsrc, 4 * sizeof(char));
    } else {
        CLUE_ASSERT(::std::isnan(value));
        static char nansrc[4] = {'N', 'a', 'N', '\0'};
        ::std::memcpy(buffer, nansrc, 4 * sizeof(char));
    }
}


} // end namespace grisu_impl
} // end namespace clue

#endif /* INCLUDE_CLUE_INTERNAL_GRISU_HPP_ */
