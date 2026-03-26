#pragma once
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
#include <limits>
#include <type_traits>

// Enable: Fixed Length Fields.
// Fields with a fixed predetermined length.
// All values are the same size on the wire, signed values are zig zag encoded
#ifndef SUB8_ENABLE_FIXED_LENGTH_FIELDS
#define SUB8_ENABLE_FIXED_LENGTH_FIELDS 1
#endif

// Enable: Variable Length Fields
// Fields encoded into groups for n bit length. Uses continuation bits to
// indicate more Smaller values more efficiently packed on the write. Larger
// values are less efficiently packed due to continuation bit overhead. Uses zig
// zag encoding to ensure negative numbers do not always consume all groups
#ifndef SUB8_ENABLE_VARIABLE_LENGTH_FIELDS
#define SUB8_ENABLE_VARIABLE_LENGTH_FIELDS 1
#endif

// Enable: Checks that arithmetic do not overflow,
// Will throw exceptions when arithmetic results in an overflow.
// When disabled, arithmetic overflow behavior is undefined but
// saves additional checks.
// Would recommend leaving enabled and use the add/sub/mul/div _unchecked(...)
// methods only when overflow is guaranteed not to occur
#ifndef SUB8_ENABLE_CHECKED_ARITHMETIC
#define SUB8_ENABLE_CHECKED_ARITHMETIC 1
#endif

// Enable predefined set of common data types
#ifndef SUB8_ENABLE_BOOL
#define SUB8_ENABLE_BOOL 1
#endif


// Primitive Field Types
// =======================

namespace sub8 {

// Foundation Field Types
// =======================

// null Field
// ------------
struct NullValue {
  using Type = void;
  using ValueType = void;
  using InitType = void;

  using IntegerType = void;
  using StorageType = void;

  static constexpr BitSize ActualSize = BitSize::from_bits(0);
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept { return ActualSize; }
  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }
};

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult write_field(BasicBitWriter<Storage> &, NullValue) noexcept {
  return BitFieldResult::ErrorCanNotWriteNullValue;
}

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &, NullValue &) noexcept {
  return BitFieldResult::ErrorCanNotReadNullValue;
}

// Bool Field
// ------------
#if SUB8_ENABLE_BOOL

struct Boolean {
  using Type = bool;
  using InitType = bool;
  using ValueType = bool;
  using StorageType = bool;

  static constexpr BitSize ActualSize = BitSize::from_bits(1);
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept { return ActualSize; }
  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  Boolean() noexcept = default;
  Boolean(const Boolean &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  Boolean(bool v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, Boolean, "sub8::Boolean(bool)");
  }
  #endif

  Boolean &operator=(const Boolean &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  Boolean &operator=(bool v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, Boolean, "sub8::Boolean &operator=");
    return *this;
  }
  #endif

  SUB8_NO_DISCARD static BitFieldResult make(bool v, Boolean &out) noexcept { return out.set_value(v); }

  #if !SUB8_ENABLE_INFALLIBLE
  static Boolean make_or_throw(bool v) SUB8_OPT_NO_EXCEPT {
    Boolean out{};
    auto r = out.set_value(v);
    SUB8_THROW_IF_ERROR(r, Boolean, "Boolean::make_or_throw(bool)");
    return out;
  }
  #endif

  BitFieldResult set_value(bool v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }

  explicit operator bool() const noexcept { return value_; }
  const bool &value() const noexcept { return value_; }
  const bool* value_ptr() const noexcept { return &value_; }

  bool operator==(const Boolean &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const Boolean &o) const noexcept { return !(*this == o); }
  friend bool operator==(bool b, const Boolean& x) noexcept { return x.value_ == b; }
  friend bool operator!=(bool b, const Boolean& x) noexcept { return x.value_ != b; }

private:
  bool value_{};
};

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const sub8::Boolean &field) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  return bw.put_bits(field.value() ? 1u : 0u, 1);
}

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br, sub8::Boolean &out) noexcept {
  size_t tmp = 0;
  auto r = br.get_bits(tmp, 1);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  return out.set_value((tmp & 1u) != 0u);
}

#endif // SUB8_ENABLE_BOOL

// Fixed Length Field
// ------------------
#if SUB8_ENABLE_FIXED_LENGTH_FIELDS

