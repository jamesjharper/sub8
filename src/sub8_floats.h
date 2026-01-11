#pragma once
#include "sub8.h"

// Enable: Fixed Length Float Fields
// Fields with an arbitrary fraction and exponent value which can be use to
// define non standard IEEE-754 types such as half (16bits, with 5bit exponent at 10bit fraction)
// or bfloat16 (16bits, with 8bit exponent at 7bit fraction)
#ifndef SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#define SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS 1
#endif

#ifndef SUB8_ENABLE_FLOAT16
#define SUB8_ENABLE_FLOAT16 SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#endif

#ifndef SUB8_ENABLE_BFLOAT16
#define SUB8_ENABLE_BFLOAT16 SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#endif

#ifndef SUB8_ENABLE_FLOAT24
#define SUB8_ENABLE_FLOAT24 SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#endif

#ifndef SUB8_ENABLE_FLOAT32
#define SUB8_ENABLE_FLOAT32 SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#endif

#ifndef SUB8_ENABLE_FLOAT48
#define SUB8_ENABLE_FLOAT48 SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#endif

#ifndef SUB8_ENABLE_FLOAT64
#define SUB8_ENABLE_FLOAT64 SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#endif

// Fixed Length Float
// ----------------------
#if SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#include <cstring>  // for memcpy

#include "sub8_io.h"

