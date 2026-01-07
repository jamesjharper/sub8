#pragma once

#ifndef SUB8_ENABLE_ENUM_FIELDS
#define SUB8_ENABLE_ENUM_FIELDS 1
#endif

#include "sub8_basic.h"
#include "sub8_errors.h"
#include "sub8_io.h"

namespace sub8 {

// Enum Field
// ----------------------

#if SUB8_ENABLE_ENUM_FIELDS

// EnumBitField
// ------------
// Dense-range enum field that auto-computes bit width.
//
// Encodes: code = (value - MinValue) in [0..(MaxValue-MinValue)]
// Bits: ceil(log2(range_size)) clamped to >= 1.
//
// Requirements:
//  - EnumT is an enum type
//  - Enum values in the serialized set are contiguous from MinValue..MaxValue
//  - Underlying type can be signed/unsigned; encoding uses unsigned StorageType.

template<typename EnumT, EnumT MinValue, EnumT MaxValue> struct EnumBitField {
 public:
  static_assert(std::is_enum_v<EnumT>, "EnumBitField requires an enum type");

  using Type = EnumT;
  using ValueType = EnumT;
  using InitType = EnumT;

  using UnderlyingType = typename unpack_t::underlying_or_self<EnumT>::type;
  static_assert(std::is_integral_v<UnderlyingType>, "Enum underlying type must be integral");

  using StorageType = std::make_unsigned_t<UnderlyingType>;

  // ---- special members (fixes -Wdeprecated-copy) ----
  EnumBitField() noexcept = default;
  EnumBitField(const EnumBitField &) noexcept = default;
  EnumBitField(EnumBitField &&) noexcept = default;
  ~EnumBitField() noexcept = default;

  EnumBitField &operator=(const EnumBitField &) noexcept = default;
  EnumBitField &operator=(EnumBitField &&) noexcept = default;

  // ---- helpers ----
  static constexpr uint32_t ceil_log2_u32(uint32_t n) noexcept {
    uint32_t bits = 0;
    uint32_t v = n - 1;
    while (v > 0) {
      v >>= 1;
      ++bits;
    }
    return bits;
  }

  static constexpr StorageType to_storage(EnumT e) noexcept {
    return static_cast<StorageType>(static_cast<UnderlyingType>(e));
  }
  static constexpr EnumT from_storage(StorageType u) noexcept {
    return static_cast<EnumT>(static_cast<UnderlyingType>(u));
  }

  static constexpr StorageType MinCode = to_storage(MinValue);
  static constexpr StorageType MaxCode = to_storage(MaxValue);
  static_assert(MaxCode >= MinCode, "EnumBitField: MaxValue must be >= MinValue");

  static constexpr StorageType RangeSize = (MaxCode - MinCode) + StorageType{1};
  static constexpr uint32_t TotalUsableBits =
      (RangeSize <= StorageType{1}) ? 1u : ceil_log2_u32(static_cast<uint32_t>(RangeSize));

  static constexpr EnumT MinEnumValue = MinValue;
  static constexpr EnumT MaxEnumValue = MaxValue;

  EnumT value_{MinValue};

  operator EnumT() const noexcept { return value_; }
  const EnumT &value() const noexcept { return value_; }

  bool operator==(const EnumBitField &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const EnumBitField &o) const noexcept { return !(*this == o); }

  bool operator==(const EnumT &o) const noexcept { return value_ == o; }
  bool operator!=(const EnumT &o) const noexcept { return value_ != o; }

  // assignment from EnumT (your convenience op)
  EnumBitField &operator=(EnumT v) noexcept {
    auto r = set_value(v);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  BitFieldResult set_value(InitType v) noexcept {
    const StorageType code = to_storage(v);
    if (code < MinCode || code > MaxCode)
      return BitFieldResult::ErrorValueTooLarge;
    value_ = v;
    return BitFieldResult::Ok;
  }
};

template<typename Storage, typename EnumT, EnumT MinV, EnumT MaxV>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const EnumBitField<EnumT, MinV, MaxV> &f) {
  using F = EnumBitField<EnumT, MinV, MaxV>;
  using ST = typename F::StorageType;

  // Dense encoding: code = value - MinCode
  const ST raw = F::to_storage(f.value());
  if (raw < F::MinCode || raw > F::MaxCode)
    return BitFieldResult::ErrorValueTooLarge;

  const ST code = raw - F::MinCode;

  if (code >= F::RangeSize)
    return BitFieldResult::ErrorValueTooLarge;

  bw.template put_bits<ST>(code, F::TotalUsableBits);
  return BitFieldResult::Ok;
}

template<typename Storage, typename EnumT, EnumT MinV, EnumT MaxV>
inline BitFieldResult read_field(BasicBitReader<Storage> &br, EnumBitField<EnumT, MinV, MaxV> &out) {
  using F = EnumBitField<EnumT, MinV, MaxV>;
  using ST = typename F::StorageType;

  ST code = 0;
  if (!br.template get_bits<ST>(code, F::TotalUsableBits))
    return BitFieldResult::ErrorExpectedMoreBits;

  // Must fall in [0..RangeSize-1]
  if (code >= F::RangeSize)
    return BitFieldResult::ErrorValueTooLarge;

  const ST raw = code + F::MinCode;
  return out.set_value(F::from_storage(raw));
}

#endif  // SUB8_ENABLE_ENUM_FIELDS

}  // namespace sub8