template <uint32_t TBitLength, bool TSigned = false> struct Integer {
  static_assert(TBitLength >= 1);

public:
  using Type = typename limits::numeric_for_bits<TBitLength, TSigned>::type;
  using ValueType = Type;
  using InitType = Type;

  using IntegerType = typename unpack_t::underlying_or_self<Type>::type;
  using StorageType = std::make_unsigned_t<IntegerType>;

  static constexpr size_t BitWidth = TBitLength;
  static constexpr BitSize ActualSize = BitSize::from_bits(TBitLength);
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept { return BitSize::from_bits(TBitLength); }
  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  using BitLimits = sub8::limits::bits_limits<Type, BitWidth>;
  static constexpr StorageType MaxCode = BitLimits::MaxCode;
  static constexpr Type MinValue = BitLimits::MinValue;
  static constexpr Type MaxValue = BitLimits::MaxValue;

  explicit operator Type() const noexcept { return value_; }
  const Type &value() const noexcept { return value_; }
  const Type* value_ptr() const noexcept { return &value_; }
  
  Integer() noexcept = default;
  Integer(const Integer &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  Integer(const Type &v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, Integer, "sub8::Integer(const Type &)");
  }
  #endif

  SUB8_NO_DISCARD static BitFieldResult make(Type v, Integer &out) noexcept { return out.set_value(v); }

  #if !SUB8_ENABLE_INFALLIBLE
  static Integer make_or_throw(Type v) SUB8_OPT_NO_EXCEPT {
    Integer out{};
    auto r = out.set_value(v);
    SUB8_THROW_IF_ERROR(r, Integer, "Integer::make_or_throw(Type)");
    return out;
  }
  #endif

  Integer &operator=(const Integer &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  Integer &operator=(Type v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, Integer, "sub8::Integer &operator=");
    return *this;
  }
  #endif

  bool operator==(const Integer &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const Integer &o) const noexcept { return !(*this == o); }

  SUB8_NO_DISCARD BitFieldResult set_value(InitType v) noexcept {
    const Type uv = static_cast<Type>(v);
    if (uv < MinValue || uv > MaxValue)
      return BitFieldResult::ErrorValueOverflow;

    value_ = v;
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value_unchecked(InitType v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult add(InitType v) noexcept {
    auto r = BitLimits::check_add(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) + static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult add_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) + static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  SUB8_NO_DISCARD BitFieldResult sub(InitType v) noexcept {
    auto r = BitLimits::check_sub(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) - static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult sub_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) - static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  SUB8_NO_DISCARD BitFieldResult mul(InitType v) noexcept {
    auto r = BitLimits::check_mul(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) * static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult mul_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) * static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  SUB8_NO_DISCARD BitFieldResult div(InitType v) noexcept {
    auto r = BitLimits::check_div(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) / static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult div_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) / static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

#if !SUB8_ENABLE_INFALLIBLE
  friend constexpr Integer operator+(Integer a, const Integer &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.add(b), Integer, "sub8::Integer operator+");
#else
    SUB8_THROW_IF_ERROR(a.add_unchecked(b), Integer, "sub8::Integer operator+")
#endif
    return a;
  }

  constexpr Integer &operator+=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->add(b), Integer, "sub8::Integer operator+=");
#else
    SUB8_THROW_IF_ERROR(this->add_unchecked(b), Integer, "sub8::Integer operator+=")
#endif
    return *this;
  }

  friend constexpr Integer operator-(Integer a, const Integer &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.sub(b), Integer, "sub8::Integer operator-");
#else
    SUB8_THROW_IF_ERROR(a.sub_unchecked(b), Integer, "sub8::Integer operator-")
#endif
    return a;
  }

  constexpr Integer &operator-=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->sub(b), Integer, "sub8::Integer operator-=");
#else
    SUB8_THROW_IF_ERROR(this->sub_unchecked(b), Integer, "sub8::Integer operator-=")
#endif
    return *this;
  }

  friend constexpr Integer operator*(Integer a, const Integer &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.mul(b), Integer, "sub8::Integer operator*");
#else
    SUB8_THROW_IF_ERROR(a.mul_unchecked(b), Integer, "sub8::Integer operator*")
#endif
    return a;
  }

  constexpr Integer &operator*=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->mul(b), Integer, "sub8::Integer operator*=");
#else
    SUB8_THROW_IF_ERROR(this->mul_unchecked(b), Integer, "sub8::Integer operator*=")
#endif
    return *this;
  }

  friend constexpr Integer operator/(Integer a, const Integer &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.div(b), Integer, "sub8::Integer operator/");
#else
    SUB8_THROW_IF_ERROR(a.div_unchecked(b), Integer, "sub8::Integer operator/")
#endif
    return a;
  }

  constexpr Integer &operator/=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->div(b), Integer, "sub8::Integer operator/=");