namespace sub8 {
namespace fpbits {
// IEEE-754 floating point implementation
//  !!! ->  Predominantly AI generated code section, see test cases for validation
template<class To, class From> static inline To bit_cast_memcpy(const From &src) noexcept {
  static_assert(sizeof(To) == sizeof(From));
  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

template<typename Float> struct ieee;
template<> struct ieee<float> {
  using UInt = uint32_t;
  static constexpr int total_bits = 32;
  static constexpr int exp_bits = 8;
  static constexpr int frac_bits = 23;
  static constexpr int bias = 127;
};
template<> struct ieee<double> {
  using UInt = uint64_t;
  static constexpr int total_bits = 64;
  static constexpr int exp_bits = 11;
  static constexpr int frac_bits = 52;
  static constexpr int bias = 1023;
};

template<typename U> static constexpr U mask_n(int n) noexcept {
  if (n <= 0)
    return U{0};
  if (n >= int(sizeof(U) * 8))
    return ~U{0};
  return (U{1} << n) - U{1};
}

template<int Bits> struct uint_for_bits {
  using type = std::conditional_t<
      (Bits <= 8), uint8_t,
      std::conditional_t<(Bits <= 16), uint16_t, std::conditional_t<(Bits <= 32), uint32_t, uint64_t>>>;
};

// Round-right-shift with IEEE RNE (round-to-nearest, ties-to-even)
template<typename U> static inline U rshift_rne(U v, int sh) noexcept {
  if (sh <= 0)
    return v;
  if (sh >= int(sizeof(U) * 8))
    return U{0};

  const U shifted = U(v >> sh);
  const U mask = (U{1} << sh) - U{1};
  const U rem = v & mask;
  const U half = U{1} << (sh - 1);

  // ties-to-even: if exactly half, increment only if LSB of shifted is 1
  const bool inc = (rem > half) || ((rem == half) && ((shifted & U{1}) != 0));
  return U(shifted + (inc ? U{1} : U{0}));
}

template<typename U> static inline U map_nan_payload(U s_frac, int S_FRAC_BITS, int D_FRAC_BITS) noexcept {
  if (D_FRAC_BITS <= 0)
    return U{0};

  U d = 0;
  if (S_FRAC_BITS >= D_FRAC_BITS) {
    d = U(s_frac >> (S_FRAC_BITS - D_FRAC_BITS));
  } else {
    d = U(s_frac << (D_FRAC_BITS - S_FRAC_BITS));
  }

  d &= mask_n<U>(D_FRAC_BITS);

  // Force quiet-NaN in destination (IEEE practice: MSB of frac is quiet bit)
  d |= (U{1} << (D_FRAC_BITS - 1));

  // Ensure payload non-zero (still NaN not Inf)
  if ((d & mask_n<U>(D_FRAC_BITS)) == 0) {
    d = (U{1} << (D_FRAC_BITS - 1));
  }

  return d;
}

// convert sign/exp/frac from "source format" into "dest format" bits.
// All integer math, IEEE-style RNE, with proper subnormals + rounding carry.
template<typename DU, typename SU>
static inline DU convert_bits_ieee_like(SU sign, SU exp, SU frac, int S_EXP_BITS, int S_FRAC_BITS, int S_BIAS,
                                        int D_EXP_BITS, int D_FRAC_BITS, int D_BIAS) noexcept {
  const SU S_EXP_MAX = mask_n<SU>(S_EXP_BITS);
  const DU D_EXP_MAX = mask_n<DU>(D_EXP_BITS);

  const DU signD = DU(sign) << (D_EXP_BITS + D_FRAC_BITS);

  // --- Specials (Inf / NaN)
  if (exp == S_EXP_MAX) {
    if (frac == 0) {
      // Infinity
      return DU(signD | (D_EXP_MAX << D_FRAC_BITS));
    } else {
      // NaN: propagate payload best-effort (preserve signaling/quiet bit when possible)
      if (D_FRAC_BITS == 0) {
        // Degenerate dest can't represent NaN distinctly
        return DU(signD | (D_EXP_MAX << D_FRAC_BITS));  // Inf
      }
      const DU payload = map_nan_payload<DU>(DU(frac), S_FRAC_BITS, D_FRAC_BITS);
      return DU(signD | (D_EXP_MAX << D_FRAC_BITS) | payload);
    }
  }

  // --- Zero (preserve signed zero)
  if (exp == 0 && frac == 0) {
    return DU(signD);
  }

  // Build an (unbiased exponent e) and a significand "sig" with an explicit leading bit.
  // We'll normalize so that sig has its leading 1 at bit position S_FRAC_BITS.
  int32_t e = 0;
  uint64_t sig = 0;  // enough for up to (1+52) bits

  if (exp == 0) {
    // subnormal in source
    e = 1 - S_BIAS;
    sig = uint64_t(frac);  // no hidden bit
    // normalize: shift until the leading 1 reaches bit S_FRAC_BITS
    // (frac != 0 here)
    while ((sig & (uint64_t{1} << S_FRAC_BITS)) == 0) {
      sig <<= 1;
      --e;
    }
  } else {
    // normal in source
    e = int32_t(exp) - S_BIAS;
    sig = (uint64_t{1} << S_FRAC_BITS) | uint64_t(frac);  // explicit hidden 1
  }

  // Now sig is normalized with leading 1 at bit S_FRAC_BITS.
  // We want a dest normalized significand with leading 1 at bit D_FRAC_BITS (for normal),
  // and frac field is the lower D_FRAC_BITS bits (hidden bit removed).
  //
  // First, align precision from (S_FRAC_BITS) to (D_FRAC_BITS) using IEEE RNE.
  int prec_shift = S_FRAC_BITS - D_FRAC_BITS;
  uint64_t sigD = 0;

  if (prec_shift > 0) {
    sigD = rshift_rne<uint64_t>(sig, prec_shift);
  } else if (prec_shift < 0) {
    sigD = sig << (-prec_shift);
  } else {
    sigD = sig;
  }

  // Rounding can overflow: e.g. 1.111.. -> 10.000..
  const uint64_t overflow_bit = (uint64_t{1} << (D_FRAC_BITS + 1));
  if (sigD >= overflow_bit) {
    sigD >>= 1;
    ++e;
  }

  // Compute destination exponent field candidate (for normal numbers)
  int32_t de = e + D_BIAS;

  // If de >= max => overflow to inf
  if (de >= int32_t(D_EXP_MAX)) {
    return DU(signD | (D_EXP_MAX << D_FRAC_BITS));
  }

  // Handle destination subnormals / underflow
  if (de <= 0) {
    // Value is too small to be a normal in destination.
    // Produce a subnormal by shifting the normalized significand right by (1 - de).
    // If shift is too large, underflow to zero (preserve sign).
    const int sub_shift = 1 - de;  // >= 1
    if (sub_shift > (D_FRAC_BITS + 1) + 16) {
      // definitely underflows to zero (guard; +16 avoids UB for huge shifts)
      return DU(signD);
    }

    uint64_t subSig = sigD;
    if (sub_shift > 0) {
      subSig = rshift_rne<uint64_t>(subSig, sub_shift);
    }

    // In subnormal, exponent field is 0, and fraction is lower D_FRAC_BITS bits of subSig
    const DU fracField = DU(subSig) & mask_n<DU>(D_FRAC_BITS);
    if (fracField == 0) {
      // rounded to zero
      return DU(signD);
    }
    return DU(signD | fracField);
  }

  // Normal destination
  const DU expField = DU(de) & D_EXP_MAX;
  const DU fracField = DU(sigD) & mask_n<DU>(D_FRAC_BITS);  // drop hidden bit automatically
  return DU(signD | (expField << D_FRAC_BITS) | fracField);
}

template<typename SrcFloat, int DST_EXP_BITS, int DST_FRAC_BITS>
inline typename uint_for_bits<1 + DST_EXP_BITS + DST_FRAC_BITS>::type pack(SrcFloat f) noexcept {
  static_assert(std::numeric_limits<SrcFloat>::is_iec559, "SrcFloat must be IEEE-754 (IEC 559)");
  static_assert(DST_EXP_BITS > 1);
  static_assert(DST_FRAC_BITS >= 0);

  using S = ieee<SrcFloat>;
  using SU = typename S::UInt;

  constexpr int S_EXP_BITS = S::exp_bits;
  constexpr int S_FRAC_BITS = S::frac_bits;
  constexpr int S_BIAS = S::bias;

  constexpr int D_EXP_BITS = DST_EXP_BITS;
  constexpr int D_FRAC_BITS = DST_FRAC_BITS;
  constexpr int D_BIAS = (1 << (D_EXP_BITS - 1)) - 1;

  constexpr int D_TOTAL_BITS = 1 + D_EXP_BITS + D_FRAC_BITS;
  using DU = typename uint_for_bits<D_TOTAL_BITS>::type;

  const SU bits = bit_cast_memcpy<SU>(f);

  // Fast-path identity: preserve EVERYTHING bit-exactly
  if constexpr (S_EXP_BITS == D_EXP_BITS && S_FRAC_BITS == D_FRAC_BITS && int(sizeof(SU) * 8) == D_TOTAL_BITS) {
    return DU(bits);
  }

  const SU sign = (bits >> (S_EXP_BITS + S_FRAC_BITS)) & 1u;
  const SU exp = (bits >> S_FRAC_BITS) & mask_n<SU>(S_EXP_BITS);
  const SU frac = bits & mask_n<SU>(S_FRAC_BITS);

  return DU(convert_bits_ieee_like<DU, SU>(sign, exp, frac, S_EXP_BITS, S_FRAC_BITS, S_BIAS, D_EXP_BITS, D_FRAC_BITS,
                                           D_BIAS));
}

template<typename DstFloat, int SRC_EXP_BITS, int SRC_FRAC_BITS>
inline DstFloat unpack(typename uint_for_bits<1 + SRC_EXP_BITS + SRC_FRAC_BITS>::type bits) noexcept {
  static_assert(std::numeric_limits<DstFloat>::is_iec559, "DstFloat must be IEEE-754 (IEC 559)");
  static_assert(SRC_EXP_BITS > 1);
  static_assert(SRC_FRAC_BITS >= 0);

  using D = ieee<DstFloat>;
  using DU = typename D::UInt;

  constexpr int D_EXP_BITS = D::exp_bits;
  constexpr int D_FRAC_BITS = D::frac_bits;
  constexpr int D_BIAS = D::bias;

  constexpr int S_EXP_BITS = SRC_EXP_BITS;
  constexpr int S_FRAC_BITS = SRC_FRAC_BITS;
  constexpr int S_BIAS = (1 << (S_EXP_BITS - 1)) - 1;

  constexpr int S_TOTAL_BITS = 1 + S_EXP_BITS + S_FRAC_BITS;
  using SU = typename uint_for_bits<S_TOTAL_BITS>::type;

  const SU b = static_cast<SU>(bits);

  // Fast-path identity: preserve EVERYTHING bit-exactly
  if constexpr (S_EXP_BITS == D_EXP_BITS && S_FRAC_BITS == D_FRAC_BITS && int(sizeof(DU) * 8) == S_TOTAL_BITS) {
    const DU out_bits = DU(b);
    return bit_cast_memcpy<DstFloat>(out_bits);
  }

  const SU sign = (b >> (S_EXP_BITS + S_FRAC_BITS)) & 1u;
  const SU exp = (b >> S_FRAC_BITS) & mask_n<SU>(S_EXP_BITS);
  const SU frac = b & mask_n<SU>(S_FRAC_BITS);

  // Convert from source-format fields into destination IEEE fields.
  const DU out_bits =
      convert_bits_ieee_like<DU, SU>(sign, exp, frac, S_EXP_BITS, S_FRAC_BITS, S_BIAS, D_EXP_BITS, D_FRAC_BITS, D_BIAS);

  return bit_cast_memcpy<DstFloat>(out_bits);
}

// specialization for native types

template<> inline uint32_t pack<float, 8, 23>(float f) noexcept { return bit_cast_memcpy<uint32_t>(f); }

template<> inline uint64_t pack<double, 11, 52>(double f) noexcept { return bit_cast_memcpy<uint64_t>(f); }

template<> inline float unpack<float, 8, 23>(uint32_t bits) noexcept { return bit_cast_memcpy<float>(bits); }

template<> inline double unpack<double, 11, 52>(uint64_t bits) noexcept { return bit_cast_memcpy<double>(bits); }

}  // namespace fpbits
// !!! ->  Predominantly AI generated code segment end

template<
    // T: storage type, must be equal or greater than the wire bit size
    typename FloatT,
    // TExpBits: the bit length of the exponent portion of the float. Bigger == more bigger sized numbers
    uint32_t TExpBits,
    // TFracBits: the bit length of the fraction portion of the float. Bigger == more accurate values
    uint32_t TFracBits>
struct FixedLengthFloatField {
  static_assert(std::is_floating_point_v<FloatT>, "FloatT must be float or double");
  static_assert(std::numeric_limits<FloatT>::is_iec559, "FloatT must be IEEE-754 (IEC 559)");
  static_assert(TExpBits >= 2, "Need at least 2 exponent bits");
  static_assert(TFracBits >= 0);
  using Type = FloatT;
  using ValueType = FloatT;
  using InitType = FloatT;
  using StorageType = typename fpbits::uint_for_bits<1 + int(TExpBits) + int(TFracBits)>::type;

