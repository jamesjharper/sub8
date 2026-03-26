#pragma once

#ifndef SUB8_ENABLE_ENUM_FIELDS
#define SUB8_ENABLE_ENUM_FIELDS 1
#endif

#include <type_traits>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_api.h"
#include "sub8_errors.h"
#include "sub8_io.h"
#endif

#include <cstdint> // uintx_t
#include <cstddef> // size_t
#include <utility> // std::declval

namespace sub8 {

// Enum Field
// ----------------------

#if SUB8_ENABLE_ENUM_FIELDS

// Enumeration
// ------------
// Dense-range enum field that auto-computes bit width.
//
// Encodes: code = (value - MinValue) in [0..(MaxValue-MinValue)]
// Bits: ceil(log2(range_size)) clamped to >= 1.
//
// Requirements:
//  - EnumT is an enum type
//  - Enum values in the serialized set are contiguous from MinValue..MaxValue
//  - Underlying type can be signed/unsigned; encoding uses unsigned
//  StorageType.

template <typename EnumT, EnumT MinEnumValue, EnumT MaxEnumValue> struct Enumeration {
public:
  using UnderlyingType = typename unpack_t::underlying_or_self<EnumT>::type;
  using StorageType = std::make_unsigned_t<UnderlyingType>;

  using Type = EnumT;
  using ValueType = EnumT;
  using InitType = EnumT;

  static_assert(std::is_integral_v<UnderlyingType>, "Enum underlying type must be integral");
  static_assert(std::is_enum_v<EnumT>, "Enumeration requires an enum type");

  static constexpr StorageType to_storage(EnumT e) noexcept { return static_cast<StorageType>(static_cast<UnderlyingType>(e)); }
  static constexpr EnumT from_storage(StorageType u) noexcept { return static_cast<EnumT>(static_cast<UnderlyingType>(u)); }

  static constexpr EnumT MinValue = MinEnumValue;
  static constexpr EnumT MaxValue = MaxEnumValue;
  static constexpr StorageType MinCode = to_storage(MinValue);
  static constexpr StorageType MaxCode = to_storage(MaxValue);
  static_assert(MaxCode >= MinCode, "Enumeration: MaxValue must be >= MinValue");

  static constexpr StorageType RangeSize = (MaxCode - MinCode) + StorageType{1};
  static_assert(RangeSize >= 1, "Enumeration: RangeSize must be >= 1");

  static constexpr BitSize ActualSize = BitSize::from_bits(RangeSize <= StorageType{1} ? 1u : limits::bitwidth_to_express_distinct_values(static_cast<size_t>(RangeSize)));
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept  { return ActualSize; }
  BitSize max_possible_size() const noexcept  { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept  { return MinPossibleSize; }

  Enumeration() noexcept = default;
  Enumeration(const Enumeration &) noexcept = default;
  Enumeration(Enumeration &&) noexcept = default;
  ~Enumeration() noexcept = default;

  Enumeration &operator=(const Enumeration &) noexcept = default;
  Enumeration &operator=(Enumeration &&) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  Enumeration &operator=(EnumT v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, Enumeration, "sub8::Enumeration(EnumT)");
    return *this;
  }
  #endif

  SUB8_NO_DISCARD static BitFieldResult make(Type v, Enumeration &out) noexcept { 
    return out.set_value(v); 
  }

  #if !SUB8_ENABLE_INFALLIBLE
  static Enumeration make_or_throw(Type v) SUB8_OPT_NO_EXCEPT {
    Enumeration out{};
    auto r = out.set_value(v);
    SUB8_THROW_IF_ERROR(r, Enumeration, "Enumeration::make_or_throw(Type)");
    return out;
  }
  #endif

  explicit operator EnumT() const noexcept { return value_; }
  const EnumT &value() const noexcept { return value_; }
  const EnumT* value_ptr() const noexcept { return &value_; }

  bool operator==(const Enumeration &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const Enumeration &o) const noexcept { return !(*this == o); }

  bool operator==(const EnumT o) const noexcept { return value_ == o; }
  bool operator!=(const EnumT o) const noexcept { return value_ != o; }

  SUB8_NO_DISCARD BitFieldResult set_value(InitType v) noexcept {
    const StorageType code = to_storage(v);
    if (code < MinCode || code > MaxCode)
      return BitFieldResult::ErrorValueOverflow;
    value_ = v;
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult set_value(StorageType code) noexcept {
    if (code < MinCode || code > MaxCode)
      return BitFieldResult::ErrorValueOverflow;
    value_ = from_storage(code);
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value_unchecked(InitType v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }

private:
  EnumT value_{MinValue};

};

template <typename Storage, typename EnumT, EnumT MinV, EnumT MaxV> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Enumeration<EnumT, MinV, MaxV> &f) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  using F = Enumeration<EnumT, MinV, MaxV>;
  using ST = typename F::StorageType;

  // Dense encoding: code = value - MinCode
  const ST raw = F::to_storage(f.value());
  if (raw < F::MinCode || raw > F::MaxCode)
    return BitFieldResult::ErrorValueOverflow;

  const ST code = raw - F::MinCode;

  if (code >= F::RangeSize)
    return BitFieldResult::ErrorValueOverflow;

  return bw.template put_bits<ST>(code, F::ActualSize.bit_size());
}

template <typename Storage, typename EnumT, EnumT MinV, EnumT MaxV> SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, Enumeration<EnumT, MinV, MaxV> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = Enumeration<EnumT, MinV, MaxV>;
  using ST = typename F::StorageType;

  ST code = 0;
  auto r = br.template get_bits<ST>(code, F::ActualSize.bit_size());
  if (r != BitFieldResult::Ok) {
    return r;
  }

  // Must fall in [0..RangeSize-1]
  if (code >= F::RangeSize)
    return BitFieldResult::ErrorValueOverflow;

  const ST raw = code + F::MinCode;
  return out.set_value(F::from_storage(raw));
}

#endif // SUB8_ENABLE_ENUM_FIELDS

} // namespace sub8