#else
    SUB8_THROW_IF_ERROR(this->div_unchecked(b), Integer, "sub8::Integer operator/=")
#endif
    return *this;
  }
#endif

private:
  Type value_{};
};

template <uint32_t TBitLength> using UnsignedInteger = Integer<TBitLength, /* signed */ false>;
template <uint32_t TBitLength> using SignedInteger = Integer<TBitLength, /* signed */ true>;

template <typename Storage, uint32_t TBitLength, bool TSigned> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Integer<TBitLength, TSigned> &f) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  using F = Integer<TBitLength, TSigned>;
  using StorageType = typename F::StorageType;

  StorageType v = sub8::packing::pack<typename F::Type>(f.value());
  if (v > F::MaxCode)
    return BitFieldResult::ErrorValueOverflow;

  return bw.template put_bits<StorageType>(v, F::ActualSize.bit_size());
}

template <typename Storage, uint32_t TBitLength, bool TSigned> SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br,
  Integer<TBitLength, TSigned> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = Integer<TBitLength, TSigned>;
  using StorageType = typename F::StorageType;

  StorageType tmp = 0;
  auto r = br.template get_bits<StorageType>(tmp, F::ActualSize.bit_size());
  if (r != BitFieldResult::Ok) {
    return r;
  }

  if (tmp > F::MaxCode) {
    return BitFieldResult::ErrorValueOverflow;
  }

  return out.set_value(sub8::packing::unpack<typename F::Type>(tmp));
}

#endif // SUB8_ENABLE_FIXED_LENGTH_FIELDS

// Variable Length Field
// ----------------------
#if SUB8_ENABLE_VARIABLE_LENGTH_FIELDS

template <bool TSigned, uint32_t... TSegments> struct VbrInteger {
public:
  static constexpr size_t MaxSegmentsCount = sizeof...(TSegments);
  static_assert(MaxSegmentsCount >= 2, "VbrInteger must have at least 2 groups");
  static_assert(((TSegments >= 1) && ...), "Each segment bit length must be >= 1");

  static constexpr size_t BitWidth = (size_t{0} + ... + size_t{TSegments});

  using Type = typename limits::numeric_for_bits<BitWidth, TSigned>::type;
  using ValueType = Type;
  using InitType = Type;

  using IntegerType = typename unpack_t::underlying_or_self<Type>::type;
  using StorageType = typename std::make_unsigned<IntegerType>::type;

  static constexpr BitSize MaxPossibleSize = []() constexpr noexcept -> BitSize {
    const uint32_t data_bits = BitWidth;
    const uint32_t cont_bits = MaxSegmentsCount - 1;
    return BitSize::from_bits(data_bits + cont_bits);
  }();

  static constexpr BitSize MinPossibleSize = []() constexpr noexcept -> BitSize {
    constexpr uint32_t segs[] = {TSegments...};
    return BitSize::from_bits(segs[0] + 1);
  }();

  BitSize actual_size() const noexcept {
    auto seg_count = calculate_segment_count(value_);
    if (seg_count == MaxSegmentsCount) {
      return MaxPossibleSize;
    }
    auto bit_length = calculate_bit_size_for_value(seg_count);
    return BitSize::from_bits(bit_length + seg_count);
  }

  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  using BitLimits = sub8::limits::bits_limits<Type, BitWidth>;
  static constexpr StorageType MaxCode = BitLimits::MaxCode;
  static constexpr Type MinValue = BitLimits::MinValue;
  static constexpr Type MaxValue = BitLimits::MaxValue;

public:

  VbrInteger() noexcept = default;
  VbrInteger(const VbrInteger &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  VbrInteger(const Type &v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, VbrInteger, "sub8::VbrInteger(const Type &)");
  }
  #endif

  SUB8_NO_DISCARD static BitFieldResult make(Type v, VbrInteger &out) noexcept { return out.set_value(v); }

  #if !SUB8_ENABLE_INFALLIBLE
  static Boolean make_or_throw(Type v) SUB8_OPT_NO_EXCEPT {
    Boolean out{};
    auto r = out.set_value(v);
    SUB8_THROW_IF_ERROR(r, VbrInteger, "VbrInteger::make_or_throw(Type)");
    return out;
  }
  #endif

  explicit operator Type() const noexcept { return value_; }
  const Type &value() const noexcept { return value_; }
  const Type* value_ptr() const noexcept { return &value_; }

  bool operator==(const VbrInteger &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const VbrInteger &o) const noexcept { return !(*this == o); }

  VbrInteger &operator=(const VbrInteger &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  VbrInteger &operator=(Type v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, VbrInteger, "sub8::VbrInteger::operator=(Type v)");
    return *this;
  }
  #endif

  SUB8_NO_DISCARD BitFieldResult set_value(InitType v) noexcept {
    const IntegerType uv = static_cast<IntegerType>(v);
    if (uv < MinValue || uv > MaxValue) {
      return BitFieldResult::ErrorValueOverflow;
    }
    value_ = v;
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value_unchecked(InitType v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult add(InitType v) noexcept {
    auto r = BitLimits::check_add(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) + static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult add_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) + static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  SUB8_NO_DISCARD BitFieldResult sub(InitType v) noexcept {
    auto r = BitLimits::check_sub(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) - static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult sub_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) - static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  SUB8_NO_DISCARD BitFieldResult mul(InitType v) noexcept {
    auto r = BitLimits::check_mul(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) * static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult mul_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) * static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  SUB8_NO_DISCARD BitFieldResult div(InitType v) noexcept {
    auto r = BitLimits::check_div(value_, v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    auto result = static_cast<typename BitLimits::Integral>(value_) / static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }

  BitFieldResult div_unchecked(InitType v) noexcept {
    auto result = static_cast<typename BitLimits::Integral>(value_) / static_cast<typename BitLimits::Integral>(v);
    return set_value(static_cast<Type>(result));
  }


    #if !SUB8_ENABLE_INFALLIBLE
  friend constexpr VbrInteger operator+(VbrInteger a, const VbrInteger &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.add(b), VbrInteger, "sub8::VbrInteger operator+");
#else
    SUB8_THROW_IF_ERROR(a.add_unchecked(b), VbrInteger, "sub8::VbrInteger operator+")
#endif
    return a;
  }

  constexpr VbrInteger &operator+=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->add(b), VbrInteger, "sub8::VbrInteger operator+=");
#else
    SUB8_THROW_IF_ERROR(this->add_unchecked(b), VbrInteger, "sub8::VbrInteger operator+=")
#endif
    return *this;
  }

  friend constexpr VbrInteger operator-(VbrInteger a, const VbrInteger &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.sub(b), VbrInteger, "sub8::VbrInteger operator-");
#else
    SUB8_THROW_IF_ERROR(a.sub_unchecked(b), VbrInteger, "sub8::VbrInteger operator-")
#endif
    return a;
  }

  constexpr VbrInteger &operator-=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->sub(b), VbrInteger, "sub8::VbrInteger operator-=");