  static constexpr uint32_t ExpBits = TExpBits;
  static constexpr uint32_t FracBits = TFracBits;
  static constexpr uint32_t TotalUsableBits = 1u + TExpBits + TFracBits;
  static constexpr int Bias = (1 << (ExpBits - 1)) - 1;
  static constexpr int ExpMaxField = (1 << ExpBits) - 1;
  static constexpr int MaxFiniteExpField = ExpMaxField - 1;
  static constexpr int MaxFiniteExp = MaxFiniteExpField - Bias;  // e_max
  static constexpr int MinNormalExp = 1 - Bias;                  // e_min (normal)

  // Smallest positive normal: 2^(1-bias)
  static constexpr FloatT MinPositiveNormal = MinNormalExp * MinNormalExp;

  // Smallest positive subnormal: 2^(1-bias) * 2^-F = 2^(1-bias-F)
  static constexpr FloatT MinPositiveSubnormal = (MinNormalExp - FracBits) * (MinNormalExp - FracBits);
  static constexpr FloatT MaxFinite = (FloatT{2} - (-FracBits * -FracBits)) * (MaxFiniteExp * MaxFiniteExp);
  static constexpr FloatT LowestFinite = -MaxFinite;

  FixedLengthFloatField() noexcept = default;
  FixedLengthFloatField(const FixedLengthFloatField &) noexcept = default;