#else
    SUB8_THROW_IF_ERROR(this->sub_unchecked(b), VbrInteger, "sub8::VbrInteger operator-=")
#endif
    return *this;
  }

  friend constexpr VbrInteger operator*(VbrInteger a, const VbrInteger &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.mul(b), VbrInteger, "sub8::VbrInteger operator*");
#else
    SUB8_THROW_IF_ERROR(a.mul_unchecked(b), VbrInteger, "sub8::VbrInteger operator*")
#endif
    return a;
  }

  constexpr VbrInteger &operator*=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->mul(b), VbrInteger, "sub8::VbrInteger operator*=");
#else
    SUB8_THROW_IF_ERROR(this->mul_unchecked(b), VbrInteger, "sub8::VbrInteger operator*=")
#endif
    return *this;
  }

  friend constexpr VbrInteger operator/(VbrInteger a, const VbrInteger &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(a.div(b), VbrInteger, "sub8::VbrInteger operator/");
#else
    SUB8_THROW_IF_ERROR(a.div_unchecked(b), VbrInteger, "sub8::VbrInteger operator/")
#endif
    return a;
  }

  constexpr VbrInteger &operator/=(const InitType &b) SUB8_OPT_NO_EXCEPT {
#if SUB8_ENABLE_CHECKED_ARITHMETIC
    SUB8_THROW_IF_ERROR(this->div(b), VbrInteger, "sub8::VbrInteger operator/=");
#else
    SUB8_THROW_IF_ERROR(this->div_unchecked(b), VbrInteger, "sub8::VbrInteger operator/=")
#endif
    return *this;
  }
  #endif

  static constexpr StorageType make_mask(uint32_t bits) noexcept {
    if (bits == 0) {
      return StorageType{0};
    }
    constexpr uint32_t W = sizeof(StorageType) * 8;
    if (bits >= W) {
      return std::numeric_limits<StorageType>::max();
    }
    return (StorageType{1} << bits) - StorageType{1};
  }

  static constexpr uint32_t segment_length(size_t i) noexcept {
    constexpr uint32_t segs[] = {TSegments...};
    return segs[i];
  }

  static constexpr uint32_t calculate_bit_size_for_value(uint32_t k) noexcept {
    uint32_t s = 0;
    for (uint32_t i = 0; i < k; ++i)
      s += segment_length(i);
    return s;
  }

  static inline size_t calculate_segment_count(const Type &value) noexcept {
    using F = VbrInteger;
    using U = typename F::StorageType;

    U v = packing::pack(value);
    if (v == 0) {
      return 1;
    }

    // Iterate over the segments to figure out
    // how many are required to hold this specific number
    uint32_t cap = 0;
    for (uint32_t k = 1; k < F::MaxSegmentsCount; ++k) {
      cap += F::segment_length(k - 1);
      if ((v >> cap) == 0) {
        return k;
      }
    }
    return F::MaxSegmentsCount;
  }