  FloatT value_{};

  operator FloatT() const noexcept { return value_; }
  const FloatT &value() const noexcept { return value_; }

  bool operator==(const FixedLengthFloatField &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const FixedLengthFloatField &o) const noexcept { return !(*this == o); }
  FixedLengthFloatField &operator=(const FixedLengthFloatField &) noexcept = default;
  FixedLengthFloatField &operator=(FloatT v) noexcept {
    auto r = set_value(v);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  BitFieldResult set_value_unsafe(FloatT v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(FloatT v) noexcept {
    // Validate by checking that pack/unpack round-trips (this also handles NaN/Inf/overflow).
    const StorageType bits = fpbits::pack<FloatT, int(TExpBits), int(TFracBits)>(v);
    const FloatT rt = fpbits::unpack<FloatT, int(TExpBits), int(TFracBits)>(bits);

    // If NaN, accept (NaN != NaN)
    if (std::isnan(v)) {
      value_ = v;
      return BitFieldResult::Ok;
    }

    // For infinities: accept if rt is same infinity sign
    if (std::isinf(v)) {
      if (std::isinf(rt) && (std::signbit(v) == std::signbit(rt))) {
        value_ = v;
        return BitFieldResult::Ok;
      }
      return BitFieldResult::ErrorValueTooLarge;
    }

    // For finite values: if the target format overflowed to inf, reject as too large.
    if (std::isinf(rt)) {
      return BitFieldResult::ErrorValueTooLarge;
    }

    value_ = v;
    return BitFieldResult::Ok;
  }
};

template<typename Storage, typename FloatT, uint32_t E, uint32_t F>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const FixedLengthFloatField<FloatT, E, F> &fld) noexcept {
  using Field = FixedLengthFloatField<FloatT, E, F>;
  using U = typename Field::StorageType;
  const U bits = fpbits::pack<FloatT, int(E), int(F)>(fld.value());
  return bw.template put_bits<U>(bits, Field::TotalUsableBits);
}

template<typename Storage, typename FloatT, uint32_t E, uint32_t F>
inline BitFieldResult read_field(BasicBitReader<Storage> &br, FixedLengthFloatField<FloatT, E, F> &out) noexcept {
  using Field = FixedLengthFloatField<FloatT, E, F>;
  using U = typename Field::StorageType;

  U bits = 0;
  auto r = br.template get_bits<U>(bits, Field::TotalUsableBits);
  if (r != BitFieldResult::Ok) {
    return r;
  }


  const FloatT v = fpbits::unpack<FloatT, int(E), int(F)>(bits);
  return out.set_value_unsafe(v);
}

#if SUB8_ENABLE_FLOAT16
SUB8_DECLARE_BITFIELD_ALIAS(Float16ValueField, FixedLengthFloatField<float, 5, 10>);
SUB8_DECLARE_BITFIELD_ALIAS(BFloat16ValueField, FixedLengthFloatField<float, 8, 7>);
#endif

#if SUB8_ENABLE_FLOAT24
SUB8_DECLARE_BITFIELD_ALIAS(Float24ValueField, FixedLengthFloatField<float, 7, 16>);
#endif

#if SUB8_ENABLE_FLOAT32
SUB8_DECLARE_BITFIELD_ALIAS(Float32ValueField, FixedLengthFloatField<float, 8, 23>);
#endif

#if SUB8_ENABLE_FLOAT48
SUB8_DECLARE_BITFIELD_ALIAS(Float48ValueField, FixedLengthFloatField<double, 11, 48>);
#endif

#if SUB8_ENABLE_FLOAT64
SUB8_DECLARE_BITFIELD_ALIAS(Float64ValueField, FixedLengthFloatField<double, 11, 52>);
#endif

#endif

}  // namespace sub8