private:
  Type value_{};
};

template <uint32_t... TSegments> using UnsignedVbrInteger = VbrInteger</* signed */ false, TSegments...>;

template <uint32_t... TSegments> using SignedVbrInteger = VbrInteger</* signed */ true, TSegments...>;


template <typename Storage, bool TSigned, uint32_t... TSegments> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const VbrInteger<TSigned, TSegments...> &field) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  using F = VbrInteger<TSigned, TSegments...>;
  using U = typename F::StorageType;

  U v = packing::pack(field.value());
  if (v > F::MaxCode) {
    return BitFieldResult::ErrorValueOverflow;
  }
  // Canonical zero: one group, cont=0
  if (v == 0) {
    const uint32_t db = F::segment_length(0);
    return bw.template put_bits<U>(0, db + 1); // [0|0...]
  }

  const uint32_t k = F::calculate_segment_count(field.value());
  if (k < 1 || k > F::MaxSegmentsCount) {
    return BitFieldResult::ErrorTooManyFragments;
  }

  const uint32_t used_bits = F::calculate_bit_size_for_value(k);

  // Emit groups 0..k-2 as [1|chunk]
  uint32_t consumed = 0;
  for (uint32_t g = 0; g + 1 < k; ++g) {
    const uint32_t db = F::segment_length(g);
    const uint32_t remaining_after = used_bits - (consumed + db);

    const U chunk = (v >> remaining_after) & F::make_mask(db);
    const U group_bits = (U{1} << db) | chunk; // cont bit above the data

    auto r = bw.template put_bits<U>(group_bits, db + 1);
    if (r != BitFieldResult::Ok) {
      return r;
    }

    consumed += db;
  }

  // Last group
  const uint32_t last = k - 1;
  const uint32_t db_last = F::segment_length(last);
  const U last_chunk = v & F::make_mask(db_last);

  if (k == F::MaxSegmentsCount) {
    // Max groups => last group has NO cont bit
    return bw.template put_bits<U>(last_chunk, db_last);
  } else {
    // Early stop => [0|data]
    return bw.template put_bits<U>(last_chunk, db_last + 1);
  }
}

template <typename Storage, bool TSigned, uint32_t... TSegments> SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, VbrInteger<TSigned, TSegments...> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = VbrInteger<TSigned, TSegments...>;
  using U = typename F::StorageType;

  U accum = 0;

  for (size_t g = 0; g < F::MaxSegmentsCount; ++g) {
    const uint32_t db = F::segment_length(g);

    if (g + 1 < F::MaxSegmentsCount) {
      // [cont|data] (db + 1 bits)
      U group_bits = 0;
      auto r = br.template get_bits<U>(group_bits, db + 1);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      const U data = group_bits & F::make_mask(db);
      const bool cont = ((group_bits >> db) != 0);

      if (accum > (std::numeric_limits<U>::max() >> db)) {
        return BitFieldResult::ErrorValueOverflow;
      }
      accum = static_cast<U>((accum << db) | data);

      if (!cont) {
        break;
      }
    } else {
      // last possible group: data only
      U data = 0;
      auto r = br.template get_bits<U>(data, db);
      if (r != BitFieldResult::Ok)
        return r;

      if (accum > (std::numeric_limits<U>::max() >> db)) {
        return BitFieldResult::ErrorValueOverflow;
      }
      accum = static_cast<U>((accum << db) | (data & F::make_mask(db)));
      break;
    }
  }

  if (accum > F::MaxCode) {
    return BitFieldResult::ErrorValueOverflow;
  }
  return out.set_value(packing::unpack<typename F::Type>(accum));
}

#endif // SUB8_ENABLE_VARIABLE_LENGTH_FIELDS

} // namespace sub8
