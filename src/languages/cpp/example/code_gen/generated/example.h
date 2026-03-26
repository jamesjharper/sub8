#pragma once

// Auto-generated (stage3_emit)

#include <cstddef>
#include <cstdint>
#include <utility>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 1
#endif

#ifndef SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_ENABLE_NEVER_THROW_EXCEPTIONS 0
#endif

#ifndef SUB8_ENABLE_INFALLIBLE
#define SUB8_ENABLE_INFALLIBLE 0
#endif

#ifndef SUB8_ENABLE_NO_DISCARD_RESULTS
#define SUB8_ENABLE_NO_DISCARD_RESULTS 0
#endif

#ifndef SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#define SUB8_ENABLE_NO_MALLOC_EXCEPTIONS 0
#endif

#ifndef SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_STL_TYPE 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_VECTOR
#define SUB8_ENABLE_STL_TYPE_VECTOR 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_ARRAY
#define SUB8_ENABLE_STL_TYPE_ARRAY 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#define SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST 1
#endif

#ifndef SUB8_ENABLE_BASIC_STRING
#define SUB8_ENABLE_BASIC_STRING 1
#endif

#ifndef SUB8_ENABLE_OPTIONAL_FIELDS
#define SUB8_ENABLE_OPTIONAL_FIELDS 1
#endif

#ifndef SUB8_ENABLE_BOUNDED_BUF
#define SUB8_ENABLE_BOUNDED_BUF 1
#endif

#ifndef SUB8_ENABLE_UNBOUNDED_BUF
#define SUB8_ENABLE_UNBOUNDED_BUF 1
#endif

#ifndef SUB8_ENABLE_VIEW_BUF
#define SUB8_ENABLE_VIEW_BUF 1
#endif

#ifndef SUB8_ENABLE_CHECKED_ARITHMETIC
#define SUB8_ENABLE_CHECKED_ARITHMETIC 1
#endif


// ============================================================
// BEGIN INLINE ./sub8_errors.h
// ============================================================

#pragma once

// Enable: Prevents any memory allocation while throwing exceptions in exchange for loss of error context infomation
#ifndef SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#define SUB8_ENABLE_NO_MALLOC_EXCEPTIONS 0
#endif

// Enable: Prevents any throw exceptions durring assignment, ctor and arithmetic operators by replacing with
// runtime assertions. Allows "noexcept" semantics in exchange for UB operations.
// Note: add, sub, mul, div, set_value operators can be used instead instead to prevent UB.
#ifndef SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_ENABLE_NEVER_THROW_EXCEPTIONS 0
#endif

#if SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_OPT_NO_EXCEPT noexcept
#else
#define SUB8_OPT_NO_EXCEPT
#endif

// Enable: Disables access to any method capable of throwing an exception or asserting an error 
#if SUB8_ENABLE_INFALLIBLE
#define SUB8_ENABLE_INFALLIBLE 0
#endif

// Enable: Prevents checked methods for enforcing [[nodiscard]]. Would recommend leaving on, and using *_uncheck(...) and *_or_throw(..)
#ifndef SUB8_ENABLE_NO_DISCARD_RESULTS
#define SUB8_ENABLE_NO_DISCARD_RESULTS 1
#endif

#if SUB8_ENABLE_NO_DISCARD_RESULTS
#define SUB8_NO_DISCARD [[nodiscard]]
#else
#define SUB8_NO_DISCARD
#endif

#if !SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#include <string>
#endif

namespace sub8 {

// Error Types
enum class BitFieldResult : uint8_t {
  Ok = 0,
  WarningNotSupportedConfiguration,
  ErrorInvalidBitFieldValue,
  ErrorTooManyElements,
  ErrorTooManyFragments,
  ErrorCanNotBeEmpty,
  ErrorValueOverflow,
  ErrorExpectedMoreBits,
  ErrorInsufficentBufferSize,
  ErrorCanNotWriteNullValue,
  ErrorCanNotReadNullValue,
  ErrorTooManyBits,
  // IO Errors
  ErrorBadAlloc,
  ErrorOversizedLength,

  ErrorOptionalValueIsEmpty,

  ErrorUnidentifiedError,

};

namespace type_info {
namespace _no_rtti {
// Hack to extract type name info without RTTI at compile time
constexpr std::string_view extract_type_name(std::string_view p) noexcept {
#if defined(__clang__)
  auto start = p.find("T = ") + 4;
  auto end = p.find(']', start);
  return p.substr(start, end - start);
#elif defined(__GNUC__)
  auto start = p.find("T = ") + 4;
  auto end = p.find(';', start);
  return p.substr(start, end - start);
#elif defined(_MSC_VER)
  auto start = p.find("name_sv<") + 8;
  auto end = p.find(">(void)", start);
  return p.substr(start, end - start);
#else
#if defined(__GXX_RTTI) || defined(_CPPRTTI)
  return typeid(T).name();
#else
  return "type";
#endif
#endif
}

template <class T> constexpr std::string_view inferred_type_name() noexcept {
#if defined(__clang__) || defined(__GNUC__)
  return _no_rtti::extract_type_name(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
  return _no_rtti::extract_type_name(__FUNCSIG__);
#else
  return "type";
#endif
}

} // namespace _no_rtti

template <typename T> inline std::string_view name() noexcept {
  if constexpr (std::is_polymorphic_v<T>) {
#if defined(__GXX_RTTI) || defined(_CPPRTTI)
    return typeid(T).name();
#else
    return _no_rtti::inferred_type_name<T>();
#endif
  } else {
    return _no_rtti::inferred_type_name<T>();
  }
}
} // namespace type_info

namespace error {

inline const char *to_string(BitFieldResult r) {
  switch (r) {
  case BitFieldResult::Ok:
    return "Ok";
  case BitFieldResult::WarningNotSupportedConfiguration:
    return "WarningNotSupportedConfiguration";
  case BitFieldResult::ErrorInvalidBitFieldValue:
    return "ErrorInvalidBitFieldValue";
  case BitFieldResult::ErrorTooManyElements:
    return "Value has more elements then the field value can hold";
  case BitFieldResult::ErrorTooManyFragments:
    return "Value has more fragments then the field value expected";
  case BitFieldResult::ErrorCanNotBeEmpty:
    return "Value can not be empty.";
  case BitFieldResult::ErrorValueOverflow:
    return "Value is larger or smaller then the field value can hold";
  case BitFieldResult::ErrorExpectedMoreBits:
    return "Cant read value from as the buffer has less bytes available then expected";
  case BitFieldResult::ErrorTooManyBits:
    return "Cant read/write value to buffer as it has more bits the the "
           "provided storage type.";
  case BitFieldResult::ErrorInsufficentBufferSize:
    return "Can write to buffer as there is insufficent space available";
  case BitFieldResult::ErrorCanNotWriteNullValue:
    return "Can not write type \"NullValue\" to stream";
  case BitFieldResult::ErrorCanNotReadNullValue:
    return "Can not read type \"NullValue\" to stream";
  case BitFieldResult::ErrorBadAlloc:
    return "Allocation failed";
  case BitFieldResult::ErrorOversizedLength:
    return "Requested size exceeds max_size()";

  case BitFieldResult::ErrorOptionalValueIsEmpty:
    return "Can not unwrap value as optional value is empty";

  case BitFieldResult::ErrorUnidentifiedError:
    return "STL subsystem threw an unexpected exception type";
  }
  return "UnknownBitFieldResult";
}

#if SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE) assert(R == BitFieldResult::Ok && WHERE);

#else

#if SUB8_ENABLE_NO_MALLOC_EXCEPTIONS

// Heap-free exceptions
struct bitfield_static_error final : std::exception {
  BitFieldResult code;
  const char *where_;
  std::string_view field_type_;

  bitfield_static_error(BitFieldResult r, const char *where, std::string_view field_type)
      : std::exception(), code(r), where_(where), field_type_(field_type) {}

  const char *what() const noexcept override { return to_string(code); }
};

using bitfield_error = bitfield_static_error;

#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE)                                                                                    \
  throw ::sub8::error::bitfield_static_error((R), (WHERE), sub8::type_info::name<TFIELDTYPE>())

#else
struct bitfield_dynamic_error final : std::runtime_error {
  BitFieldResult code;
  std::string_view field_type_;

  bitfield_dynamic_error(BitFieldResult r, const char *where, std::string_view field_type)
      : std::runtime_error(std::string(where) + " where T = " + std::string(field_type) + " failed with error \"" + to_string(r) + "\""),
        code(r), field_type_(field_type) {}

  constexpr const char *code_cstr() const noexcept { return to_string(code); }
};

using bitfield_error = bitfield_dynamic_error;

#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE)                                                                                    \
  throw ::sub8::error::bitfield_dynamic_error((R), (WHERE), sub8::type_info::name<TFIELDTYPE>())

#endif
#endif

#define SUB8_THROW_IF_ERROR(R, TFIELDTYPE, WHERE)                                                                                          \
  {                                                                                                                                        \
    BitFieldResult __r = R;                                                                                                                \
    if (__r != BitFieldResult::Ok) {                                                                                                       \
      SUB8_THROW_BITFIELD_ERROR(__r, TFIELDTYPE, WHERE);                                                                                   \
    }                                                                                                                                      \
  }

inline BitFieldResult map_std_exception_to_bitfield_result() noexcept {
  try {
    throw; // rethrow active exception
  } catch (const std::bad_alloc &) {
    return BitFieldResult::ErrorBadAlloc;
  } catch (const std::length_error &) {
    return BitFieldResult::ErrorOversizedLength;
  } catch (...) {
    return BitFieldResult::ErrorUnidentifiedError;
  }
}

} // namespace error
} // namespace sub8
// ============================================================
// END INLINE ./sub8_errors.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_type_information.h
// ============================================================

#pragma once

#include <limits>
#include <string_view>
#include <type_traits>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_api.h"
#endif

namespace sub8 {

// Template helpers for resolving the inner type of an enum
namespace unpack_t {
template <typename T, bool IsEnum = std::is_enum<T>::value> struct underlying_or_self {
  using type = T;
};

template <typename T> struct underlying_or_self<T, true> {
  using type = typename std::underlying_type<T>::type;
};
} // namespace unpack_t

namespace details_t {

template <typename...> using void_t = void;

template <typename T, typename = void> struct has_actual_size : std::false_type {};

template <typename T> struct has_actual_size<T, void_t<decltype(T::ActualSize)>> : std::true_type {};

template <typename T> inline constexpr bool has_actual_size_v = has_actual_size<T>::value;

} // namespace details_t

namespace limits {

template <typename T> using underlying_or_self_t = typename unpack_t::underlying_or_self<T>::type;

template <typename T, uint32_t Bits> struct bits_limits {
  using Integral = underlying_or_self_t<T>;
  using UnsignedIntegral = std::make_unsigned_t<Integral>;
  using SignedIntegral = std::make_signed_t<UnsignedIntegral>;

  static constexpr Integral as_integral(T v) noexcept {
    if constexpr (std::is_enum_v<T>) {
      return static_cast<Integral>(v);
    } else {
      return v;
    }
  }

  static constexpr T as_t(Integral v) noexcept {
    if constexpr (std::is_enum_v<T>) {
      return static_cast<T>(v);
    } else {
      return v;
    }
  }

  static_assert(Bits >= 1, "Bits must be >= 1");
  static_assert(std::is_integral_v<Integral> || std::is_enum_v<T>, "bits_limits requires integral (or enum) types");

  static constexpr uint32_t BitsWidth = Bits;
  static constexpr uint32_t StorageTypeWidth = sizeof(UnsignedIntegral) * 8;

  // Max code representable in Bits
  static constexpr UnsignedIntegral MaxCode = (BitsWidth >= StorageTypeWidth) ? std::numeric_limits<UnsignedIntegral>::max()
                                                                              : (UnsignedIntegral{1} << BitsWidth) - UnsignedIntegral{1};

  static constexpr T MinValue = []() constexpr noexcept -> T {
    if constexpr (std::is_signed_v<Integral>) {
      if constexpr (BitsWidth >= StorageTypeWidth) {
        return static_cast<T>(std::numeric_limits<Integral>::min());
      } else {
        const UnsignedIntegral mag = (UnsignedIntegral{1} << (BitsWidth - 1));
        return static_cast<T>(-static_cast<Integral>(mag));
      }
    } else {
      return static_cast<T>(0);
    }
  }();

  static constexpr T MaxValue = []() constexpr noexcept -> T {
    if constexpr (std::is_signed_v<Integral>) {
      if constexpr (BitsWidth >= StorageTypeWidth) {
        return static_cast<T>(std::numeric_limits<Integral>::max());
      } else {
        const UnsignedIntegral mag = (UnsignedIntegral{1} << (BitsWidth - 1));
        return static_cast<T>(static_cast<Integral>(mag - 1));
      }
    } else {
      if constexpr (BitsWidth >= StorageTypeWidth) {
        return static_cast<T>(std::numeric_limits<Integral>::max());
      } else {
        return static_cast<T>(static_cast<Integral>(MaxCode));
      }
    }
  }();

  static constexpr Integral MinIntegralValue = as_integral(MinValue);
  static constexpr Integral MaxIntegralValue = as_integral(MaxValue);

  static constexpr BitFieldResult check_add(T a, T b) noexcept {
    const Integral ua = as_integral(a);
    const Integral ub = as_integral(b);

    if constexpr (std::is_signed_v<Integral>) {
      // a + b overflow checks
      if (ub > 0) {
        if (ua > MaxIntegralValue - ub) {
          return BitFieldResult::ErrorValueOverflow;
        }
      } else if (ub < 0) {
        if (ua < MinIntegralValue - ub) {
          return BitFieldResult::ErrorValueOverflow;
        }
      }
      return BitFieldResult::Ok;
    } else {
      // unsigned: wrap guard
      if (ua > MaxIntegralValue - ub) {
        return BitFieldResult::ErrorValueOverflow;
      }
      return BitFieldResult::Ok;
    }
  }

  static constexpr BitFieldResult check_sub(T a, T b) noexcept {
    const Integral ua = as_integral(a);
    const Integral ub = as_integral(b);

    if constexpr (std::is_signed_v<Integral>) {
      // a - b overflow checks (done without negating b)
      if (ub > 0) {
        if (ua < MinIntegralValue + ub) {
          return BitFieldResult::ErrorValueOverflow;
        }
      } else if (ub < 0) {
        if (ua > MaxIntegralValue + ub) {
          return BitFieldResult::ErrorValueOverflow; // ub is negative
        }
      }
      return BitFieldResult::Ok;
    } else {
      // unsigned underflow guard
      if (ua < ub) {
        return BitFieldResult::ErrorValueOverflow;
      }
      return BitFieldResult::Ok;
    }
  }

  static constexpr BitFieldResult check_mul(T a, T b) noexcept {
    const Integral ua = as_integral(a);
    const Integral ub = as_integral(b);

    if constexpr (std::is_signed_v<Integral>) {

      if (ua == 0 || ub == 0) {
        return BitFieldResult::Ok;
      }

      // Guard classic two's-complement overflow: min * -1
      if (ua == MinIntegralValue && ub == Integral{-1}) {
        return BitFieldResult::ErrorValueOverflow;
      }
      if (ub == MinIntegralValue && ua == Integral{-1}) {
        return BitFieldResult::ErrorValueOverflow;
      }

      // Use division-based bounds checks to avoid overflow
      if (ua > 0) {
        if (ub > 0) {
          if (ua > MaxIntegralValue / ub) {
            return BitFieldResult::ErrorValueOverflow;
          }
        } else { // ub < 0
          if (ub < MinIntegralValue / ua) {
            return BitFieldResult::ErrorValueOverflow;
          }
        }
      } else { // ua < 0
        if (ub > 0) {
          if (ua < MinIntegralValue / ub) {
            return BitFieldResult::ErrorValueOverflow;
          }
        } else { // ub < 0 => result positive
          // max_under()/ub is <= 0 because ub<0; compare against ua (negative)
          if (ua < MaxIntegralValue / ub) {
            return BitFieldResult::ErrorValueOverflow;
          }
        }
      }
      return BitFieldResult::Ok;
    } else {
      // unsigned
      if (ub != 0) {
        if (ua > MaxIntegralValue / ub) {
          return BitFieldResult::ErrorValueOverflow;
        }
      }
      return BitFieldResult::Ok;
    }
  }

  static constexpr BitFieldResult check_div(T a, T b) noexcept {
    const Integral ua = as_integral(a);
    const Integral ub = as_integral(b);

    // divide-by-zero
    if (ub == 0) {
      return BitFieldResult::ErrorInvalidBitFieldValue;
    }

    if constexpr (std::is_signed_v<Integral>) {
      // Guard classic overflow: min / -1
      if (ua == MinIntegralValue && ub == Integral{-1}) {
        return BitFieldResult::ErrorValueOverflow;
      }
    }
    return BitFieldResult::Ok;
  }
};

// bits to encode values 0..max_value (max_value inclusive). Returns 0 for
// max_value==0 if you want that, or return at least 1 depending on your field
// rules.
constexpr uint32_t bitwidth_to_express_max_value(size_t max_value) noexcept {
  uint32_t bits = 0;
  while (max_value > 0) {
    max_value >>= 1;
    ++bits;
  }
  return bits;
}

// bits to encode `count` distinct values. count=0 is invalid; count=1 -> 0
// bits.
constexpr uint32_t bitwidth_to_express_distinct_values(size_t count) noexcept {
  if (count <= 1)
    return 0;
  return bitwidth_to_express_max_value(count - 1);
}

template <int Bits> struct uint_for_bits {
  using type = std::conditional_t<(Bits <= 8), uint8_t,
    std::conditional_t<(Bits <= 16), uint16_t, std::conditional_t<(Bits <= 32), uint32_t, uint64_t>>>;
};

template <int Bits> struct int_for_bits {
  using type =
    std::conditional_t<(Bits <= 8), int8_t, std::conditional_t<(Bits <= 16), int16_t, std::conditional_t<(Bits <= 32), int32_t, int64_t>>>;
};

template <int Bits, bool Signed> struct numeric_for_bits {
  using type = std::conditional_t<Signed, typename int_for_bits<Bits>::type, typename uint_for_bits<Bits>::type>;
};

template <int Bits> struct float_for_bits {
  using type = std::conditional_t<(Bits <= 32), float, double>;
};

} // namespace limits

} // namespace sub8

// ============================================================
// END INLINE ./sub8_type_information.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_io.h
// ============================================================

#pragma once
#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_errors.h"
#include "sub8_type_information.h"
#endif

#include <cassert>

// Feature flags":
// ===============

// Supported read/write buffer types
// ---------------------------------

// Enable: Support for buffers with known compile time size
#ifndef SUB8_ENABLE_BOUNDED_BUF
#define SUB8_ENABLE_BOUNDED_BUF 1
#endif

// Enable: Support heap based buffers (uses vectors internally)
#ifndef SUB8_ENABLE_UNBOUNDED_BUF
#define SUB8_ENABLE_UNBOUNDED_BUF 1
#endif

// Enable: Support pointer based buffers.
#ifndef SUB8_ENABLE_VIEW_BUF
#define SUB8_ENABLE_VIEW_BUF 1
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
#include <vector>
#endif

namespace sub8 {

namespace packing {

template <typename T> using underlying_or_self_t = typename unpack_t::underlying_or_self<T>::type;

template <typename T> using packed_t = std::make_unsigned_t<underlying_or_self_t<T>>;

template <typename S> using make_unsigned_t = typename std::make_unsigned<S>::type;

template <typename S> constexpr make_unsigned_t<S> zigzag_encode(S x) noexcept {
  static_assert(std::is_signed<S>::value, "zigzag::encode expects signed type");
  using U = make_unsigned_t<S>;
  return (static_cast<U>(x) << 1) ^ static_cast<U>(x >> (std::numeric_limits<S>::digits));
}

template <typename S> constexpr S zigzag_decode(make_unsigned_t<S> u) noexcept {
  static_assert(std::is_signed<S>::value, "zigzag::decode expects signed type");
  using U = make_unsigned_t<S>;
  return static_cast<S>((u >> 1) ^ static_cast<U>(-static_cast<typename std::make_signed<U>::type>(u & 1)));
}

template <typename T> constexpr packed_t<T> pack(T v) noexcept {
  using U = underlying_or_self_t<T>;
  using P = packed_t<T>;

  if constexpr (std::is_signed_v<U>) {
    return static_cast<P>(zigzag_encode<U>(static_cast<U>(v)));
  } else {
    return static_cast<P>(static_cast<U>(v));
  }
}

template <typename T> constexpr T unpack(packed_t<T> code) noexcept {
  using U = underlying_or_self_t<T>;

  if constexpr (std::is_signed_v<U>) {
    const U v = zigzag_decode<U>(code);
    return static_cast<T>(v);
  } else {
    return static_cast<T>(static_cast<U>(code));
  }
}

} // namespace packing

// Bit buffers
// =======================

// Object for addressing bit buffer lengths and position
class BitSize {
  size_t byte_size_ = 0;      // whole bytes
  uint8_t bit_remainder_ = 0; // 0..7 extra bits beyond whole bytes

  // Keeps invariant: bit_remainder_ in [0,7] by carrying into bytes.
  constexpr void normalize() noexcept {
    if (bit_remainder_ >= 8) {
      byte_size_ += static_cast<size_t>(bit_remainder_ / 8);
      bit_remainder_ = static_cast<uint8_t>(bit_remainder_ % 8);
    }
  }

public:
  constexpr BitSize() noexcept = default;

  constexpr BitSize(size_t byte_size, uint8_t bit_remainder) noexcept : byte_size_{byte_size}, bit_remainder_{bit_remainder} {
    normalize();
  }

  static constexpr BitSize from_bytes(size_t bytes) noexcept { return BitSize{bytes, 0}; }

  static constexpr BitSize from_bits(uint64_t bits) noexcept {
    return BitSize{static_cast<size_t>(bits / 8), static_cast<uint8_t>(bits % 8)};
  }

  // Accessors
  constexpr uint8_t bit_remainder() const noexcept { return bit_remainder_; }
  constexpr size_t byte_size_round_down() const noexcept { return byte_size_; }

  // Bytes required to store this many bits (round up to next byte if partial)
  constexpr size_t byte_size_round_up() const noexcept { return byte_size_ + (bit_remainder_ ? 1u : 0u); }

  // Generally avoid using bit_size() and lbit_size().
  // Will overflow with very large values.
  constexpr size_t bit_size() const noexcept { return (byte_size_ * size_t{8}) + size_t{bit_remainder_}; }

  constexpr uint64_t lbit_size() const noexcept {
    return (static_cast<uint64_t>(byte_size_) * 8ULL) + static_cast<uint64_t>(bit_remainder_);
  }

  constexpr BitSize &add_bits(uint64_t bits) noexcept {
    *this += from_bits(bits);
    return *this;
  }

  constexpr BitSize &sub_bits(uint64_t bits) noexcept {
    *this -= from_bits(bits);
    return *this;
  }

  // Comparisons
  friend constexpr bool operator==(const BitSize &a, const BitSize &b) noexcept {
    return a.byte_size_ == b.byte_size_ && a.bit_remainder_ == b.bit_remainder_;
  }
  friend constexpr bool operator!=(const BitSize &a, const BitSize &b) noexcept { return !(a == b); }

  friend constexpr bool operator<(const BitSize &a, const BitSize &b) noexcept {
    return (a.byte_size_ < b.byte_size_) || (a.byte_size_ == b.byte_size_ && a.bit_remainder_ < b.bit_remainder_);
  }
  friend constexpr bool operator>(const BitSize &a, const BitSize &b) noexcept { return b < a; }
  friend constexpr bool operator<=(const BitSize &a, const BitSize &b) noexcept { return !(b < a); }
  friend constexpr bool operator>=(const BitSize &a, const BitSize &b) noexcept { return !(a < b); }

  // Arithmetic (BitSize +/- BitSize)
  friend constexpr BitSize operator+(BitSize a, const BitSize &b) noexcept {
    a += b;
    return a;
  }

  constexpr BitSize &operator+=(const BitSize &other) noexcept {
    byte_size_ += other.byte_size_;
    bit_remainder_ = static_cast<uint8_t>(bit_remainder_ + other.bit_remainder_);
    normalize();
    return *this;
  }

  friend constexpr BitSize operator-(BitSize a, const BitSize &b) noexcept {
    a -= b;
    return a;
  }

  constexpr BitSize &operator-=(const BitSize &other) noexcept {
    // Require no underflow
    assert(*this >= other && "BitSize underflow");
    if (bit_remainder_ < other.bit_remainder_) {
      // borrow 1 byte => +8 bits
      assert(byte_size_ > other.byte_size_ && "BitSize underflow (borrow)");
      byte_size_ -= 1;
      bit_remainder_ = static_cast<uint8_t>(bit_remainder_ + 8);
    }
    bit_remainder_ = static_cast<uint8_t>(bit_remainder_ - other.bit_remainder_);
    byte_size_ -= other.byte_size_;
    return *this;
  }

  friend constexpr BitSize operator*(BitSize a, const size_t b) noexcept {
    a *= b;
    return a;
  }

  constexpr BitSize &operator*=(const size_t b) noexcept {
    const size_t bits = static_cast<size_t>(bit_remainder_) * b;
    byte_size_ = byte_size_ * b + (bits / 8u);
    bit_remainder_ = static_cast<uint8_t>(bits % 8u);
    return *this;
  }
};

template <typename Storage> 
class BasicBitWriter {
  Storage buf_;
  uint8_t sub_byte_pos_ = 0;

public:
  Storage &storage() noexcept { return buf_; }
  const Storage &storage() const noexcept { return buf_; }

  BitSize size() const noexcept {
    auto buf_size = buf_.size();
    if (sub_byte_pos_ == 0 || buf_size == 0) {
      return BitSize(buf_size, sub_byte_pos_);
    }
    return BitSize(buf_size - 1, sub_byte_pos_);
  }

  BitFieldResult put_padding(uint8_t nbits) noexcept {
    if (!nbits) {
      return BitFieldResult::Ok;
    }

    uint8_t remaining = nbits;
    while (remaining >= 8) {
      auto r = put_bits(uint8_t{0}, uint8_t{8});
      if (r != BitFieldResult::Ok) {
        return r;
      }
      remaining -= 8;
    }
    if (remaining) {
      return put_bits(uint8_t{0}, remaining);
    }
    return BitFieldResult::Ok;
  }

  template <typename T> BitFieldResult put_bits(T value, uint8_t nbits) noexcept {
    using U = typename unpack_t::underlying_or_self<T>::type;
    static_assert(std::is_unsigned<U>::value, "BitWriter::put_bits expects unsigned values");

    if (nbits == 0) {
      return BitFieldResult::Ok;
    }

    const U v = static_cast<U>(value);
    if (nbits <= 8) {
      return put_bits(static_cast<uint8_t>(v), static_cast<uint8_t>(nbits));
    }

    if (nbits > sizeof(U) * 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    size_t remaining = nbits;
    while (remaining) {
      const uint8_t take = static_cast<uint8_t>(remaining >= 8 ? 8 : remaining);
      const uint8_t shift = static_cast<uint8_t>(remaining - take);
      const uint8_t chunk = static_cast<uint8_t>((v >> shift) & ((U(1) << take) - 1u));

      auto r = put_bits(chunk, take);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      remaining -= take;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult put_bits(uint8_t v, uint8_t nbits) noexcept {
    if (nbits == 0) {
      return BitFieldResult::Ok;
    }

    if (nbits > 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    if (nbits < 8) {
      v &= static_cast<uint8_t>((static_cast<uint8_t>(1u) << nbits) - 1u);
    }

    uint8_t pos = sub_byte_pos_;

    while (nbits) {
      if (pos == 0) {
        auto r = buf_.push_back(0);
        if (r != BitFieldResult::Ok) {
          return r;
        }
      }

      const uint8_t remaining_space = static_cast<uint8_t>(8u - pos);
      const uint8_t take = (nbits < remaining_space ? nbits : remaining_space);

      const uint8_t shift = static_cast<uint8_t>(nbits - take);
      const uint8_t chunk = static_cast<uint8_t>((v >> shift) & ((static_cast<uint8_t>(1u) << take) - 1u));

      uint8_t *dst = nullptr;
      auto br = buf_.back_mut_ptr(dst);

      if (br != BitFieldResult::Ok) 
        return br;

      if (dst == nullptr) 
      return BitFieldResult::ErrorUnidentifiedError;

      *dst |= static_cast<uint8_t>(chunk << (remaining_space - take));

      pos = static_cast<uint8_t>(pos + take);
      nbits = static_cast<uint8_t>(nbits - take);

      if (pos == 8)
        pos = 0;
    }

    sub_byte_pos_ = pos;
    return BitFieldResult::Ok;
  }
};

template <typename Storage>
class BasicBitReader {
  Storage *buf_ = nullptr;

  // Inline owned storage for the rvalue case.
  using OwnedStorage = typename std::aligned_storage<sizeof(Storage), alignof(Storage)>::type;
  bool owns_ = false;
  OwnedStorage owned_{};

  BitSize total_size_{};
  size_t idx_ = 0;
  uint8_t sub_byte_pos_ = 0;

  Storage *owned_ptr() noexcept {
#if __cplusplus >= 201703L
    return std::launder(reinterpret_cast<Storage *>(&owned_));
#else
    return reinterpret_cast<Storage *>(&owned_);
#endif
  }
  const Storage *owned_ptr() const noexcept {
#if __cplusplus >= 201703L
    return std::launder(reinterpret_cast<const Storage *>(&owned_));
#else
    return reinterpret_cast<const Storage *>(&owned_);
#endif
  }

public:
  // Borrowing ctor (no copies)
  explicit BasicBitReader(Storage &data) noexcept : buf_(&data), total_size_(buf_->size()) {}

  // Borrowing ctor with explicit bit_size
  BasicBitReader(Storage &data, BitSize bit_size) noexcept : buf_(&data), total_size_(bit_size) {
    assert(total_size_.byte_size_round_up() <= buf_->size());
  }

  // Owning ctor (stores Storage by value inside this reader)
  explicit BasicBitReader(Storage &&data) noexcept(std::is_nothrow_move_constructible_v<Storage>)
      : buf_(nullptr), owns_(true) {
    ::new (&owned_) Storage(std::move(data));
    buf_ = owned_ptr();
    total_size_ = BitSize::from_bytes(buf_->size());
  }

  // Owning ctor with explicit bit_size
  BasicBitReader(Storage &&data, BitSize bit_size) noexcept(std::is_nothrow_move_constructible_v<Storage>)
      : buf_(nullptr), owns_(true), total_size_(bit_size) {
    ::new (&owned_) Storage(std::move(data));
    buf_ = owned_ptr();
    assert(total_size_.byte_size_round_up() <= buf_->size());
  }

  // No copying (safe and simple)
  BasicBitReader(const BasicBitReader &) = delete;
  BasicBitReader &operator=(const BasicBitReader &) = delete;

  // Move support
  BasicBitReader(BasicBitReader &&other) noexcept(std::is_nothrow_move_constructible_v<Storage>) {
    *this = std::move(other);
  }

  BasicBitReader &operator=(BasicBitReader &&other) noexcept(std::is_nothrow_move_constructible_v<Storage>) {
    if (this == &other) {
      return *this;
    }

    // Clean up existing owned storage
    if (owns_) {
      owned_ptr()->~Storage();
      owns_ = false;
      buf_ = nullptr;
    }


    
    total_size_ = other.total_size_;
    idx_ = other.idx_;
    sub_byte_pos_ = other.sub_byte_pos_;

    if (other.owns_) {
      // Move-construct our owned Storage from theirs
      ::new (&owned_) Storage(std::move(*other.owned_ptr()));
      buf_ = owned_ptr();
      owns_ = true;

      // Destroy other's owned storage and mark as non-owning
      other.owned_ptr()->~Storage();
      other.owns_ = false;
      other.buf_ = nullptr;

    } else {
      buf_ = other.buf_;
      owns_ = false;
      // Make the "other" buffer inert
      other.buf_ = nullptr;
      other.total_size_ = BitSize{};
    }
    other.idx_ = 0;
    other.sub_byte_pos_ = 0;

    return *this;
  }

  ~BasicBitReader() noexcept {
    if (owns_) {
      owned_ptr()->~Storage();
    }
  }

  Storage &storage() noexcept { return *buf_; }
  const Storage &storage() const noexcept { return *buf_; }
  const BitSize &size() const noexcept { return total_size_; }
  BitSize cursor_position() const noexcept { return BitSize(idx_, sub_byte_pos_); }

  void set_cursor_position(BitSize pos) noexcept {
    idx_ = pos.byte_size_round_down();
    sub_byte_pos_ = pos.bit_remainder();
  }

 bool has(size_t bits) const noexcept {
    size_t has_bytes = (bits >> 3) + idx_;
    uint8_t has_bits = static_cast<uint8_t>(bits & 7) + sub_byte_pos_;

    if (has_bits >= 8) {
      has_bits -= 8;
      has_bytes++;
    }

    return has_bytes == total_size_.byte_size_round_down() ? has_bits <= total_size_.bit_remainder()
                                                           : has_bytes <= total_size_.byte_size_round_down();
  }

  template <typename T> BitFieldResult get_bits(T &out, size_t nbits) noexcept {
    using U = typename unpack_t::underlying_or_self<T>::type;

    if (nbits == 0) {
      out = T(0);
      return BitFieldResult::Ok;
    }

    if (nbits > sizeof(U) * 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    if (!has(nbits)) {
      return BitFieldResult::ErrorExpectedMoreBits;
    }

    U acc = 0;
    size_t rem = nbits;

    while (rem) {
      uint8_t take = uint8_t(rem > 8 ? 8 : rem);
      uint8_t chunk = 0;
      auto r = get_bits_unchecked(chunk, take);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      acc = U((acc << take) | chunk);
      rem -= take;
    }

    out = static_cast<T>(acc);
    return BitFieldResult::Ok;
  }

  BitFieldResult get_bits(uint8_t &out, size_t nbits) noexcept {
    if (nbits == 0) {
      out = 0;
      return BitFieldResult::Ok;
    }

    if (nbits > 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    if (!has(nbits)) {
      return BitFieldResult::ErrorExpectedMoreBits;
    }

    return get_bits_unchecked(out, nbits);
  }

private:
  BitFieldResult get_bits_unchecked(uint8_t &out, size_t nbits) noexcept {
    uint8_t v = 0;
    uint8_t need = static_cast<uint8_t>(nbits);
    const size_t max_bytes = total_size_.byte_size_round_up();

    size_t idx = idx_;
    uint8_t pos = sub_byte_pos_;

    while (need) {
      if (idx >= max_bytes) {
        return BitFieldResult::ErrorExpectedMoreBits;
      }

      uint8_t cur = 0;
      auto r = buf_->try_get(idx, cur);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      const uint8_t avail = static_cast<uint8_t>(8 - pos);
      const uint8_t take = (need < avail ? need : avail);

      const uint8_t shift = static_cast<uint8_t>(avail - take);
      const uint8_t mask = static_cast<uint8_t>(((1u << take) - 1u) << shift);
      v = static_cast<uint8_t>((v << take) | ((cur & mask) >> shift));

      pos = static_cast<uint8_t>(pos + take);
      need = static_cast<uint8_t>(need - take);

      if (pos == 8) {
        pos = 0;
        ++idx;
      }
    }

    idx_ = idx;
    sub_byte_pos_ = pos;
    out = v;
    return BitFieldResult::Ok;
  }
};

#if SUB8_ENABLE_BOUNDED_BUF

template <size_t N> class BoundedByteBuffer {
  uint8_t data_[N]{};
  size_t size_ = 0;
  bool overflow_ = false;

public:
  using value_type = uint8_t;

  BoundedByteBuffer() noexcept = default;

  BoundedByteBuffer(std::initializer_list<uint8_t> init) noexcept : size_(0), overflow_(false) {
    for (auto v : init) {
      if (size_ < N) {
        data_[size_++] = v;
      } else {
        overflow_ = true;
        break;
      }
    }
  }

  constexpr size_t capacity() const noexcept { return N; }
  constexpr size_t size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }
  constexpr bool ok() const noexcept { return !overflow_; }
  uint8_t *data() noexcept { return data_; }
  const uint8_t *data() const noexcept { return data_; }

  void clear() noexcept {
    size_ = 0;
    overflow_ = false;
  }

  BitFieldResult reserve(size_t n) noexcept {
    if (n <= N)
      return BitFieldResult::Ok;
    overflow_ = true;
    return BitFieldResult::ErrorOversizedLength;
  }

  BitFieldResult push_back(uint8_t v) noexcept {
    if (size_ < N) {
      data_[size_++] = v;
      return BitFieldResult::Ok;
    }
    overflow_ = true;
    return BitFieldResult::ErrorInsufficentBufferSize;
  }

  BitFieldResult back_mut_ptr(uint8_t *&out) noexcept {
    if (size_ == 0) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }

  BitFieldResult back_ptr(const uint8_t *&out) const noexcept {
    if (size_ == 0) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }

  uint8_t at(size_t i) const noexcept {
    uint8_t out;
    auto r = try_get(i, out);
    if (r != BitFieldResult::Ok) {
      return 0; // to maintain noexcept, must return 0
    }
    return out;
  }

  BitFieldResult try_get(size_t i, uint8_t &out) const noexcept {
    if (i >= size_) {
      out = 0;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = data_[i];
    return BitFieldResult::Ok;
  }
};

template <size_t N> using BoundedBitWriter = BasicBitWriter<BoundedByteBuffer<N>>;

template <size_t N> using BoundedBitReader = BasicBitReader<BoundedByteBuffer<N>>;

template <typename MessageType> inline constexpr size_t recommended_bounded_buffer_byte_size() noexcept {
  return MessageType::MaxPossibleSize.byte_size_round_up();
}

template <typename MessageType> using BoundedWriterFor = sub8::BoundedBitWriter<recommended_bounded_buffer_byte_size<MessageType>()>;

template <typename MessageType> using BoundedReaderFor = sub8::BoundedBitReader<recommended_bounded_buffer_byte_size<MessageType>()>;

template <typename MessageType> inline BoundedWriterFor<MessageType> make_bounded_writer_for() noexcept {
  return BoundedWriterFor<MessageType>{};
}

template <typename MessageType> inline BoundedReaderFor<MessageType> make_bounded_reader_for() noexcept {
  return BoundedReaderFor<MessageType>{};
}

#endif

#if SUB8_ENABLE_VIEW_BUF

class ByteBufferView {
  uint8_t *data_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
  bool overflow_ = false;

public:
  constexpr ByteBufferView() noexcept = default;

  constexpr ByteBufferView(uint8_t *data, size_t size) noexcept : data_(data), size_(size), capacity_(size) {}

  constexpr ByteBufferView(uint8_t *data, size_t size, size_t capacity) noexcept : data_(data), size_(size), capacity_(capacity) {}

  constexpr size_t capacity() const noexcept { return capacity_; }
  constexpr size_t size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }
  constexpr bool ok() const noexcept { return !overflow_; }

  constexpr uint8_t *data() noexcept { return data_; }
  constexpr const uint8_t *data() const noexcept { return data_; }

  void clear() noexcept {
    size_ = 0;
    overflow_ = false;
  }

  BitFieldResult reserve(size_t n) noexcept {
    if (n <= capacity_)
      return BitFieldResult::Ok;
    overflow_ = true;
    return BitFieldResult::ErrorOversizedLength;
  }

  BitFieldResult push_back(uint8_t v) noexcept {
    if (size_ < capacity_) {
      data_[size_++] = v;
      return BitFieldResult::Ok;
    }
    overflow_ = true;
    return BitFieldResult::ErrorInsufficentBufferSize;
  }

  BitFieldResult try_get(size_t i, uint8_t &out) const noexcept {
    if (i >= size_) {
      out = 0;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = data_[i];
    return BitFieldResult::Ok;
  }

  BitFieldResult back_mut_ptr(uint8_t*& out) noexcept {
    if (size_ == 0) { out = nullptr; return BitFieldResult::ErrorExpectedMoreBits; }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }

  BitFieldResult back_ptr(const uint8_t*& out) const noexcept {
    if (size_ == 0) { out = nullptr; return BitFieldResult::ErrorExpectedMoreBits; }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }
};

using BitWriter = BasicBitWriter<ByteBufferView>;
using BitReader = BasicBitReader<ByteBufferView>;


inline BitReader make_reader(uint8_t *data, BitSize size) noexcept {
  ByteBufferView view(data, size.byte_size_round_up());
  return BitReader(std::move(view), size); 
}

#endif

#if SUB8_ENABLE_UNBOUNDED_BUF

class UnboundedByteBuffer {
  std::vector<uint8_t> buf_;

public:
  using value_type = uint8_t;
  UnboundedByteBuffer() noexcept = default;
  explicit UnboundedByteBuffer(size_t reserve) { buf_.reserve(reserve); } // can throw STL errors
  UnboundedByteBuffer(std::initializer_list<uint8_t> init) : buf_(init) {} // can throw STL errors

  size_t capacity() const noexcept { return buf_.capacity(); }
  size_t size() const noexcept { return buf_.size(); }
  bool empty() const noexcept { return buf_.empty(); }
  constexpr bool ok() const noexcept { return true; }

  uint8_t *data() noexcept { return buf_.data(); }
  const uint8_t *data() const noexcept { return buf_.data(); }

  void clear() noexcept { buf_.clear(); }

  BitFieldResult reserve(size_t n) noexcept {
    try {
      buf_.reserve(n);
      return BitFieldResult::Ok;
    } catch (const std::length_error &) {
      return BitFieldResult::ErrorOversizedLength;
    } catch (const std::bad_alloc &) {
      return BitFieldResult::ErrorBadAlloc;
    } catch (...) {
      // Will only occur if you have some non STL
      return BitFieldResult::ErrorUnidentifiedError;
    }
  }

  BitFieldResult push_back(uint8_t v) noexcept {
    try {
      buf_.push_back(v);
      return BitFieldResult::Ok;
    } catch (const std::bad_alloc &) {
      return BitFieldResult::ErrorBadAlloc;
    } catch (...) {
      // Will only occur if you have some non STL
      return BitFieldResult::ErrorUnidentifiedError;
    }
  }

  BitFieldResult back_mut_ptr(uint8_t *&out) noexcept {
    if (buf_.empty()) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &buf_.back();
    return BitFieldResult::Ok;
  }

  BitFieldResult back_ptr(const uint8_t *&out) const noexcept {
    if (buf_.empty()) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &buf_.back();
    return BitFieldResult::Ok;
  }

  uint8_t at(size_t i) const noexcept {
    uint8_t out;
    auto r = try_get(i, out);
    if (r != BitFieldResult::Ok) {
      return 0; // to maintain noexcept, must return 0
    }
    return out;
  }

  BitFieldResult try_get(size_t i, uint8_t &out) const noexcept {
    if (i >= buf_.size()) {
      out = 0;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = buf_[i];
    return BitFieldResult::Ok;
  }
};

using UnboundedBitWriter = BasicBitWriter<UnboundedByteBuffer>;
using UnboundedBitReader = BasicBitReader<UnboundedByteBuffer>;

template <typename MessageType> constexpr size_t recommended_unbounded_buffer_byte_size(uint8_t percentage_0_100) noexcept {
  const size_t min_b = MessageType::MinPossibleSize.byte_size_round_up();
  const size_t max_b = MessageType::MaxPossibleSize.byte_size_round_up();

  if (max_b <= min_b)
    return min_b;

  const size_t delta = max_b - min_b;

  const size_t p = (percentage_0_100 > 100u) ? 100u : size_t{percentage_0_100};
  return min_b + (delta * p) / 100u;
}

template <typename MessageType> inline sub8::BitFieldResult init_unbounded_writer_for(sub8::UnboundedBitWriter &out,
  uint8_t reserve_percentage_0_100 = 0) noexcept {
  out.storage().clear();
  const size_t reserve_bytes = recommended_unbounded_buffer_byte_size<MessageType>(reserve_percentage_0_100);
  return out.storage().reserve(reserve_bytes);
}

#endif

} // namespace sub8
// ============================================================
// END INLINE ./sub8_io.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_api.h
// ============================================================

#pragma once

#include <cstdint>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_errors.h"
#endif

namespace sub8 {

template <typename Storage> class BasicBitReader;
template <typename Storage> class BasicBitWriter;

// Reads and returns the value as the feild type
template <typename TFeildType, typename Storage> inline BitFieldResult read(BasicBitReader<Storage> &br, TFeildType &out) {
  TFeildType f{};
  auto r = read_field(br, f);
  if (r == BitFieldResult::Ok) {
    out = f;
  }
  return r;
}

template <typename TFeildType, typename Storage> inline TFeildType read_or_throw(BasicBitReader<Storage> &br) {
  TFeildType ret{};
  auto r = read_field(br, ret);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType, "sub8::read_or_throw<TFeildType>(BasicBitReader<Storage>)");
  }
  return ret;
}

// Write

template <typename TFeildType, typename Storage> inline void write_or_throw(BasicBitWriter<Storage> &br, TFeildType in) {
  auto r = write_field(br, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
      "sub8::write_or_throw<TFeildType>(BasicBitWriter<"
      "Storage>, TFeildType)");
  }
}

template <typename TFeildType, typename Storage> inline void write_or_throw(BasicBitWriter<Storage> &br, typename TFeildType::InitType in) {
  TFeildType v{};
  auto r = v.set_value(in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
      "sub8::write_or_throw<TFeildType>(BasicBitWriter<"
      "Storage>, TFeildType::InitType)");
  }

  r = write_field(br, v);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
      "sub8::write_or_throw<TFeildType>(BasicBitWriter<"
      "Storage>, TFeildType::InitType)");
  }
}
} // namespace sub8
// ============================================================
// END INLINE ./sub8_api.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_strings.h
// ============================================================

#pragma once

// Enable: 5bit String Fields
// 5bit strings which us control chars to shift between character sheets and
// enable a extended 10bit encoding mode.
#ifndef SUB8_ENABLE_FIVE_BIT_STRING
#define SUB8_ENABLE_FIVE_BIT_STRING 1
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS
// Auto enable foundational string type
#if SUB8_ENABLE_FIVE_BIT_STRING
#define SUB8_ENABLE_STRING_FIELDS 1
#else
#define SUB8_ENABLE_STRING_FIELDS 0
#endif
#endif

#ifndef SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_STL_TYPE 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_BASIC_STRING
#define SUB8_ENABLE_STL_TYPE_BASIC_STRING SUB8_ENABLE_STL_TYPE
#endif

#if SUB8_ENABLE_STRING_FIELDS
// Enable / Disable specific characters widths
#ifndef SUB8_ENABLE_STRING_FIELDS__CHAR
#define SUB8_ENABLE_STRING_FIELDS__CHAR 1
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__WCHAR
#define SUB8_ENABLE_STRING_FIELDS__WCHAR 1
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__U8_CHAR
#if __cplusplus >= 202002L
#define SUB8_ENABLE_STRING_FIELDS__U8_CHAR 1
#else
#define SUB8_ENABLE_STRING_FIELDS__U8_CHAR 0
#endif
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__U16_CHAR
#if __cplusplus >= 201103L
#define SUB8_ENABLE_STRING_FIELDS__U16_CHAR 1
#else
#define SUB8_ENABLE_STRING_FIELDS__U16_CHAR 0
#endif
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__U32_CHAR
#if __cplusplus >= 201103L
#define SUB8_ENABLE_STRING_FIELDS__U32_CHAR 1
#else
#define SUB8_ENABLE_STRING_FIELDS__U32_CHAR 0
#endif
#endif
#else
#define SUB8_ENABLE_STRING_FIELDS__CHAR 0
#define SUB8_ENABLE_STRING_FIELDS__WCHAR 0
#define SUB8_ENABLE_STRING_FIELDS__U8_CHAR 0
#define SUB8_ENABLE_STRING_FIELDS__U16_CHAR 0
#define SUB8_ENABLE_STRING_FIELDS__U32_CHAR 0
#endif

#if SUB8_ENABLE_STRING_FIELDS

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8.h"
#include "sub8_errors.h"
#include "sub8_io.h"
#include "sub8_type_information.h"
#endif

#include <cstring>
#include <cstdint> // uintx_t
#include <utility> // std::declval

#include <type_traits>
#include <uchar.h>

#if SUB8_ENABLE_STL_TYPE_BASIC_STRING
#include <string_view>
#include <string>
#endif

namespace sub8 {
namespace utf {
template <typename CharT> struct UtfMaxUnits;
template <typename T> size_t encode_codepoint_to_utf(char32_t cp, T *out) noexcept;

#if SUB8_ENABLE_STRING_FIELDS__CHAR
template <> struct UtfMaxUnits<char> {
  static constexpr size_t value = 4;
};

template <> inline size_t encode_codepoint_to_utf<char>(char32_t cp, char *out) noexcept {
  // Reject invalid code points (surrogates / > U+10FFFF)
  if (cp > 0x10FFFFu) {
    return 0;
  }
  if (cp >= 0xD800u && cp <= 0xDFFFu) {
    return 0; // UTF-16 surrogate range, not a Unicode scalar value
  }

  if (cp <= 0x7Fu) {
    out[0] = static_cast<char>(cp);
    return 1;
  }
  if (cp <= 0x7FFu) {
    out[0] = static_cast<uint8_t>(0xC0u | ((cp >> 6) & 0x1Fu));
    out[1] = static_cast<uint8_t>(0x80u | (cp & 0x3Fu));
    return 2;
  }
  if (cp <= 0xFFFFu) {
    out[0] = static_cast<uint8_t>(0xE0u | ((cp >> 12) & 0x0Fu));
    out[1] = static_cast<uint8_t>(0x80u | ((cp >> 6) & 0x3Fu));
    out[2] = static_cast<uint8_t>(0x80u | (cp & 0x3Fu));
    return 3;
  }

  // 4-byte sequence
  out[0] = static_cast<uint8_t>(0xF0u | ((cp >> 18) & 0x07u));
  out[1] = static_cast<uint8_t>(0x80u | ((cp >> 12) & 0x3Fu));
  out[2] = static_cast<uint8_t>(0x80u | ((cp >> 6) & 0x3Fu));
  out[3] = static_cast<uint8_t>(0x80u | (cp & 0x3Fu));
  return 4;
}

#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR
template <> struct UtfMaxUnits<char8_t> {
  static constexpr size_t value = 4;
};

template <> inline size_t encode_codepoint_to_utf<char8_t>(char32_t cp, char8_t *out) noexcept {
  char tmp[4];
  const size_t n = encode_codepoint_to_utf<char>(cp, tmp);
  for (size_t i = 0; i < n; ++i)
    out[i] = static_cast<char8_t>(static_cast<unsigned char>(tmp[i]));
  return n;
}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR
template <> struct UtfMaxUnits<char16_t> {
  static constexpr size_t value = 2;
};

template <> inline size_t encode_codepoint_to_utf<char16_t>(char32_t cp, char16_t *out) noexcept {
  // Reject invalid code points (surrogates / > U+10FFFF)
  if (cp > 0x10FFFFu) {
    return 0;
  }
  if (cp >= 0xD800u && cp <= 0xDFFFu) {
    return 0; // stand-alone surrogates invalid as scalar values
  }

  if (cp <= 0xFFFFu) {
    out[0] = static_cast<uint16_t>(cp);
    return 1;
  }

  // Encode surrogate pair
  char32_t v = cp - 0x10000u;
  uint16_t high = static_cast<uint16_t>(0xD800u + ((v >> 10) & 0x3FFu));
  uint16_t low = static_cast<uint16_t>(0xDC00u + (v & 0x3FFu));

  out[0] = high;
  out[1] = low;
  return 2;
}

#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR
template <> struct UtfMaxUnits<char32_t> {
  static constexpr size_t value = 1;
};

template <> inline size_t encode_codepoint_to_utf<char32_t>(char32_t cp, char32_t *out) noexcept {
  // Reject invalid code points (surrogates / > U+10FFFF)
  if (cp > 0x10FFFFu) {
    return 0;
  }
  if (cp >= 0xD800u && cp <= 0xDFFFu) {
    return 0;
  }

  out[0] = static_cast<uint32_t>(cp);
  return 1;
}

#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
template <> struct UtfMaxUnits<wchar_t> {
  static constexpr size_t value = (sizeof(wchar_t) == 2 ? 2 : 1);
};

template <> inline size_t encode_codepoint_to_utf<wchar_t>(char32_t cp, wchar_t *out) noexcept {
  if constexpr (sizeof(wchar_t) == 2) {
    char16_t tmp[2];
    const size_t n = encode_codepoint_to_utf<char16_t>(cp, tmp);
    if (n == 0)
      return 0;
    out[0] = static_cast<wchar_t>(tmp[0]);
    if (n == 2)
      out[1] = static_cast<wchar_t>(tmp[1]);
    return n;
  } else {
    char32_t tmp[1];
    const size_t n = encode_codepoint_to_utf<char32_t>(cp, tmp);
    if (n == 0)
      return 0;
    out[0] = static_cast<wchar_t>(tmp[0]);
    return 1;
  }
}

#endif

// Codepoint iterator is used to abstract the underlying storage,
// ie UTF-8/16/32 from the encoding layer which only cares about individual code
// points
class UtfCodepointIterator {
public:
  enum class Encoding : uint8_t { Utf8, Utf16, Utf32 };

  UtfCodepointIterator() noexcept : encoding_(Encoding::Utf8), start_(nullptr), cur_(nullptr), end_(nullptr) {}

#if SUB8_ENABLE_STRING_FIELDS__CHAR
  explicit UtfCodepointIterator(const char *utf8_z, size_t len) noexcept
      : encoding_(Encoding::Utf8), start_(utf8_z), cur_(utf8_z), end_(utf8_z ? utf8_z + len : utf8_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
  explicit UtfCodepointIterator(const wchar_t *utf_wide_z, size_t len) noexcept
      : encoding_((sizeof(wchar_t) == 2 ? Encoding::Utf16 : Encoding::Utf32)), start_(utf_wide_z), cur_(utf_wide_z),
        end_(utf_wide_z ? utf_wide_z + len : utf_wide_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR
  explicit UtfCodepointIterator(const char8_t *utf8_z, size_t len) noexcept
      : encoding_(Encoding::Utf8), start_(utf8_z), cur_(utf8_z), end_(utf8_z ? utf8_z + len : utf8_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR
  explicit UtfCodepointIterator(const char16_t *utf16_z, size_t len) noexcept
      : encoding_(Encoding::Utf16), start_(utf16_z), cur_(utf16_z), end_(utf16_z ? utf16_z + len : utf16_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR
  explicit UtfCodepointIterator(const char32_t *utf32_z, size_t len) noexcept
      : encoding_(Encoding::Utf32), start_(utf32_z), cur_(utf32_z), end_(utf32_z ? utf32_z + len : utf32_z) {}
#endif

  void reset() noexcept { cur_ = start_; }

  // Advance and decode next codepoint.
  bool try_get_next_utf32_char(char32_t &out_cp) noexcept {
    switch (encoding_) {
    case Encoding::Utf8:
      return decode_utf8(out_cp, /* advance_cur */ true);
    case Encoding::Utf16:
      return decode_utf16(out_cp, /* advance_cur */ true);
    case Encoding::Utf32:
      return decode_utf32(out_cp, /* advance_cur */ true);
    default:
      return false;
    }
  }

  bool try_peek_next_utf32_char(char32_t &out_cp) noexcept {
    switch (encoding_) {
    case Encoding::Utf8:
      return decode_utf8(out_cp, /* advance_cur */ false);
    case Encoding::Utf16:
      return decode_utf16(out_cp, /* advance_cur */ false);
    case Encoding::Utf32:
      return decode_utf32(out_cp, /* advance_cur */ false);
    default:
      return false;
    }
  }

private:
  Encoding encoding_;
  const void *start_{nullptr};
  const void *cur_{nullptr};
  const void *end_{nullptr};

  // ---- UTF-8 decoding ----
  bool decode_utf8(char32_t &out_cp, bool advance_cur = true) noexcept {
    const char *p = static_cast<const char *>(cur_);
    const char *end = static_cast<const char *>(end_);

    if (!p || !end || p >= end) {
      return false;
    }

    const unsigned char *s = reinterpret_cast<const unsigned char *>(p);
    unsigned char b0 = *s;

    // Helper for continuation bytes
    auto is_cont = [](unsigned char b) noexcept { return (b & 0xC0u) == 0x80u; };

    // 1-byte (ASCII)
    if (b0 < 0x80u) {
      out_cp = static_cast<char32_t>(b0);
      p = reinterpret_cast<const char *>(s + 1);
      if (advance_cur)
        cur_ = p;
      return true;
    }

    // Determine sequence length
    int len = 0;
    char32_t cp = 0;

    if ((b0 & 0xE0u) == 0xC0u) { // 110xxxxx -> 2-byte
      len = 2;
      cp = b0 & 0x1Fu;
    } else if ((b0 & 0xF0u) == 0xE0u) { // 1110xxxx -> 3-byte
      len = 3;
      cp = b0 & 0x0Fu;
    } else if ((b0 & 0xF8u) == 0xF0u) { // 11110xxx -> 4-byte
      len = 4;
      cp = b0 & 0x07u;
    } else {
      // Invalid leading byte: replacement, advance one byte
      out_cp = 0xFFFDu;
      p = reinterpret_cast<const char *>(s + 1);
      if (p > end)
        p = end;
      if (advance_cur)
        cur_ = p;
      return true;
    }

    // Ensure we have enough bytes
    if (p + len > end) {
      // truncated sequence at end
      out_cp = 0xFFFDu;
      if (advance_cur)
        cur_ = end;
      return true;
    }

    const unsigned char *q = s + 1;
    for (int i = 1; i < len; ++i) {
      unsigned char bx = q[i - 1];
      if (!is_cont(bx)) {
        // Invalid continuation -> replacement, skip just the first byte
        out_cp = 0xFFFDu;
        p = reinterpret_cast<const char *>(s + 1);
        if (p > end)
          p = end;
        if (advance_cur)
          cur_ = p;
        return true;
      }
      cp = (cp << 6) | (bx & 0x3Fu);
    }

    // Minimal form & range checks
    if ((len == 2 && cp < 0x80u) || (len == 3 && cp < 0x800u) || (len == 4 && cp < 0x10000u) || cp > 0x10FFFFu ||
        (cp >= 0xD800u && cp <= 0xDFFFu)) {
      out_cp = 0xFFFDu;
    } else {
      out_cp = cp;
    }

    p = reinterpret_cast<const char *>(s + len);
    if (p > end)
      p = end;
    if (advance_cur)
      cur_ = p;
    return true;
  }

  // ---- UTF-16 decoding ----
  bool decode_utf16(char32_t &out_cp, bool advance_cur = true) noexcept {
    const char16_t *p = static_cast<const char16_t *>(cur_);
    const char16_t *end = static_cast<const char16_t *>(end_);

    if (!p || !end || p >= end) {
      return false;
    }

    char16_t w1 = *p++;
    // Single-unit BMP (non-surrogate)
    if (w1 < 0xD800u || w1 > 0xDFFFu) {
      out_cp = static_cast<char32_t>(w1);
      if (advance_cur)
        cur_ = p;
      return true;
    }

    // Surrogates
    if (w1 >= 0xD800u && w1 <= 0xDBFFu) { // high surrogate
      if (p >= end) {
        out_cp = 0xFFFDu; // truncated
        if (advance_cur)
          cur_ = end;
        return true;
      }
      char16_t w2 = *p;
      if (w2 >= 0xDC00u && w2 <= 0xDFFFu) {
        ++p;
        char32_t hi = static_cast<char32_t>(w1 - 0xD800u);
        char32_t lo = static_cast<char32_t>(w2 - 0xDC00u);
        out_cp = ((hi << 10) | lo) + 0x10000u;
        if (advance_cur)
          cur_ = p;
        return true;
      } else {
        // Isolated high surrogate
        out_cp = 0xFFFDu;
        if (advance_cur)
          cur_ = p;
        return true;
      }
    }

    // Isolated low surrogate
    out_cp = 0xFFFDu;
    if (advance_cur)
      cur_ = p;
    return true;
  }

  // ---- UTF-32 decoding ----
  bool decode_utf32(char32_t &out_cp, bool advance_cur = true) noexcept {
    const char32_t *p = static_cast<const char32_t *>(cur_);
    const char32_t *end = static_cast<const char32_t *>(end_);

    if (!p || !end || p >= end) {
      return false;
    }

    char32_t cp = *p++;
    if (cp > 0x10FFFFu || (cp >= 0xD800u && cp <= 0xDFFFu)) {
      out_cp = 0xFFFDu;
    } else {
      out_cp = cp;
    }

    if (advance_cur)
      cur_ = p;
    return true;
  }
};

template <typename T> struct UtfCodepointMapper {
  using value_type = T;
  static value_type map(char32_t cp) noexcept;
};

// Look ahead iterator is used to "look ahead" n code points. This is
// used to allow the encoding layer to factor for non-scalar glyphs
// and allow for encoding optimizations
template <typename Mapper, size_t LookAheadDistance = 3> class UtfCodepointLookAheadIteratorT {
public:
  using value_type = typename Mapper::value_type;

  UtfCodepointLookAheadIteratorT() noexcept = default;

  explicit UtfCodepointLookAheadIteratorT(utf::UtfCodepointIterator iter) noexcept : iter_(iter) { fill_initial_buffer(); }

  bool empty() const noexcept { return buf_len_ == 0; }

  void reset() noexcept {
    iter_.reset();
    buf_idx_ = 0;
    buf_len_ = 0;
    fill_initial_buffer();
  }

  bool try_pop(value_type &out_value) noexcept {
    if (buf_len_ == 0)
      return false;

    out_value = buf_[buf_idx_];
    advance_buffer(1);
    fill_to_capacity();
    return true;
  }

  bool try_peek_ahead(size_t offset, value_type &out_value) const noexcept {
    if (offset >= buf_len_)
      return false;

    out_value = buf_[(buf_idx_ + offset) % LookAheadDistance];
    return true;
  }

  bool try_peek_next(value_type &out_value) const noexcept { return try_peek_ahead(0, out_value); }

private:
  void advance_buffer(size_t distance) noexcept {
    if (distance >= buf_len_) {
      buf_idx_ = 0;
      buf_len_ = 0;
      return;
    }

    buf_idx_ = (buf_idx_ + distance) % LookAheadDistance;
    buf_len_ -= static_cast<uint8_t>(distance);
  }

  void enqueue(const value_type &value) noexcept {
    buf_[(buf_idx_ + buf_len_) % LookAheadDistance] = value;

    if (buf_len_ < LookAheadDistance) {
      ++buf_len_;
    } else {
      buf_idx_ = (buf_idx_ + 1) % LookAheadDistance;
    }
  }

  void fill_initial_buffer() noexcept {
    for (size_t i = 0; i < LookAheadDistance; ++i) {
      value_type value{};
      if (!read_next(value))
        break;
      enqueue(value);
    }
  }

  void fill_to_capacity() noexcept {
    while (buf_len_ < LookAheadDistance) {
      value_type value{};
      if (!read_next(value))
        break;
      enqueue(value);
    }
  }

  bool read_next(value_type &out_value) noexcept {
    char32_t cp = 0;
    if (!iter_.try_get_next_utf32_char(cp))
      return false;

    out_value = Mapper::map(cp);
    return true;
  }

private:
  utf::UtfCodepointIterator iter_{};

  value_type buf_[LookAheadDistance]{};
  uint8_t buf_idx_ = 0;
  uint8_t buf_len_ = 0;
};

template <> struct UtfCodepointMapper<char32_t> {
  using value_type = char32_t;

  static value_type map(char32_t cp) noexcept { return cp; }
};

template <size_t LookAheadDistance = 3> using UtfCodepointLookAheadIterator =
  UtfCodepointLookAheadIteratorT<UtfCodepointMapper<char32_t>, LookAheadDistance>;

} // namespace utf

#if SUB8_ENABLE_BOUNDED_BUF

template <class CharT, size_t Len> 
class BoundedBasicString {

public:
  using CharType = CharT;
  using size_type = size_t;

  static_assert(std::is_nothrow_copy_assignable<CharType>::value, "BoundedBasicString requires nothrow copy-assignable CharType");

  static constexpr size_t kCapacity = Len;

  constexpr BoundedBasicString() noexcept { clear(); }

  constexpr size_t capacity() const noexcept { return kCapacity; }
  constexpr size_t max_size() const noexcept { return kCapacity; }
  constexpr size_t size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }
  constexpr bool ok() const noexcept { return !overflow_; }
  constexpr bool overflowed() const noexcept { return overflow_; }

  CharType *data() noexcept { return data_; }
  const CharType *data() const noexcept { return data_; }
  const CharType *c_str() const noexcept { return data_; }

  #if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  std::basic_string_view<CharType> view() const noexcept { return {data_, size_}; }
  #endif

  void clear() noexcept {
    size_ = 0;
    overflow_ = false;
    data_[0] = CharType(0);
  }

  SUB8_NO_DISCARD BitFieldResult resize(size_t n, CharType fill = CharType(0)) noexcept {

    if (n > kCapacity) {
      n = kCapacity;
      overflow_ = true;
    }
    if (n > size_) {
      for (size_t i = size_; i < n; ++i)
        data_[i] = fill;
    }
    size_ = n;
    data_[size_] = CharType(0);
    return overflow_ ? BitFieldResult::ErrorInsufficentBufferSize : BitFieldResult::Ok;
  }

  #if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  SUB8_NO_DISCARD BitFieldResult set_value(std::basic_string_view<CharType> s) noexcept {
    clear();
    return append(s.data(), s.size());
  }
  #endif

  SUB8_NO_DISCARD BitFieldResult set_value(const CharType *zstr) noexcept {
    clear();
    if (!zstr) {
      return BitFieldResult::Ok;
    }
    size_t n = 0;
    while (zstr[n] != CharType(0))
      ++n; // strlen
    return append(zstr, n);
  }

  SUB8_NO_DISCARD BitFieldResult set_value(const CharType *ptr, size_t n) noexcept {
    clear();
    if (!ptr || n == 0) {
      return BitFieldResult::Ok;
    }
    return append(ptr, n);
  }

  SUB8_NO_DISCARD BitFieldResult push_back(CharType c) noexcept {
    if (size_ >= kCapacity) {
      overflow_ = true;
      data_[kCapacity] = CharType(0);
      return BitFieldResult::ErrorInsufficentBufferSize;
    }
    data_[size_++] = c;
    data_[size_] = CharType(0);
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult append(const CharType *ptr, size_t n) noexcept {
    if (!ptr || n == 0) {
      return BitFieldResult::Ok;
    }

    const size_t avail = (kCapacity > size_) ? (kCapacity - size_) : 0;
    const size_t to_copy = (n <= avail) ? n : avail;

    if (to_copy > 0) {
      CharT *dst = data_ + size_;
      const CharT *src = ptr;

      if constexpr (std::is_trivially_copyable<CharType>::value) {
        std::memmove(static_cast<void *>(dst), static_cast<const void *>(src), to_copy * sizeof(CharType));
      } else {
        // "memmove semantics" for non-trivial: copy backward if overlap and src
        // < dst ranges overlap if [src, src+to_copy) intersects [dst,
        // dst+to_copy
        const CharT *begin = data_;
        const CharT *endp = data_ + (kCapacity + 1);

        const bool src_in_self = (src >= begin && src < endp);
        const bool dst_in_self = (dst >= begin && dst < endp);

        bool overlap = false;
        if (src_in_self && dst_in_self) {
          overlap = (src < (dst + to_copy)) && (dst < (src + to_copy));
        }

        if (overlap && src < dst) {
          for (size_t i = to_copy; i-- > 0;) {
            dst[i] = src[i];
          }
        } else {
          for (size_t i = 0; i < to_copy; ++i) {
            dst[i] = src[i];
          }
        }
      }

      size_ += to_copy;
    }

    data_[size_] = CharType(0);

    if (to_copy != n) {
      overflow_ = true;
      return BitFieldResult::ErrorInsufficentBufferSize;
    }
    return BitFieldResult::Ok;
  }

  #if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  SUB8_NO_DISCARD BitFieldResult append(std::basic_string_view<CharType> s) noexcept { return append(s.data(), s.size()); }
  #endif

  friend bool operator==(const BoundedBasicString &a, const BoundedBasicString &b) noexcept {
    if (a.size_ != b.size_) {
      return false;
    }

    if constexpr (std::is_trivially_copyable<CharType>::value) {
      return std::memcmp(static_cast<const void *>(a.data_), static_cast<const void *>(b.data_), a.size_ * sizeof(CharType)) == 0;
    } else {
      for (size_t i = 0; i < a.size_; ++i) {
        if (!(a.data_[i] == b.data_[i]))
          return false;
      }
      return true;
    }
  }

  friend bool operator!=(const BoundedBasicString &a, const BoundedBasicString &b) noexcept { return !(a == b); }

private:
  // +1 for null terminator
  CharType data_[kCapacity + 1]{};
  size_t size_ = 0;
  bool overflow_ = false;
};

#if SUB8_ENABLE_STL_TYPE_BASIC_STRING

template <class CharT, size_t Len> inline bool operator==(
  const BoundedBasicString<CharT, Len> &a,
  std::basic_string_view<CharT> b
) noexcept {
  return a.size() == b.size() && a.view() == b;
}
template <class CharT, size_t Len> inline bool operator==(
  std::basic_string_view<CharT> a,
  const BoundedBasicString<CharT, Len> &b
) noexcept {
  return a.size() == b.size() && a == b.view();
}


template <class CharT, size_t Len> inline bool operator!=(
  const BoundedBasicString<CharT, Len> &a,
  std::basic_string_view<CharT> b
) noexcept {
  return !(a == b);
}
template <class CharT, size_t Len> inline bool operator!=(
  std::basic_string_view<CharT> a,
  const BoundedBasicString<CharT, Len> &b
) noexcept {
  return !(a == b);
}


template <class CharT, size_t Len> inline bool operator==(
  const BoundedBasicString<CharT, Len> &a, 
  const std::basic_string<CharT> &b
) noexcept {
  return a.size() == b.size() && a.view() == std::basic_string_view<CharT>(b);
}

template <class CharT, size_t Len> inline bool operator==(
  const std::basic_string<CharT> &a, 
  const BoundedBasicString<CharT, Len> &b
) noexcept {
  return a.size() == b.size() && std::basic_string_view<CharT>(a) == b.view();
}

template <class CharT, size_t Len> inline bool operator!=(const std::basic_string<CharT> &a,
  const BoundedBasicString<CharT, Len> &b) noexcept {
  return !(a == b);
}

template <class CharT, size_t Len> inline bool operator!=(
  const BoundedBasicString<CharT, Len> &a,
  const std::basic_string<CharT> &b
) noexcept {
  return !(a == b);
}

#endif

// CharT * Eq
template <class CharT, size_t Len> inline bool operator==(const BoundedBasicString<CharT, Len> &a, const CharT *b) noexcept {
  if (!b) {
    return a.size() == 0;
  }

#if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  return a.view() == std::basic_string_view<CharT>(b);
#else
  // manual zstr compare
  size_t i = 0;
  for (; i < a.size(); ++i) {
    if (b[i] == CharT(0)) return false;
    if (!(a.data()[i] == b[i])) return false;
  }
  return b[i] == CharT(0);
#endif
}

template <class CharT, size_t Len> inline bool operator==(const CharT *a, const BoundedBasicString<CharT, Len> &b) noexcept {
  if (!a) {
    return b.size() == 0;
  }

#if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  return std::basic_string_view<CharT>(a) == b.view();
#else
  // manual zstr compare
  size_t i = 0;
  for (; i < b.size(); ++i) {
    if (a[i] == CharT(0)) return false;
    if (!(b.data()[i] == a[i])) return false;
  }
  return a[i] == CharT(0);
#endif
}

template <class CharT, size_t Len> inline bool operator!=(const BoundedBasicString<CharT, Len> &a, const CharT *b) noexcept {
  return !(a == b);
}
template <class CharT, size_t Len> inline bool operator!=(const CharT *a, const BoundedBasicString<CharT, Len> &b) noexcept {
  return !(a == b);
}

#endif // SUB8_ENABLE_BOUNDED_BUF

#if SUB8_ENABLE_UNBOUNDED_BUF

// currently no Unbounded implementation which doesnt use stl
#if SUB8_ENABLE_STL_TYPE_BASIC_STRING
template <class CharT> class UnboundedBasicString {
public:
  using CharType = CharT;
  using size_type = size_t;
  using string_type = std::basic_string<CharType>;

  UnboundedBasicString() noexcept = default;

  // Non-throwing clear
  void clear() noexcept { s_.clear(); }

  // Introspection
  size_t size() const noexcept { return s_.size(); }
  bool empty() const noexcept { return s_.empty(); }
  const CharType *data() const noexcept { return s_.data(); }
  const CharType *c_str() const noexcept { return s_.c_str(); }
  const string_type &value() const noexcept { return s_; }
  string_type &value_unsafe() noexcept { return s_; } // for non-noexcept callers who accept exceptions
  std::basic_string_view<CharType> view() const noexcept { return std::basic_string_view<CharType>(s_); }

  SUB8_NO_DISCARD BitFieldResult set_value(std::basic_string_view<CharType> v) noexcept {
    try {
      s_.assign(v.data(), v.size());
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  SUB8_NO_DISCARD BitFieldResult set_value(const string_type &v) noexcept {
    try {
      s_ = v;
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  SUB8_NO_DISCARD BitFieldResult set_value(const CharType *zstr) noexcept {
    if (!zstr) {
      s_.clear();
      return BitFieldResult::Ok;
    }
    try {
      s_.assign(zstr);
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  SUB8_NO_DISCARD BitFieldResult set_value(const CharType *ptr, size_t n) noexcept {
    if (!ptr || n == 0) {
      s_.clear();
      return BitFieldResult::Ok;
    }
    try {
      s_.assign(ptr, n);
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  SUB8_NO_DISCARD BitFieldResult reserve(size_t n) noexcept {
    try {
      s_.reserve(n);
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  SUB8_NO_DISCARD BitFieldResult append(const CharT *ptr, size_t n) noexcept {
    if (!ptr || n == 0)
      return BitFieldResult::Ok;
    try {
      s_.append(ptr, n);
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  SUB8_NO_DISCARD BitFieldResult append(std::basic_string_view<CharT> v) noexcept { return append(v.data(), v.size()); }

  SUB8_NO_DISCARD BitFieldResult push_back(CharT c) noexcept {
    try {
      s_.push_back(c);
      return BitFieldResult::Ok;
    } catch (...) {
      return error::map_std_exception_to_bitfield_result();
    }
  }

  friend bool operator==(const UnboundedBasicString &a, const UnboundedBasicString &b) noexcept { return a.s_ == b.s_; }
  friend bool operator!=(const UnboundedBasicString &a, const UnboundedBasicString &b) noexcept { return !(a == b); }

private:
  string_type s_;
};

template <class CharT> inline bool operator==(const UnboundedBasicString<CharT> &a, std::basic_string_view<CharT> b) noexcept {
  return a.view() == b;
}
template <class CharT> inline bool operator==(std::basic_string_view<CharT> a, const UnboundedBasicString<CharT> &b) noexcept {
  return a == b.view();
}

template <class CharT> inline bool operator!=(const UnboundedBasicString<CharT> &a, std::basic_string_view<CharT> b) noexcept {
  return !(a == b);
}
template <class CharT> inline bool operator!=(std::basic_string_view<CharT> a, const UnboundedBasicString<CharT> &b) noexcept {
  return !(a == b);
}

// std::basic_string<CharT> eq
template <class CharT> inline bool operator==(const UnboundedBasicString<CharT> &a, const std::basic_string<CharT> &b) noexcept {
  return a.view() == std::basic_string_view<CharT>(b);
}
template <class CharT> inline bool operator==(const std::basic_string<CharT> &a, const UnboundedBasicString<CharT> &b) noexcept {
  return std::basic_string_view<CharT>(a) == b.view();
}

template <class CharT> inline bool operator!=(const UnboundedBasicString<CharT> &a, const std::basic_string<CharT> &b) noexcept {
  return !(a == b);
}
template <class CharT> inline bool operator!=(const std::basic_string<CharT> &a, const UnboundedBasicString<CharT> &b) noexcept {
  return !(a == b);
}

// CharT * Eq
template <class CharT> inline bool operator==(const CharT *a, const UnboundedBasicString<CharT> &b) noexcept {
  if (!a) {
    return b.size() == 0;
  }
  return std::basic_string_view<CharT>(a) == b.view();
}
template <class CharT> inline bool operator==(const UnboundedBasicString<CharT> &a, const CharT *b) noexcept {
  if (!b) {
    return a.size() == 0;
  }
  return a.view() == std::basic_string_view<CharT>(b);
}
template <class CharT> inline bool operator!=(const CharT *a, const UnboundedBasicString<CharT> &b) noexcept { return !(a == b); }
template <class CharT> inline bool operator!=(const UnboundedBasicString<CharT> &a, const CharT *b) noexcept { return !(a == b); }

#endif // SUB8_ENABLE_STL_TYPE_BASIC_STRING
#endif // SUB8_ENABLE_UNBOUNDED_BUF

template <
  // CharT: underlying string character type
  typename StringBuffer,

  // Encoder / Decoder: Text encoder to be use for transcoding the string
  typename Encoder, typename Decoder>
class BasicString {
public:
  using CharType = typename StringBuffer::CharType;
  using ValueType = const CharType*;
  using InitType = const CharType*;

  using StorageType = typename unpack_t::underlying_or_self<CharType>::type;
  using EncoderType = Encoder;
  using DecoderType = Decoder;

  static constexpr BitSize MaxPossibleSize = Encoder::MaxPossibleSize;
  static constexpr BitSize MinPossibleSize = Encoder::MinPossibleSize;

  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  BitSize actual_size() const noexcept {
    utf::UtfCodepointIterator it(data_ptr(), data_len());
    return Encoder::actual_size(it);
  }

  BasicString() noexcept = default;
  BasicString(const BasicString &) noexcept = default;
  #if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  BasicString(const std::basic_string<CharType> &init) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, BasicString, "sub8::BasicString(const std::basic_string<CharType>)");
  }
  #endif

  #if !SUB8_ENABLE_INFALLIBLE
  BasicString(const CharType *cstr) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(cstr);
    SUB8_THROW_IF_ERROR(r, BasicString, "sub8::BasicString(const CharType *)");
  }
  #endif

  #if !SUB8_ENABLE_INFALLIBLE
  BasicString(const CharType *cstr, size_t n) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(cstr, n);
    SUB8_THROW_IF_ERROR(r, BasicString, "sub8::BasicString(const CharType *, size_t)");
  }
  #endif

  size_t size() const noexcept { return data_.size(); }

  const StringBuffer &value() const noexcept { return data_; }
  StringBuffer &value() noexcept { return data_; }

  std::basic_string_view<CharType> view() const noexcept { return data_.view(); }

  explicit operator const StringBuffer &() const noexcept { return data_; }
  bool operator==(const BasicString &o) const noexcept { return data_ == o.data_; }
  bool operator!=(const BasicString &o) const noexcept { return !(*this == o); }

  BasicString &operator=(const BasicString &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  BasicString &operator=(const std::basic_string<CharType> &s) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(s);
    SUB8_THROW_IF_ERROR(r, BasicString, "sub8::BasicString &operator=");
    return *this;
  }
  #endif

  #if !SUB8_ENABLE_INFALLIBLE
  BasicString &operator=(const CharType *cstr) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(cstr);
    SUB8_THROW_IF_ERROR(r, BasicString, "sub8::BasicString &operator=");
    return *this;
  }
  #endif

  void clear() noexcept { data_.clear(); }

  SUB8_NO_DISCARD BitFieldResult set_value(const std::basic_string<CharType> &s) noexcept { return data_.set_value(s); }

  SUB8_NO_DISCARD BitFieldResult set_value(const CharType *cstr) noexcept { return data_.set_value(cstr); }

  SUB8_NO_DISCARD BitFieldResult set_value(const CharType *cstr, size_t n) noexcept { return data_.set_value(cstr, n); }

  SUB8_NO_DISCARD BitFieldResult push_back(CharType c) noexcept { return data_.push_back(c);}

  SUB8_NO_DISCARD BitFieldResult append(const CharType *ptr, size_t n) noexcept { return data_.append(ptr, n); }

  #if SUB8_ENABLE_STL_TYPE_BASIC_STRING
  SUB8_NO_DISCARD BitFieldResult append(std::basic_string_view<CharType> s) noexcept { return data_.append(s); }
  #endif

  // Expose raw data for write_field
  const CharType *data_ptr() const noexcept { return data_.data(); }
  size_t data_len() const noexcept { return data_.size(); }

  private:
  StringBuffer data_;
};

template <typename Storage, class StringBuffer, typename Encoder, typename Decoder> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const BasicString<StringBuffer, Encoder, Decoder> &field) noexcept {
  
  // This method is noexcept. Do not throw exceptions from any implementation of Encoder or Decoder
  static_assert(noexcept(std::declval<Encoder&>().init(std::declval<utf::UtfCodepointIterator&>())));
  static_assert(noexcept(std::declval<Encoder&>().try_encode(std::declval<uint32_t&>(), std::declval<uint8_t&>())));
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));

  Encoder enc;
  utf::UtfCodepointIterator it(field.data_ptr(), field.data_len());

  auto init_result = enc.init(it);
  if (init_result != BitFieldResult::Ok) {
    return init_result;
  }

  uint32_t code = 0;
  uint8_t bit_len = 0;

  while (enc.try_encode(code, bit_len)) {
    if (bit_len == 0)
      continue;
    auto r = bw.put_bits(code, bit_len);
    if (r != BitFieldResult::Ok) {
      return r;
    }
  }

  return BitFieldResult::Ok;
}

template <typename Storage, class StringBuffer, typename Encoder, typename Decoder> 
SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br, BasicString<StringBuffer, Encoder, Decoder> &out) noexcept {
  using CharType = typename StringBuffer::CharType;
  // This method is noexcept. Do not throw exceptions from any implementation of Encoder or Decoder
  static_assert(noexcept(std::declval<Decoder&>().init()));
  static_assert(noexcept(std::declval<Decoder&>().expected_next_bit_len()));
  static_assert(noexcept(std::declval<Decoder&>().try_decode_byte(0u, 0u, std::declval<char32_t&>())));
  static_assert(noexcept(std::declval<Decoder&>().end_of_sequence()));
  static_assert(noexcept(std::declval<Decoder&>().flush(std::declval<char32_t&>())));
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  static_assert(noexcept(std::declval<BasicString<StringBuffer, Encoder, Decoder>&>().append(std::declval<const CharType*>(), size_t{})));

  out.clear();

  Decoder dec;
  auto init_result = dec.init();
  if (init_result != BitFieldResult::Ok) {
    return init_result;
  }

  char32_t cp = 0;
  CharType buf[utf::UtfMaxUnits<CharType>::value];

  // Take note!!!
  // This method is noexcept. Do not throw exceptions from any implementation of
  // Encoder or Decoder
  BitFieldResult r = BitFieldResult::Ok;
  while (!dec.end_of_sequence()) {
    auto expected_bits = dec.expected_next_bit_len();
    uint8_t get_n_bits = (expected_bits < 32) ? expected_bits : 32;

    // read more bits from stream.
    // Note, that the decode may request zero bits to read as it still has
    // buffered content i wanted to emit. Calling br.get_bits(...) with a length
    // of zero should not have any negative side effects
    uint32_t raw_bits = 0;
    r = br.get_bits(raw_bits, get_n_bits);
    if (r != BitFieldResult::Ok) {
      return r;
    }

    if (dec.try_decode_byte(raw_bits, get_n_bits, cp)) {
      // Break on terminating null
      if (cp == 0) {
        break;
      }

      size_t n = utf::encode_codepoint_to_utf<CharType>(cp, buf);
      // Garbage char, omit
      if (n <= 0)
        continue;

      r = out.append(buf, n);
      if (r != BitFieldResult::Ok) {
        return r;
      }
    }
  }

  if (dec.flush(cp)) {
    size_t n = utf::encode_codepoint_to_utf<CharType>(cp, buf);
    if (n > 0) {
      r = out.append(buf, n);
      if (r != BitFieldResult::Ok) {
        return r;
      }
    }
  }

  return BitFieldResult::Ok;
}

// Standard Bounded / Unbounded string types

#if SUB8_ENABLE_STRING_FIELDS__CHAR

#if SUB8_ENABLE_BOUNDED_BUF
template <size_t MaxLen, typename Encoder, typename Decoder> using BoundedString =
  BasicString<BoundedBasicString<char, MaxLen>, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
template <typename Encoder, typename Decoder> using UnboundedString = BasicString<UnboundedBasicString<char>, Encoder, Decoder>;
#endif
#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR

#if SUB8_ENABLE_BOUNDED_BUF
template <size_t MaxLen, typename Encoder, typename Decoder> using BoundedWString =
  BasicString<BoundedBasicString<wchar_t, MaxLen>, Encoder, Decoder>;
#endif
#if SUB8_ENABLE_UNBOUNDED_BUF
template <typename Encoder, typename Decoder> using UnboundedWString = BasicString<UnboundedBasicString<wchar_t>, Encoder, Decoder>;
#endif
#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR

#if SUB8_ENABLE_BOUNDED_BUF
template <size_t MaxLen, typename Encoder, typename Decoder> using BoundedU8String =
  BasicString<BoundedBasicString<char8_t, MaxLen>, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
template <typename Encoder, typename Decoder> using UnboundedU8String = BasicString<UnboundedBasicString<char8_t>, Encoder, Decoder>;
#endif

#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR

#if SUB8_ENABLE_BOUNDED_BUF
template <size_t MaxLen, typename Encoder, typename Decoder> using BoundedU16String =
  BasicString<BoundedBasicString<char16_t, MaxLen>, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
template <typename Encoder, typename Decoder> using UnboundedU16String = BasicString<UnboundedBasicString<char16_t>, Encoder, Decoder>;
#endif

#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR

#if SUB8_ENABLE_BOUNDED_BUF
template <size_t MaxLen, typename Encoder, typename Decoder> using BoundedU32String =
  BasicString<BoundedBasicString<char32_t, MaxLen>, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
template <typename Encoder, typename Decoder> using UnboundedU32String = BasicString<UnboundedBasicString<char32_t>, Encoder, Decoder>;
#endif

#endif

#if SUB8_ENABLE_FIVE_BIT_STRING

// Hints for LLMs and Debugging
// clang-format off

// | Dec | Binary  | TO   | T1   | T2   | T3                     |    | T0 (GREEK) | T1 (GREEK) |    | T0 (CYRILLIC) | T1 (CYRILLIC) |
// |-----|---------|------|------|------|------------------------|    |------------|------------|    |---------------|---------------|
// | 0   | 0b00000 | a    | A    | 0    | Std Table              |    |     α      |     Α      |    |      а        |      А        |
// | 1   | 0b00001 | b    | B    | 1    | Greek Table            |    |     β      |     Β      |    |      б        |      Б        |
// | 2   | 0b00010 | c    | C    | 2    | Cyrillic Table         |    |     σ      |     Σ      |    |      ц        |      Ц        |
// | 3   | 0b00011 | d    | D    | 3    | reserved               |    |     δ      |     Δ      |    |      д        |      Д        |
// | 4   | 0b00100 | e    | E    | 4    | reserved               |    |     ε      |     Ε      |    |      є        |      Є        |
// | 5   | 0b00101 | f    | F    | 5    | reserved               |    |     φ      |     Φ      |    |      ф        |      Ф        |
// | 6   | 0b00110 | g    | G    | 6    | reserved               |    |     γ      |     Γ      |    |      ґ        |      Ґ        |
// | 7   | 0b00111 | h    | H    | 7    | reserved               |    |     χ      |     Χ      |    |      х        |      Х        |
// | 8   | 0b01000 | i    | I    | 8    | reserved               |    |     ι      |     Ι      |    |      і        |      І        |
// | 9   | 0b01001 | j    | J    | 9    | reserved               |    |     ζ      |     Ζ      |    |      ж        |      Ж        |
// | 10  | 0b01010 | k    | K    | {    | modifier 0             |    |     κ      |     Κ      |    |      к        |      К        |
// | 11  | 0b01011 | l    | L    | }    | modifier 1             |    |     λ      |     Λ      |    |      л        |      Л        |
// | 12  | 0b01100 | m    | M    | [    | modifier 2             |    |     μ      |     Μ      |    |      м        |      М        |
// | 13  | 0b01101 | n    | N    | ]    | modifier 3             |    |     ν      |     Ν      |    |      н        |      Н        |
// | 14  | 0b01110 | o    | O    | :    | modifier 4             |    |     ο      |     Ο      |    |      о        |      О        |
// | 15  | 0b01111 | p    | P    | ,    | modifier 5             |    |     π      |     Π      |    |      п        |      П        |
// | 16  | 0b10000 | q    | Q    | "    | modifier 6             |    |     θ      |     Θ      |    |      к        |      К        |
// | 17  | 0b10001 | r    | R    | .    | modifier 7             |    |     ρ      |     Ρ      |    |      р        |      Р        |
// | 18  | 0b10010 | s    | S    | +    | modifier 8             |    |     ς      |     Σ      |    |      с        |      С        |
// | 19  | 0b10011 | t    | T    | =    | modifier 9             |    |     τ      |     Τ      |    |      т        |      Т        |
// | 20  | 0b10100 | u    | U    | '    | modifier 10            |    |     υ      |     Υ      |    |      у        |      У        |
// | 21  | 0b10101 | v    | V    | /    | modifier 11            |    |     ω      |     Ω      |    |      в        |      В        |
// | 22  | 0b10110 | w    | W    | \\   | modifier 12            |    |     ψ      |     Ψ      |    |      ў        |      Ў        |
// | 23  | 0b10111 | x    | X    | \t   | modifier 13            |    |     ξ      |     Ξ      |    |      х        |      Х        |
// | 24  | 0b11000 | y    | Y    | \n   | modifier 14            |    |     η      |     Η      |    |      ї        |      Ї        |
// | 25  | 0b11001 | z    | Z    | \r   | modifier 15            |    |     ζ      |     Ζ      |    |      з        |      З        |
// | 26  | 0b11010 | ' '  | ' '  | ' '  | modifier 16            |    |    ' '     |    ' '     |    |     ' '       |     ' '       |
// | 27  | 0b11011 |  _   |  _   |  _   | modifier 17            |    |     _      |     _      |    |      _        |      _        |
// | 28  | 0b11100 |  -   |  -   |  -   | Switch Character Set   |    |     -      |     -      |    |      -        |      -        |
// | 29  | 0b11101 | T1   | T0   | T1   | T1                     |    |     T1     |     T0     |    |     T1        |      T0       |
// | 30  | 0b11110 | T2   | T2   | T0   | T2                     |    |     T2     |     T2     |    |     T2        |      T2       |
// | 31  | 0b11111 | T3   | T3   | T3   | T0                     |    |     T3     |     T3     |    |     T3        |      T3       |

// clang-format on
namespace b5 {

struct B5CharAddress final {
  using address_t = uint32_t;

  //
  // Bit layout (LSB = bit 0)
  //
  // [25:22] AVAILABLE TABLE FLAGS   (4 bits)
  // [21:10] AVAILABLE CHARSET FLAGS (12 bits)
  // [ 9: 5] ADDRESS                 (5 bits)
  // [ 4: 0] EXT                     (5 bits)
  //
  // Unused: bits 26–31
  //

  static constexpr address_t EXT_SHIFT = 0;      // 5 bits
  static constexpr address_t ADDR_SHIFT = 5;     // 5 bits
  static constexpr address_t ADDR10_SHIFT = 0;   // 10 bits
  static constexpr address_t CHARSET_SHIFT = 10; // 12 bits
  static constexpr address_t TABLE_SHIFT = 22;   // 4 bits

  static constexpr address_t EXT_MASK = 0b1'1111;             // 5 bits
  static constexpr address_t ADDR_MASK = 0b1'1111;            // 5 bits
  static constexpr address_t ADDR10_MASK = 0b11'1111'1111;    // 10 bits
  static constexpr address_t CHARSET_MASK = 0b1111'1111'1111; // 12 bits
  static constexpr address_t TABLE_MASK = 0b1111;             // 4 bits

  static constexpr uint8_t SWITCH_CHARSET_CODE = 28;
  static constexpr uint8_t MAX_CHARSET_CODE = 10;

  static constexpr uint8_t T1_CODE = 29;
  static constexpr uint8_t T2_CODE = 30;
  static constexpr uint8_t T3_CODE = 31;

  enum ApplicableTables : uint8_t {
    APPLICABLE_TABLES_NONE = 0b0000,
    APPLICABLE_TABLES_T1 = 0b0001,
    APPLICABLE_TABLES_T2 = 0b0010,
    APPLICABLE_TABLES_T3 = 0b0100,
    APPLICABLE_TABLES_T0 = 0b1000, // Equal to the current table
    APPLICABLE_TABLES_ALL = 0b1111,
  };

  enum Table : uint8_t {
    TABLE_T1 = 0,
    TABLE_T2 = 1,
    TABLE_T3 = 2,
    TABLE_T0 = 3, // Equal to the current table
    TABLE_NONE = 255,
  };

  enum ApplicableCharsets : uint16_t {
    APPLICABLE_CHARSET_NONE = 0b0000'0000'0000,
    APPLICABLE_CHARSET_LATIN = 0b0000'0000'0001,
    APPLICABLE_CHARSET_GREEK = 0b0000'0000'0010,
    APPLICABLE_CHARSET_CYRILLIC = 0b0000'0000'0100,
    // reserved bits ...
    APPLICABLE_CHARSET_EXTENDED = (1u << MAX_CHARSET_CODE),
    APPLICABLE_CHARSET_ALL = 0b1111'1111'1111
  };

  enum Charset : uint8_t {
    CHARSET_NONE = 32,
    CHARSET_LATIN = 0,
    CHARSET_GREEK = 1,
    CHARSET_CYRILLIC = 2,
    // reserved ...
    CHARSET_EXTENDED = MAX_CHARSET_CODE
  };

  address_t value{0};

  constexpr B5CharAddress() = default;

  constexpr B5CharAddress(address_t raw_value) : value(raw_value) {}

  constexpr B5CharAddress(uint8_t addr5, uint8_t ext5, uint16_t charset12, uint8_t table4) : value(pack(addr5, ext5, charset12, table4)) {}

  static constexpr address_t pack(uint8_t addr5, uint8_t ext5, uint16_t charset12, uint8_t table4) noexcept {
    return ((address_t(ext5 & EXT_MASK) << EXT_SHIFT) | (address_t(addr5 & ADDR_MASK) << ADDR_SHIFT) |
            (address_t(charset12 & CHARSET_MASK) << CHARSET_SHIFT) | (address_t(table4 & TABLE_MASK) << TABLE_SHIFT));
  }

  static constexpr address_t make(uint8_t addr5, uint8_t ext5, uint16_t charset12, uint8_t table4) noexcept {
    return pack(addr5, ext5, charset12, table4);
  }

  static constexpr address_t make_null_address() noexcept { return pack(0, 0, APPLICABLE_CHARSET_NONE, APPLICABLE_TABLES_NONE); }

  static constexpr address_t make_char_not_found_address() noexcept {
    return pack(T3_CODE, T2_CODE, APPLICABLE_CHARSET_NONE, APPLICABLE_TABLES_NONE);
  }

  static constexpr address_t make_terminating_address() noexcept {
    return pack(T3_CODE, T3_CODE, APPLICABLE_CHARSET_EXTENDED, APPLICABLE_TABLES_ALL);
  }

  static constexpr Charset into_single_charset(ApplicableCharsets charset) noexcept {
    uint16_t cs = static_cast<uint16_t>(charset);
    if (cs == 0)
      return CHARSET_NONE;

    // isolate lowest set bit
    uint16_t lsb = cs & (~cs + 1);

    // find bit index
    uint8_t idx = 0;
    while ((lsb >> idx) != 1u) {
      ++idx;
    }

    return static_cast<Charset>(idx);
  }

  static constexpr ApplicableCharsets into_single_applicable_charset(Charset charset) noexcept {
    if (charset == B5CharAddress::CHARSET_NONE) {
      return B5CharAddress::APPLICABLE_CHARSET_NONE;
    }
    if (charset >= B5CharAddress::CHARSET_EXTENDED) {
      return B5CharAddress::APPLICABLE_CHARSET_EXTENDED;
    }
    return static_cast<ApplicableCharsets>(1u << static_cast<uint8_t>(charset));
  }

  static constexpr ApplicableCharsets into_common_charsets(ApplicableCharsets charset_a, ApplicableCharsets charset_b) noexcept {
    uint16_t csa = static_cast<uint16_t>(charset_a);
    uint16_t csb = static_cast<uint16_t>(charset_b);
    return static_cast<ApplicableCharsets>(csa & csb);
  }

  static constexpr bool has_common_charsets(ApplicableCharsets charset_a, ApplicableCharsets charset_b) noexcept {
    return into_common_charsets(charset_a, charset_b) != APPLICABLE_CHARSET_NONE;
  }

  static constexpr Table into_single_table(ApplicableTables table) noexcept {
    uint8_t t = static_cast<uint8_t>(table);
    if (t == 0)
      return TABLE_NONE;

    uint8_t lsb = t & (~t + 1);

    switch (lsb) {
    case APPLICABLE_TABLES_T0:
      return TABLE_T0;
    case APPLICABLE_TABLES_T1:
      return TABLE_T1;
    case APPLICABLE_TABLES_T2:
      return TABLE_T2;
    case APPLICABLE_TABLES_T3:
      return TABLE_T3;
    default:
      return TABLE_NONE; // invalid mask
    }
  }

  static constexpr ApplicableTables into_single_applicable_table(Table table) noexcept {
    if (table > B5CharAddress::TABLE_T0) {
      return B5CharAddress::APPLICABLE_TABLES_NONE;
    }
    return static_cast<ApplicableTables>(1u << static_cast<uint8_t>(table));
  }

  static constexpr ApplicableTables into_common_tables(ApplicableTables table_a, ApplicableTables table_b) noexcept {
    uint8_t ta = static_cast<uint8_t>(table_a);
    uint8_t tb = static_cast<uint8_t>(table_b);
    return static_cast<ApplicableTables>(ta & tb);
  }

  static constexpr bool has_common_tables(ApplicableTables table_a, ApplicableTables table_b) noexcept {
    return into_common_tables(table_a, table_b) != APPLICABLE_TABLES_NONE;
  }

  // Address fields

  constexpr uint8_t char_code_ext() const noexcept { return static_cast<uint8_t>((value >> EXT_SHIFT) & EXT_MASK); }

  constexpr uint8_t char_code() const noexcept { return static_cast<uint8_t>((value >> ADDR_SHIFT) & ADDR_MASK); }

  constexpr uint16_t char_code_10_bit() const noexcept { return static_cast<uint16_t>((value >> ADDR10_SHIFT) & ADDR10_MASK); }

  constexpr ApplicableCharsets applicable_charsets() const noexcept {
    return ApplicableCharsets(static_cast<uint16_t>((value >> CHARSET_SHIFT) & CHARSET_MASK));
  }

  constexpr ApplicableTables applicable_tables() const noexcept {
    return ApplicableTables(static_cast<uint8_t>((value >> TABLE_SHIFT) & TABLE_MASK));
  }

  constexpr size_t table_index() const noexcept { return (static_cast<size_t>(into_single_table(applicable_tables())) + 1) % 4; }

  constexpr bool is_available_on_table(Table single_table_scope) const noexcept {
    auto applicable_table = into_single_applicable_table(single_table_scope);
    return into_common_tables(applicable_table, applicable_tables()) != APPLICABLE_TABLES_NONE;
  }

  constexpr Table pick_best_table(ApplicableTables table_scope) const noexcept {
    // Picks the best char set based on the past and previous
    // ApplicableCharsetss
    auto t1 = applicable_tables();
    auto t2 = into_common_tables(t1, table_scope);
    if (t2 == APPLICABLE_TABLES_NONE) {
      return into_single_table(t1);
    }
    return into_single_table(t2);
  }

  constexpr Charset charset() const noexcept { return into_single_charset(applicable_charsets()); }

  constexpr Charset pick_common_charset(ApplicableCharsets charset_scope) const noexcept {
    // Picks the best char set based on the past and previous
    // ApplicableCharsetss
    auto cs1 = applicable_charsets();
    auto cs2 = into_common_charsets(cs1, charset_scope);
    return into_single_charset(cs2);
  }

  constexpr bool is_available_in_charset(Charset c) const noexcept {
    return into_common_charsets(applicable_charsets(), into_single_applicable_charset(c)) != APPLICABLE_CHARSET_NONE;
  }

  constexpr bool is_extended_char() const noexcept {
    return into_common_charsets(applicable_charsets(), APPLICABLE_CHARSET_EXTENDED) != APPLICABLE_CHARSET_NONE;
  }

  constexpr bool is_null() const noexcept { return char_code_10_bit() == 0 && applicable_tables() == APPLICABLE_TABLES_NONE; }

  constexpr bool is_terminator() const noexcept { return char_code_10_bit() == 1023; }

  constexpr bool is_not_found() const noexcept { return char_code_10_bit() == 1022; }

  constexpr bool operator==(const B5CharAddress &other) const noexcept { return value == other.value; }

  constexpr bool operator!=(const B5CharAddress &other) const noexcept { return value != other.value; }
};

} // namespace b5

namespace b5::ser::lut {

#define B5_LATIN static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_LATIN)
#define B5_GREEK static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_GREEK)
#define B5_CYRILLIC static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_CYRILLIC)
#define B5_EXTENDED static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_EXTENDED)

#define B5_T0 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T0)
#define B5_T1 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T1)
#define B5_T2 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T2)
#define B5_T3 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T3)

struct B5EncoderLutNode final {
  char32_t ch;
  B5CharAddress addr;
};

// insert generated lut in sub8.cpp
extern const B5EncoderLutNode B5EncoderLut[];
extern const size_t B5EncoderLutSize;

inline B5CharAddress into_b5_char_address(char32_t utf32_in) noexcept {
  size_t left = 0;
  size_t right = B5EncoderLutSize;

  while (left < right) {
    const size_t mid = left + (right - left) / 2;
    const char32_t mid_ch = B5EncoderLut[mid].ch;

    if (utf32_in < mid_ch)
      right = mid;
    else if (utf32_in > mid_ch)
      left = mid + 1;
    else
      return B5EncoderLut[mid].addr;
  }
  return B5CharAddress::make_char_not_found_address();
}

inline uint8_t into_table_control_code(B5CharAddress::Table target_table,
  B5CharAddress::Table current_table = B5CharAddress::TABLE_NONE) noexcept {
  if (target_table == B5CharAddress::TABLE_NONE) {
    return B5CharAddress::T1_CODE + static_cast<uint8_t>(current_table);
  }

  if (target_table == B5CharAddress::TABLE_T0) {
    // Switching back to T0 is indicated by the current table control char
    target_table = current_table;
  }
  return B5CharAddress::T1_CODE + static_cast<uint8_t>(target_table);
}

inline uint8_t into_charset_control_code(B5CharAddress::Charset target_charset) noexcept { return static_cast<uint8_t>(target_charset); }
} // namespace b5::ser::lut

namespace utf {
template <> struct UtfCodepointMapper<b5::B5CharAddress> {
  using value_type = b5::B5CharAddress;

  static value_type map(char32_t cp) noexcept { return b5::ser::lut::into_b5_char_address(cp); }
};

} // namespace utf

namespace b5::ser {

using B5LookAheadIterator = utf::UtfCodepointLookAheadIteratorT<utf::UtfCodepointMapper<B5CharAddress>, 2>;

class B5CodeSequenceEncoder {
public:
  B5CodeSequenceEncoder() noexcept = default;

  explicit B5CodeSequenceEncoder(utf::UtfCodepointIterator iter, bool emit_terminating_zero = false,
    bool starting_in_multi_code_state = false, B5CharAddress::Charset starting_charset = B5CharAddress::CHARSET_LATIN,
    B5CharAddress::Table starting_table = B5CharAddress::TABLE_T0) noexcept
      : iter_(B5LookAheadIterator(iter)), previous_table(starting_table), previous_charset_(starting_charset),
        previous_multi_code_state_(starting_in_multi_code_state), starting_table_(starting_table), starting_charset_(starting_charset),
        starting_multi_code_state_(starting_in_multi_code_state), emit_terminating_zero_(emit_terminating_zero) {}

  size_t try_encode_next(uint8_t *buffer, size_t len) noexcept {

    // catch case where string is empty
    if (emit_terminating_zero_ && iter_.empty()) {
      if(terminator_emitted_) {
        return 0;
      }
      terminator_emitted_ = true;
      return emit_terminating_null_control_char(buffer, len);
    }

    size_t seq_len = 0;
    B5CharAddress head = B5CharAddress::make_null_address();
    B5CharAddress look_ahead_1 = B5CharAddress::make_null_address();

    if (!iter_.try_pop(head)) {
      return seq_len;
    }

    bool end_of_string = !iter_.try_peek_ahead(0, look_ahead_1);

    // -----------------------------------------------------------------
    // 0) Char not found
    // -----------------------------------------------------------------
    if (head.is_not_found()) {
      seq_len += emit_not_found_char(buffer, len);
      return seq_len;
    }

    // -----------------------------------------------------------------
    // 1) Select Table
    // -----------------------------------------------------------------

    // if the char is not available on the previously selected table
    if (!head.is_available_on_table(previous_table)) {
      // Try pick a table which will fit the next two chars
      auto target_table = head.pick_best_table(look_ahead_1.applicable_tables());
      if (target_table != B5CharAddress::TABLE_NONE) {
        // Emit code(s) to switch table
        seq_len += emit_table_control_char(target_table, buffer, len);
        previous_table = target_table;
      }
    }

    // -----------------------------------------------------------------
    // 2) Multi-code (10-bit) mode, controlled by T3 + T1 / exit logic
    // -----------------------------------------------------------------
    if (!previous_multi_code_state_) {
      // Enter 10bit mode if next two char are in the extended character sets
      if (head.is_extended_char() && (!look_ahead_1.is_null() && look_ahead_1.is_extended_char())) {
        // emit chars to go to multi byte mode
        seq_len += emit_enter_multi_byte_mode_control_char(buffer, len);
        previous_multi_code_state_ = true;
      }
    } else {
      if (!head.is_extended_char() && (!look_ahead_1.is_null() && !look_ahead_1.is_extended_char())) {
        // emit chars to go to exit multi byte mode
        seq_len += emit_exit_multi_byte_mode_control_char(buffer, len);
        previous_multi_code_state_ = false;
      }
    }

    // -----------------------------------------------------------------
    // 3) Charset selection / shifting (only in 5-bit mode)
    // -----------------------------------------------------------------

    // if not in 10bit mode, then extend char are shifted
    bool is_15bit_extended_char = head.is_extended_char() && !previous_multi_code_state_;

    if (!previous_multi_code_state_ && !is_15bit_extended_char && !head.is_available_in_charset(previous_charset_)) {
      // Check to see if the next char is also in the new character set. If it
      // is, the lock to new character set, otherwise shift only
      auto common_charset = head.pick_common_charset(look_ahead_1.applicable_charsets());

      if (common_charset != B5CharAddress::CHARSET_NONE) {
        // Emit char set lock
        seq_len += emit_switch_char_set_control_char(common_charset, buffer, len);
        // switch char set
        previous_charset_ = common_charset;
      } else {
        // Emit char set shift
        is_15bit_extended_char |= true;
      }
    }

    // -----------------------------------------------------------------
    // 4) Emit primary code
    // -----------------------------------------------------------------
    if (!head.is_null()) {
      seq_len += emit_code(head.char_code(), buffer, len);
    }

    // -----------------------------------------------------------------
    // 5) Emit extended shift control for 15bit chars
    // -----------------------------------------------------------------
    if (is_15bit_extended_char) {
      seq_len += emit_extended_shift_control_char(buffer, len);
    }

    // -----------------------------------------------------------------
    // 6) Emit extended code for 10 and 15bit chars
    // -----------------------------------------------------------------
    if (previous_multi_code_state_ || is_15bit_extended_char) {
      seq_len += emit_code(head.char_code_ext(), buffer, len);
    }

    // -----------------------------------------------------------------
    // 7) Emit terminating code at end of stream if enabled
    // -----------------------------------------------------------------
    if (emit_terminating_zero_ && end_of_string) {
      terminator_emitted_ = true;
      seq_len += emit_terminating_null_control_char(buffer, len);
    }

    return seq_len;
  }

  bool empty() const noexcept { return iter_.empty(); }

  void reset() noexcept {
    iter_.reset();
    previous_table = starting_table_;
    previous_charset_ = starting_charset_;
    previous_multi_code_state_ = starting_multi_code_state_;
    terminator_emitted_ = false;
  }

private:
  size_t emit_table_control_char(B5CharAddress::Table table, uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> {Tx}
    uint8_t code = lut::into_table_control_code(table, previous_table);
    emit_code(code, buffer, len);
    return 1;
  }

  size_t emit_not_found_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> T3 | T2
    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T3), buffer, len);
    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T2), buffer, len);
    return 2;
  }

  size_t emit_enter_multi_byte_mode_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> T3 | T1

    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T3), buffer, len);
    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T1), buffer, len);
    return 2;
  }

  size_t emit_exit_multi_byte_mode_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> T3
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    emit_code(t3_code, buffer, len);
    return 1;
  }

  size_t emit_extended_shift_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // 15 bit char code
    // EMIT -> {code} | T3 | {ext code}
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    emit_code(t3_code, buffer, len);
    return 1;
  }

  size_t emit_switch_char_set_control_char(B5CharAddress::Charset charset, uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> 28 | T3 | {char set code}
    uint8_t change_char_set_code = 28;
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    uint8_t char_set_ext_code = lut::into_charset_control_code(charset);

    emit_code(change_char_set_code, buffer, len);
    emit_code(t3_code, buffer, len);
    emit_code(char_set_ext_code, buffer, len);
    return 3;
  }

  size_t emit_terminating_null_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT ->  T3 | T3
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    emit_code(t3_code, buffer, len);
    emit_code(t3_code, buffer, len);
    return 2;
  }

  size_t emit_code(uint8_t code, uint8_t *&buffer, size_t &len) noexcept {
    if (len == 0) {
      return 1;
    }
    buffer[0] = code & 0x1Fu;

    buffer++;
    len--;
    return 1;
  }

  B5LookAheadIterator iter_{};

  B5CharAddress::Table previous_table{B5CharAddress::TABLE_T0};
  B5CharAddress::Charset previous_charset_{B5CharAddress::CHARSET_LATIN};
  bool previous_multi_code_state_{false};

  B5CharAddress::Table starting_table_{B5CharAddress::TABLE_T0};
  B5CharAddress::Charset starting_charset_{B5CharAddress::CHARSET_LATIN};

  bool starting_multi_code_state_{false};
  bool emit_terminating_zero_{false};

  bool terminator_emitted_{false};
};
} // namespace b5::ser

namespace b5::de {

namespace lut {
struct B5DecoderExtEntry final {
  uint16_t code10;
  char32_t ch;
};

inline constexpr std::size_t B5_NUM_TABLES = 3;
inline constexpr std::size_t B5_NUM_CODES = 32;
extern const size_t B5_NUM_CHARSETS;
extern const char32_t B5DecoderLut[][B5_NUM_TABLES][B5_NUM_CODES];

extern const B5DecoderExtEntry *B5DecoderLutExt[];

extern const size_t B5DecoderLutExtCount[];

inline char32_t get_extended_char_from_b5_char_address(const B5CharAddress b5) noexcept {
  if (b5.is_terminator() || b5.is_null()) {
    return U'\0';
  }

  if (b5.is_not_found()) {
    return U'\uFFFD'; // � not found
  }

  const size_t table = b5.table_index();
  const uint16_t key = b5.char_code_10_bit();

  const auto *lut = B5DecoderLutExt[table];
  const auto count = B5DecoderLutExtCount[table];

  size_t left = 0;
  size_t right = count;
  while (left < right) {
    const size_t mid = left + ((right - left) / 2);
    const auto mid_key = lut[mid].code10;

    if (key < mid_key) {
      right = mid;
    } else if (key > mid_key) {
      left = mid + 1;
    } else {
      return lut[mid].ch;
    }
  }

  return U'\uFFFD'; // � not found
}

inline char32_t get_char_from_b5_char_address(b5::B5CharAddress b5) noexcept {
  if (b5.is_terminator() || b5.is_null()) {
    return U'\0';
  }

  if (b5.is_not_found()) {
    return U'\uFFFD'; // � not found
  }

  if (b5.is_extended_char()) {
    return get_extended_char_from_b5_char_address(b5);
  }

  const size_t cs_idx = static_cast<size_t>(b5.charset());
  const size_t code = static_cast<size_t>(b5.char_code());

  size_t tbl_idx = b5.table_index();

  if (cs_idx >= B5_NUM_CHARSETS || tbl_idx >= B5_NUM_TABLES || code >= B5_NUM_CODES) {
    return U'\uFFFD'; // �
  }
  return B5DecoderLut[cs_idx][tbl_idx][code];
}

} // namespace lut

inline B5CharAddress::ApplicableCharsets code_into_charset(uint8_t code) noexcept {
  if (code <= B5CharAddress::MAX_CHARSET_CODE) {
    return static_cast<B5CharAddress::ApplicableCharsets>(1u << code);
  }
  return B5CharAddress::APPLICABLE_CHARSET_EXTENDED;
}

inline uint16_t into_charset_bit_flags(B5CharAddress::Charset cs) noexcept {
  return static_cast<uint16_t>(code_into_charset(static_cast<uint8_t>(cs)));
}

inline uint8_t into_table_bit_flags(B5CharAddress::Table table) noexcept {
  return static_cast<uint8_t>(B5CharAddress::into_single_applicable_table(table));
}

inline uint8_t into_table_bit_flags(uint8_t code, uint8_t current_table) noexcept {
  if (code < B5CharAddress::T1_CODE || code > B5CharAddress::T3_CODE) {
    return static_cast<uint8_t>(B5CharAddress::APPLICABLE_TABLES_NONE);
  }
  uint8_t table = into_table_bit_flags(static_cast<B5CharAddress::Table>(code - B5CharAddress::T1_CODE));
  if (table == current_table) {
    return static_cast<uint8_t>(B5CharAddress::APPLICABLE_TABLES_T0);
  }
  return table;
}

constexpr bool is_control_code(uint8_t code) noexcept { return code >= B5CharAddress::T1_CODE && code < (B5CharAddress::T3_CODE + 1); }

class B5CodeSequenceBuffer {
  static constexpr uint8_t MAX_CODE_LEN = 3;

public:
  B5CodeSequenceBuffer() noexcept = default;

  void enqueue_buffer(uint8_t code_in) noexcept {
    buf_[(idx_ + len_) % MAX_CODE_LEN] = code_in;
    if (len_ < MAX_CODE_LEN) {
      ++len_;
    } else {
      idx_ = (idx_ + 1) % MAX_CODE_LEN;
    }
  }

  void advance_buffer(uint8_t distance = 1) noexcept {
    if (distance >= len_) {
      idx_ = 0;
      len_ = 0;
      return;
    }

    idx_ = (idx_ + distance) % MAX_CODE_LEN;
    len_ -= distance;
  }

  uint8_t at(uint8_t i) const noexcept { return buf_[(idx_ + i) % MAX_CODE_LEN]; }

  uint8_t size() const noexcept { return len_; }
  bool empty() const noexcept { return len_ == 0; }
  bool full() const noexcept { return len_ == MAX_CODE_LEN; }

private:
  uint8_t idx_ = 0;
  uint8_t len_ = 0;
  uint8_t buf_[MAX_CODE_LEN]{}; // circular storage
};

class B5CodeSequenceDecoder {
public:
  B5CodeSequenceDecoder() noexcept = default;

  explicit B5CodeSequenceDecoder(bool starting_in_multi_code_state = false,
    B5CharAddress::Charset starting_charset = B5CharAddress::CHARSET_LATIN,
    B5CharAddress::Table starting_table = B5CharAddress::TABLE_T0) noexcept
      : current_charset12_(into_charset_bit_flags(starting_charset)), current_table4_(into_table_bit_flags(starting_table)),
        current_multi_code_state_(starting_in_multi_code_state) {}

  bool try_decode_next_address(B5CodeSequenceBuffer &buff, B5CharAddress &b5_out, bool end_of_stream) noexcept {
    const uint8_t size = buff.size();

    if (size < 2 && !end_of_stream) {
      return false;
    }

    if (size == 0) {
      return false;
    }

    const uint8_t a = buff.at(0);
    const uint8_t b = (size >= 2) ? buff.at(1) : 128;
    const uint8_t c = (size >= 3) ? buff.at(2) : 128;

    // ------------------------------------------------------------
    // 1) Handle control codes
    // ------------------------------------------------------------
    if (is_control_code(a)) {
      if (a == B5CharAddress::T3_CODE) {
        if (size < 2) {
          return false;
        }
        // Terminating null T3 + T3
        if (b == B5CharAddress::T3_CODE) {
          b5_out = B5CharAddress::make_terminating_address();
          buff.advance_buffer(2);
          return true;
        }

        // char not found T3 + T2
        if (b == B5CharAddress::T2_CODE) {
          b5_out = B5CharAddress::make_char_not_found_address();
          buff.advance_buffer(2);
          return true;
        }

        // Enter multi-code (10-bit) mode: T3 + T1
        if (!current_multi_code_state_ && b == B5CharAddress::T1_CODE) {
          current_multi_code_state_ = true;
          buff.advance_buffer(2);
          return false;
        }

        // Exit multi-code mode: T3 + ...
        if (current_multi_code_state_) {
          current_multi_code_state_ = false;
          buff.advance_buffer(1);
          return false;
        }
      } else {
        // Table switch: T{n}
        current_table4_ = into_table_bit_flags(a, current_table4_);
        buff.advance_buffer(1);
        return false;
      }
    }

    if (size == 1 && end_of_stream) {
      buff.advance_buffer(1);
      return decode_single(a, b5_out);
    }

    // ------------------------------------------------------------
    // 2) 10-bit encoding mode (multi-code mode)
    // ------------------------------------------------------------
    if (current_multi_code_state_) {
      if (size < 2) {
        return false;
      }
      buff.advance_buffer(2);
      return decode_double(a, b, b5_out);
    }

    // ------------------------------------------------------------
    // 3) 15-bit shift encoding:
    //    {code} | T3 | {ext code}
    //    or Charset switch: 28 | T3 | {charset code}
    // ------------------------------------------------------------
    if (b == B5CharAddress::T3_CODE) {
      if (size < 3) {
        return false;
      }
      if (!is_control_code(c)) {
        buff.advance_buffer(3);
        return decode_double(a, c, b5_out);
      }
    }

    // ------------------------------------------------------------
    // 4) 5-bit encoding: {code}
    // ------------------------------------------------------------
    buff.advance_buffer(1);
    return decode_single(a, b5_out);
  }

private:
  // Decode a single 5-bit code into a B5CharAddress
  bool decode_single(uint8_t a, B5CharAddress &b5_out) noexcept {
    b5_out = B5CharAddress(a, 0, current_charset12_, current_table4_);
    return true;
  }

  // Decode a double-code sequence (10-bit or 15-bit shift)
  bool decode_double(uint8_t a, uint8_t b, B5CharAddress &b5_out) noexcept {
    auto char_set = code_into_charset(b);

    // Charset switch: #28 | T3 | {charset code}
    if (a == B5CharAddress::SWITCH_CHARSET_CODE && char_set != B5CharAddress::ApplicableCharsets::APPLICABLE_CHARSET_EXTENDED) {
      current_charset12_ = static_cast<uint16_t>(char_set);
      return false;
    }

    // Extend char set char: {code} | T3 | {ext code} or {code} | {ext code}
    b5_out = B5CharAddress(a, b, static_cast<uint16_t>(char_set), current_table4_);
    return true;
  }

private:
  uint16_t current_charset12_{0};
  uint8_t current_table4_{0};
  bool current_multi_code_state_{false};
};

}; // namespace b5::de

// Encoder
template <
  // MaxSymbols: Max encoded symbol length of path.
  //
  // When `TerminatedSequence == false`:
  //   The string is prefixed with the length, value which is optimized
  //   to be the shortest possible encoded length. Ie a max length of 15 will
  //   require an additional 4bits to encode the length. The size penalty will
  //   increase for each n^2 increment in length This process requires the
  //   string to be pre-passed to calculate the the actual encoded length.
  //
  // When `TerminatedSequence == true`:
  //   The string is terminated with a 10bit termination code. This will use
  //   more bits for any string under a 1,024 length. However the a null
  //   terminated sequence does not require pre-pass step to calculate the
  //   full length.
  //
  size_t MaxSymbols = 255,

  // TerminatedSequence: Terminates sequence with 10bit null char
  // If enabled, string length is not prefixed and instead a terminating null
  // is encoded.
  bool TerminatedSequence = false,

  // StartingMultiCodeState: Indicate if the encoder should start in 10bit
  // multi code or 5bit mode.
  bool StartingMultiCodeState = false,

  // StartingCharset: Indicate the starting character set
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,

  // StartingCharset: Indicate the starting table set (ie lower case / upper
  // case / numeric)
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
class B5Encoder {
  b5::ser::B5CodeSequenceEncoder iter_{};

  // Total number of 5-bit symbols (controls + data) that will follow the header
  size_t total_symbol_length_ = 0;
  size_t remaining_header_bits_ = 0;

  static constexpr size_t ENCODED_CHAR_BUFFER_LEN = 6;
  uint8_t code_buffer_[ENCODED_CHAR_BUFFER_LEN]{};
  size_t code_buffer_size_ = 0;
  size_t code_buffer_idx_ = 0;

  static constexpr uint8_t SymbolBits = 5; // 5-bit codes
  static constexpr uint8_t LengthPrefixBitWidth = MaxSymbols <= 1u ? 1u : limits::bitwidth_to_express_max_value(MaxSymbols);

public:
  static constexpr BitSize MaxPossibleSize = []() constexpr noexcept -> BitSize {
    BitSize len;
    if (!TerminatedSequence) {
      len.add_bits(LengthPrefixBitWidth);
    }
    return len + (BitSize::from_bits(SymbolBits) * MaxSymbols);
  }();

  static constexpr BitSize MinPossibleSize = []() constexpr noexcept -> BitSize {
    BitSize len;
    if (!TerminatedSequence) {
      len.add_bits(LengthPrefixBitWidth);
    } else {

      len += BitSize::from_bits(SymbolBits) * 2; // length of terminating control codes
    }
    return len;
  }();

  static BitSize actual_size(utf::UtfCodepointIterator utf_iter) noexcept {
    if constexpr (TerminatedSequence) {
      return BitSize::from_bits(SymbolBits) * (calc_encoded_length(utf_iter));
    } else {
      return (BitSize::from_bits(SymbolBits) * calc_encoded_length(utf_iter)) + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }

  BitFieldResult init(utf::UtfCodepointIterator utf_iter) noexcept {
    iter_ = b5::ser::B5CodeSequenceEncoder(utf_iter, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable);
    if constexpr (TerminatedSequence) {
      remaining_header_bits_ = 0;
      total_symbol_length_ = MaxSymbols;
    } else {
      total_symbol_length_ = calc_encoded_length(utf_iter);
      remaining_header_bits_ = LengthPrefixBitWidth;
      if (total_symbol_length_ > MaxSymbols) {
        return BitFieldResult::ErrorValueOverflow;
      }
    }
    return BitFieldResult::Ok;
  }

  bool try_encode(uint32_t &out, uint8_t &bit_len) noexcept {
    // 1) Emit header once
    if (remaining_header_bits_ != 0) {
      out = static_cast<uint32_t>(total_symbol_length_);
      bit_len = remaining_header_bits_;
      remaining_header_bits_ = 0;
      return true;
    }

    // 2) If we still have pending 5-bit symbols, emit them first
    if (code_buffer_idx_ < code_buffer_size_) {
      out = static_cast<uint32_t>(code_buffer_[code_buffer_idx_++]);
      bit_len = SymbolBits;
      return true;
    }

    code_buffer_idx_ = 0;
    code_buffer_size_ = 0;

    while (!iter_.empty()) {
      uint8_t *buffer = code_buffer_ + code_buffer_idx_;
      size_t buffer_len = ENCODED_CHAR_BUFFER_LEN - code_buffer_idx_;
      code_buffer_size_ = iter_.try_encode_next(buffer, buffer_len) + code_buffer_idx_;

      if (code_buffer_size_ != 0) {
        out = static_cast<uint32_t>(code_buffer_[code_buffer_idx_++]);
        bit_len = SymbolBits;
        return true;
      }
    }

    // emit terminating null 
    code_buffer_size_ = iter_.try_encode_next(code_buffer_, ENCODED_CHAR_BUFFER_LEN);

    if (code_buffer_size_ != 0) {
      out = static_cast<uint32_t>(code_buffer_[code_buffer_idx_++]);
      bit_len = SymbolBits;
      return true;
    }

    return false;
  }

private:
  static size_t calc_encoded_length(utf::UtfCodepointIterator &utf_iter) noexcept {

    auto iter = b5::ser::B5CodeSequenceEncoder(utf_iter, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable);
    size_t len = 0;
    uint8_t dummy_buffer = 0;
    iter.reset();
    while (!iter.empty()) {
      len += iter.try_encode_next(&dummy_buffer, 0);
    }

    // terminating null
    return len + iter.try_encode_next(&dummy_buffer, 0);
  }
};

// Decoder
template <
  // MaxSymbols: Max encoded symbol length of path.
  //
  // When `TerminatedSequence == false`:
  //   The string is prefixed with the length, value which is optimized
  //   to be the shortest possible encoded length. Ie a max length of 15 will
  //   require an additional 4bits to encode the length. The size penalty will
  //   increase for each n^2 increment in length This process requires the
  //   string to be pre-passed to calculate the the actual encoded length.
  //
  // When `TerminatedSequence == true`:
  //   The string is terminated with a 10bit termination code. This will use
  //   more bits for any string under a 1,024 length. However the a null
  //   terminated sequence does not require pre-pass step to calculate the
  //   full length.
  //
  size_t MaxSymbols = 255,

  // TerminatedSequence: Terminates sequence with 10bit null char
  // If enabled, string length is not prefixed and instead a terminating null
  // is encoded.
  bool TerminatedSequence = false,

  // StartingMultiCodeState: Indicate if the encoder should start in 10bit
  // multi code or 5bit mode.
  bool StartingMultiCodeState = false,

  // StartingCharset: Indicate the starting character set
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,

  // StartingCharset: Indicate the starting table set (ie lower case / upper
  // case / numeric)
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
class B5Decoder {
  bool pending_read_header_ = !TerminatedSequence;
  bool waiting_for_more_symbols_ = false;
  size_t symbols_remaining_ = MaxSymbols;

  b5::de::B5CodeSequenceBuffer buff_;
  b5::de::B5CodeSequenceDecoder seq_decoder_{StartingMultiCodeState, StartingCharset, StartingTable};

  static constexpr uint8_t LengthPrefixBitWidth = []() constexpr noexcept -> uint8_t {
    size_t v = MaxSymbols;
    uint8_t bits = 0;
    while (v > 0) {
      v >>= 1;
      ++bits;
    }
    return bits ? bits : uint8_t{1};
  }();

  static constexpr uint8_t SymbolBits = 5; // 5-bit codes

public:
  BitFieldResult init() noexcept { return BitFieldResult::Ok; }

  bool end_of_sequence() const noexcept { return symbols_remaining_ == 0 && buff_.empty(); }

  uint8_t expected_next_bit_len() const noexcept {
    if (pending_read_header_) {
      return LengthPrefixBitWidth;
    }

    if (buff_.full() || !waiting_for_more_symbols_) {
      return 0;
    }

    if (symbols_remaining_ != 0) {
      return SymbolBits;
    }

    return 0;
  }

  bool try_decode_byte(uint32_t code_in, uint8_t code_bit_len, char32_t &utf32_out) noexcept {
    if (code_bit_len != expected_next_bit_len()) {
      // unexpected bit width
      return false;
    }

    // 1) Read header if present
    if (pending_read_header_) {
      symbols_remaining_ = static_cast<size_t>(code_in);
      if (symbols_remaining_ > MaxSymbols) {
        symbols_remaining_ = MaxSymbols;
      }
      pending_read_header_ = false;
      return false;
    }

    // 2) enqueue code
    if (code_bit_len != 0) {
      buff_.enqueue_buffer(static_cast<uint8_t>(code_in & 0x1Fu));
      symbols_remaining_ = symbols_remaining_ == 0 ? 0 : symbols_remaining_ - 1;
    }

    // 3) try decode
    b5::B5CharAddress b5_out;
    if (seq_decoder_.try_decode_next_address(buff_, b5_out, symbols_remaining_ == 0)) {
      waiting_for_more_symbols_ = false;
      if (b5_out.is_terminator()) {
        symbols_remaining_ = 0;
      }
      utf32_out = b5::de::lut::get_char_from_b5_char_address(b5_out);
      return true;
    }
    waiting_for_more_symbols_ = true;
    return false;
  }

  bool flush(char32_t &) noexcept { return false; }
};

// Standard B5/B10 string types
// -----------------------------

#if SUB8_ENABLE_STRING_FIELDS__CHAR

// B5
template <size_t MaxLength, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB5String =
  BoundedString<MaxLength, B5Encoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB5String = UnboundedString<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
  B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

// B5 Null terminated string
template <size_t MaxLength, bool StartingMultiCodeState = false, b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB5StringNullTerminated =
  BoundedString<MaxLength, B5Encoder<MaxLength, true, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, true, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB5StringNullTerminated = UnboundedString<B5Encoder<MaxSymbols, true, StartingMultiCodeState, StartingCharset, StartingTable>,
  B5Decoder<MaxSymbols, true, StartingMultiCodeState, StartingCharset, StartingTable>>;

// B10

template <size_t MaxLength, bool TerminatedSequence = false, bool StartingMultiCodeState = true,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB10String =
  BoundedString<MaxLength, B5Encoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool TerminatedSequence = false, bool StartingMultiCodeState = true,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB10String =
  UnboundedString<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
template <size_t MaxLength, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB5WString =
  BoundedWString<MaxLength, B5Encoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB5WString =
  UnboundedWString<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR

template <size_t MaxLength, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB5U8String =
  BoundedU8String<MaxLength, B5Encoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB5U8String =
  UnboundedU8String<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR

template <size_t MaxLength, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB5U16String =
  BoundedU16String<MaxLength, B5Encoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB5U16String =
  UnboundedU16String<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR

template <size_t MaxLength, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using BoundedB532String =
  BoundedU32String<MaxLength, B5Encoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxLength, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template <size_t MaxSymbols, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
  b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
  b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using UnboundedB5U32String =
  UnboundedU32String<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
    B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#endif // SUB8_ENABLE_FIVE_BIT_STRING
} // namespace sub8
#endif // SUB8_ENABLE_STRING_FIELDS

// ============================================================
// END INLINE ./sub8_strings.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_primitives.h
// ============================================================

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

// ============================================================
// END INLINE ./sub8_primitives.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_floats.h
// ============================================================

#pragma once

// Enable: Fixed Length Float Fields
// Fields with an arbitrary fraction and exponent value which can be use to
// define non standard IEEE-754 types such as half (16bits, with 5bit exponent
// at 10bit fraction) or bfloat16 (16bits, with 8bit exponent at 7bit fraction)
#ifndef SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#define SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS 1
#endif


// Fixed Length Float
// ----------------------
#if SUB8_ENABLE_FIXED_LENGTH_FLOAT_FIELDS
#include <cstdint> // uintx_t
#include <cstddef> // size_t
#include <utility> // std::declval
#include <cstring> // memcpy
#include <cmath>   // std::isnan, std::isinf, std::signbit
#include <limits>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_api.h"
#include "sub8_io.h"
#include "sub8_type_information.h"
#endif

namespace sub8 {

namespace details {
namespace fpbits {
// IEEE-754 floating point implementation
//  !!! ->  Predominantly AI generated code section, see test cases for
//  validation
template <class To, class From> static inline To bit_cast_memcpy(const From &src) noexcept {
  static_assert(sizeof(To) == sizeof(From));
  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

template <typename Float> struct ieee;
template <> struct ieee<float> {
  using UInt = uint32_t;
  static constexpr int total_bits = 32;
  static constexpr int exp_bits = 8;
  static constexpr int frac_bits = 23;
  static constexpr int bias = 127;
};
template <> struct ieee<double> {
  using UInt = uint64_t;
  static constexpr int total_bits = 64;
  static constexpr int exp_bits = 11;
  static constexpr int frac_bits = 52;
  static constexpr int bias = 1023;
};

template <typename U> static constexpr U mask_n(int n) noexcept {
  if (n <= 0)
    return U{0};
  if (n >= int(sizeof(U) * 8))
    return ~U{0};
  return (U{1} << n) - U{1};
}

// Round-right-shift with IEEE RNE (round-to-nearest, ties-to-even)
template <typename U> static inline U rshift_rne(U v, int sh) noexcept {
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

template <typename U> static inline U map_nan_payload(U s_frac, int S_FRAC_BITS, int D_FRAC_BITS) noexcept {
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
template <typename DU, typename SU> static inline DU convert_bits_ieee_like(SU sign, SU exp, SU frac, int S_EXP_BITS, int S_FRAC_BITS,
  int S_BIAS, int D_EXP_BITS, int D_FRAC_BITS, int D_BIAS) noexcept {
  const SU S_EXP_MAX = mask_n<SU>(S_EXP_BITS);
  const DU D_EXP_MAX = mask_n<DU>(D_EXP_BITS);

  const DU signD = DU(sign) << (D_EXP_BITS + D_FRAC_BITS);

  // --- Specials (Inf / NaN)
  if (exp == S_EXP_MAX) {
    if (frac == 0) {
      // Infinity
      return DU(signD | (D_EXP_MAX << D_FRAC_BITS));
    } else {
      // NaN: propagate payload best-effort (preserve signaling/quiet bit when
      // possible)
      if (D_FRAC_BITS == 0) {
        // Degenerate dest can't represent NaN distinctly
        return DU(signD | (D_EXP_MAX << D_FRAC_BITS)); // Inf
      }
      const DU payload = map_nan_payload<DU>(DU(frac), S_FRAC_BITS, D_FRAC_BITS);
      return DU(signD | (D_EXP_MAX << D_FRAC_BITS) | payload);
    }
  }

  // --- Zero (preserve signed zero)
  if (exp == 0 && frac == 0) {
    return DU(signD);
  }

  // Build an (unbiased exponent e) and a significand "sig" with an explicit
  // leading bit. We'll normalize so that sig has its leading 1 at bit position
  // S_FRAC_BITS.
  int32_t e = 0;
  uint64_t sig = 0; // enough for up to (1+52) bits

  if (exp == 0) {
    // subnormal in source
    e = 1 - S_BIAS;
    sig = uint64_t(frac); // no hidden bit
    // normalize: shift until the leading 1 reaches bit S_FRAC_BITS
    // (frac != 0 here)
    while ((sig & (uint64_t{1} << S_FRAC_BITS)) == 0) {
      sig <<= 1;
      --e;
    }
  } else {
    // normal in source
    e = int32_t(exp) - S_BIAS;
    sig = (uint64_t{1} << S_FRAC_BITS) | uint64_t(frac); // explicit hidden 1
  }

  // Now sig is normalized with leading 1 at bit S_FRAC_BITS.
  // We want a dest normalized significand with leading 1 at bit D_FRAC_BITS
  // (for normal), and frac field is the lower D_FRAC_BITS bits (hidden bit
  // removed).
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
    // Produce a subnormal by shifting the normalized significand right by (1 -
    // de). If shift is too large, underflow to zero (preserve sign).
    const int sub_shift = 1 - de; // >= 1
    if (sub_shift > (D_FRAC_BITS + 1) + 16) {
      // definitely underflows to zero (guard; +16 avoids UB for huge shifts)
      return DU(signD);
    }

    uint64_t subSig = sigD;
    if (sub_shift > 0) {
      subSig = rshift_rne<uint64_t>(subSig, sub_shift);
    }

    // In subnormal, exponent field is 0, and fraction is lower D_FRAC_BITS bits
    // of subSig
    const DU fracField = DU(subSig) & mask_n<DU>(D_FRAC_BITS);
    if (fracField == 0) {
      // rounded to zero
      return DU(signD);
    }
    return DU(signD | fracField);
  }

  // Normal destination
  const DU expField = DU(de) & D_EXP_MAX;
  const DU fracField = DU(sigD) & mask_n<DU>(D_FRAC_BITS); // drop hidden bit automatically
  return DU(signD | (expField << D_FRAC_BITS) | fracField);
}

template <typename SrcFloat, int DST_EXP_BITS, int DST_FRAC_BITS> inline
  typename limits::uint_for_bits<1 + DST_EXP_BITS + DST_FRAC_BITS>::type
  pack(SrcFloat f) noexcept {
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
  using DU = typename limits::uint_for_bits<D_TOTAL_BITS>::type;

  const SU bits = bit_cast_memcpy<SU>(f);

  // Fast-path identity: preserve EVERYTHING bit-exactly
  if constexpr (S_EXP_BITS == D_EXP_BITS && S_FRAC_BITS == D_FRAC_BITS && int(sizeof(SU) * 8) == D_TOTAL_BITS) {
    return DU(bits);
  }

  const SU sign = (bits >> (S_EXP_BITS + S_FRAC_BITS)) & 1u;
  const SU exp = (bits >> S_FRAC_BITS) & mask_n<SU>(S_EXP_BITS);
  const SU frac = bits & mask_n<SU>(S_FRAC_BITS);

  return DU(convert_bits_ieee_like<DU, SU>(sign, exp, frac, S_EXP_BITS, S_FRAC_BITS, S_BIAS, D_EXP_BITS, D_FRAC_BITS, D_BIAS));
}

template <typename DstFloat, int SRC_EXP_BITS, int SRC_FRAC_BITS> inline DstFloat unpack(
  typename limits::uint_for_bits<1 + SRC_EXP_BITS + SRC_FRAC_BITS>::type bits) noexcept {
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
  using SU = typename limits::uint_for_bits<S_TOTAL_BITS>::type;

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
  const DU out_bits = convert_bits_ieee_like<DU, SU>(sign, exp, frac, S_EXP_BITS, S_FRAC_BITS, S_BIAS, D_EXP_BITS, D_FRAC_BITS, D_BIAS);

  return bit_cast_memcpy<DstFloat>(out_bits);
}

// specialization for native types

template <> inline uint32_t pack<float, 8, 23>(float f) noexcept { return bit_cast_memcpy<uint32_t>(f); }

template <> inline uint64_t pack<double, 11, 52>(double f) noexcept { return bit_cast_memcpy<uint64_t>(f); }

template <> inline float unpack<float, 8, 23>(uint32_t bits) noexcept { return bit_cast_memcpy<float>(bits); }

template <> inline double unpack<double, 11, 52>(uint64_t bits) noexcept { return bit_cast_memcpy<double>(bits); }

} // namespace fpbits
// !!! ->  Predominantly AI generated code segment end
} // namespace details

template <
  // TExpBits: the bit length of the exponent portion of the float. Bigger ==
  // more bigger sized numbers
  uint32_t TExpBits,
  // TFracBits: the bit length of the fraction portion of the float. Bigger ==
  // more accurate values
  uint32_t TFracBits>
struct FloatingPoint {
  static_assert(TExpBits >= 2, "Need at least 2 exponent bits");

  using Type = typename limits::float_for_bits<1 + int(TExpBits) + int(TFracBits)>::type;
  static_assert(std::numeric_limits<Type>::is_iec559, "Type must be IEEE-754 (IEC 559)");

  using ValueType = Type;
  using InitType = Type;
  using StorageType = typename limits::uint_for_bits<1 + int(TExpBits) + int(TFracBits)>::type;

  static constexpr BitSize ActualSize = BitSize::from_bits(1u + TExpBits + TFracBits);
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept { return ActualSize; }
  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  static constexpr uint32_t ExpBits = TExpBits;
  static constexpr uint32_t FracBits = TFracBits;
  static constexpr int Bias = (1 << (ExpBits - 1)) - 1;
  static constexpr int ExpMaxField = (1 << ExpBits) - 1;
  static constexpr int MaxFiniteExpField = ExpMaxField - 1;
  static constexpr int MaxFiniteExp = MaxFiniteExpField - Bias; // e_max
  static constexpr int MinNormalExp = 1 - Bias;                 // e_min (normal)


  FloatingPoint() noexcept = default;
  FloatingPoint(const FloatingPoint &) noexcept = default;
  FloatingPoint(const Type &v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, FloatingPoint, "sub8::FloatingPoint(const Type &)");
  }

  SUB8_NO_DISCARD static BitFieldResult make(Type v, FloatingPoint &out) noexcept { 
    return out.set_value(v); 
  }

  static FloatingPoint make_or_throw(Type v) SUB8_OPT_NO_EXCEPT {
    FloatingPoint out{};
    auto r = out.set_value(v);
    SUB8_THROW_IF_ERROR(r, FloatingPoint, "FloatingPoint::make_or_throw(Type)");
    return out;
  }

  explicit operator Type() const noexcept { return value_; }
  const Type &value() const noexcept { return value_; }
  const Type *value_ptr() const noexcept { return &value_; }

  const Type value_quantized() const noexcept {
    // Round trip pack and then unpack will give the quantized value of v
    const StorageType bits = details::fpbits::pack<Type, int(TExpBits), int(TFracBits)>(value_);
    return details::fpbits::unpack<Type, int(TExpBits), int(TFracBits)>(bits);
  }

  FloatingPoint &operator=(const FloatingPoint &) noexcept = default;

  FloatingPoint &operator=(Type v) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(v);
    SUB8_THROW_IF_ERROR(r, FloatingPoint, "FloatingPoint::operator=(Type)");
    return *this;
  }


  BitFieldResult set_value_unchecked(Type v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult set_value(Type v) noexcept {
    // Validate by checking that pack/unpack round-trips (this also handles
    // NaN/Inf/overflow).
    const StorageType bits = details::fpbits::pack<Type, int(TExpBits), int(TFracBits)>(v);
    const Type rt = details::fpbits::unpack<Type, int(TExpBits), int(TFracBits)>(bits);

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
      return BitFieldResult::ErrorValueOverflow;
    }

    // For finite values: if the target format overflowed to inf, reject as too
    // large.
    if (std::isinf(rt)) {
      return BitFieldResult::ErrorValueOverflow;
    }

    value_ = v;
    return BitFieldResult::Ok;
  }

  void quantize() noexcept { value_ = this->value_quantized(); }

  bool operator==(const FloatingPoint &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const FloatingPoint &o) const noexcept { return !(*this == o); }

  private:
  Type value_{};
};

template <typename Storage, uint32_t E, uint32_t F> SUB8_NO_DISCARD inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const FloatingPoint<E, F> &fld) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  using Field = FloatingPoint<E, F>;
  using FloatType = typename Field::Type;
  using U = typename Field::StorageType;

  const U bits = details::fpbits::pack<FloatType, int(E), int(F)>(fld.value());
  return bw.template put_bits<U>(bits, Field::ActualSize.bit_size());
}

template <typename Storage, uint32_t E, uint32_t F> SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br, FloatingPoint<E, F> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using Field = FloatingPoint<E, F>;
  using U = typename Field::StorageType;
  using FloatType = typename Field::Type;

  U bits = 0;
  auto r = br.template get_bits<U>(bits, Field::ActualSize.bit_size());
  if (r != BitFieldResult::Ok) {
    return r;
  }

  const FloatType v = details::fpbits::unpack<FloatType, int(E), int(F)>(bits);
  return out.set_value_unchecked(v);
}

#endif

} // namespace sub8
// ============================================================
// END INLINE ./sub8_floats.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_arrays.h
// ============================================================

#pragma once

// Enable: Array Fields
#ifndef SUB8_ENABLE_ARRAY_FIELDS
#define SUB8_ENABLE_ARRAY_FIELDS 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_STL_TYPE 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_VECTOR
#define SUB8_ENABLE_STL_TYPE_VECTOR SUB8_ENABLE_STL_TYPE
#endif

#ifndef SUB8_ENABLE_STL_TYPE_ARRAY
#define SUB8_ENABLE_STL_TYPE_ARRAY SUB8_ENABLE_STL_TYPE
#endif

#ifndef SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#define SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST SUB8_ENABLE_STL_TYPE
#endif

#if SUB8_ENABLE_ARRAY_FIELDS

#include <cstdint> // uintx_t
#include <cstddef> // size_t
#include <utility> // std::declval
#include <type_traits>

#if SUB8_ENABLE_STL_TYPE_VECTOR
#include <vector>
#endif

#if SUB8_ENABLE_STL_TYPE_ARRAY
#include <array>
#endif

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#include <initializer_list>
#endif

#include <cstring> // std::memcmp, std::memcpy
#include <utility> // std::move


#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_api.h"
#include "sub8_errors.h"
#include "sub8_io.h"
#endif

namespace sub8 {

// `read` APIs
// --------------------------------

template <typename T, typename Storage, typename Elem> inline BitFieldResult read(BasicBitReader<Storage> &br, Elem *out, size_t len) {
  T f{};
  auto r = read_field(br, f);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  const auto seq = f.value();
  const size_t s = seq.size();
  const size_t m = (len < s) ? len : s;

  // Zero-fill any remaining destination elements if the source is shorter.
  for (size_t i = m; i < len; ++i) {
    out[i] = Elem{};
  }

  if (m == 0) {
    return BitFieldResult::Ok;
  }

  using Iter = decltype(seq.begin());
  using SrcElem = std::remove_cv_t<std::remove_reference_t<decltype(*std::declval<Iter>())>>;

  if constexpr (std::is_trivially_copyable_v<Elem> && std::is_trivially_copyable_v<SrcElem> && (sizeof(Elem) == sizeof(SrcElem))) {
    std::memcpy(static_cast<void *>(out), static_cast<const void *>(seq.begin()), m * sizeof(Elem));
    return BitFieldResult::Ok;
  } else {
    // Fallback: element-wise conversion.
    for (size_t i = 0; i < m; ++i) {
      out[i] = static_cast<Elem>(seq[i]);
    }
    return BitFieldResult::Ok;
  }
}

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult read(BasicBitReader<Storage> &br, Elem (&out)[N]) {
  return read<T>(br, out, N);
}

#if SUB8_ENABLE_STL_TYPE_ARRAY

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult read(BasicBitReader<Storage> &br,
  std::array<Elem, N> &out) {
  return read<T>(br, out.data(), N);
}

#endif

#if SUB8_ENABLE_STL_TYPE_VECTOR

template <typename T, typename Storage, typename Elem> inline BitFieldResult read(BasicBitReader<Storage> &br, std::vector<Elem> &out) {
  T f{};
  auto r = read_field(br, f);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  const auto seq = f.value();
  const size_t m = seq.size();

  out.clear();
  out.reserve(m);
  for (size_t i = 0; i < m; ++i) {
    out.push_back(static_cast<Elem>(seq[i]));
  }
  return BitFieldResult::Ok;
}

#endif

// `write` APIs
// --------------------------------

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult write(BasicBitWriter<Storage> &bw,
  const Elem (&in)[N]) noexcept {
  T f{};
  auto r = f.set_value(in, N);
  if (r != BitFieldResult::Ok) {
    return r;
  }
  return write_field(bw, f);
}

#if !SUB8_ENABLE_INFALLIBLE
template <typename T, typename Storage, typename Elem, size_t N> inline void write_or_throw(BasicBitWriter<Storage> &bw,
  const Elem (&in)[N]) {
  auto r = write<T>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, T,
      "sub8::write_or_throw<T>(BasicBitWriter<Storage>,"
      " const Elem (&in)[N])");
  }
}
#endif

#if SUB8_ENABLE_STL_TYPE_ARRAY

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult write(BasicBitWriter<Storage> &bw, const std::array<Elem, N> &in) noexcept {
  T f{};
  auto r = f.set_value(in.data(), N);
  if (r != BitFieldResult::Ok) {
    return r;
  }
  return write_field(bw, f);
}

#if !SUB8_ENABLE_INFALLIBLE
template <typename T, typename Storage, typename Elem, size_t N> inline void write_or_throw(BasicBitWriter<Storage> &bw, const std::array<Elem, N> &in) {
  auto r = write<T>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, T,
      "sub8::write_or_throw<T>(BasicBitWriter<Storage>,"
      " const std::array<Elem, N>)");
  }
}

#endif

#endif

#if SUB8_ENABLE_STL_TYPE_VECTOR

template <typename T, typename Storage, typename Elem> inline BitFieldResult write(BasicBitWriter<Storage> &bw,
  const std::vector<Elem> &in) noexcept {
  T f{};
  auto r = f.set_value(in.data(), in.size());
  if (r != BitFieldResult::Ok) {
    return r;
  }
  return write_field(bw, f);
}

#if !SUB8_ENABLE_INFALLIBLE
template <typename T, typename Storage, typename Elem> inline void write_or_throw(BasicBitWriter<Storage> &bw, const std::vector<Elem> &in) {
  auto r = write<T>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, T,
      "sub8::write_or_throw<T>(BasicBitWriter<Storage>,"
      " const std::vector<Elem>)");
  }
}
#endif

#endif

// Array view
template <typename T> struct BitIndexValue {
  const T *data_{nullptr};
  const size_t *size_ptr_{nullptr};

  const T *begin() const noexcept { return data_; }
  const T *end() const noexcept { return data_ + size(); }

  size_t size() const noexcept { return size_ptr_ ? *size_ptr_ : 0; }

  const T &operator[](size_t i) const noexcept { return data_[i]; }

  template <typename Seq> bool equals(const Seq &seq) const noexcept {
    const auto n = size();
    if (n != seq.size())
      return false;
    for (size_t i = 0; i < n; ++i) {
      if (data_[i] != seq[i])
        return false;
    }
    return true;
  }

  bool operator==(const BitIndexValue &o) const noexcept { return equals(o); }
  bool operator!=(const BitIndexValue &o) const noexcept { return !(*this == o); }

  template <typename Seq> bool operator==(const Seq &o) const noexcept { return equals(o); }
  template <typename Seq> bool operator!=(const Seq &o) const noexcept { return !(*this == o); }
};

// Array Field
// ----------------------

enum class ArrayEncoding : uint8_t { ThreePlusPrefixed = 0, Delimited, Prefixed };

template <uint8_t BitLength, bool Signed = false> struct PrimitiveElement {
  using Type = typename limits::numeric_for_bits<BitLength, Signed>::type;
  static constexpr bool IsObject = false;
  static constexpr uint8_t BitsPerElement = BitLength;

  static_assert(BitsPerElement >= 1, "Bits must be >= 1");

  using ElementLimits = sub8::limits::bits_limits<Type, BitsPerElement>;
  using UnderlyingType = typename ElementLimits::Integral;
  using StorageType = typename ElementLimits::UnsignedIntegral;

  static constexpr StorageType MaxCode = ElementLimits::MaxCode;
  static constexpr Type MinValue = ElementLimits::MinValue;
  static constexpr Type MaxValue = ElementLimits::MaxValue;
  static constexpr StorageType DataMask = static_cast<StorageType>(MaxCode);

  // True if we can pack (delimiter bit + data bits) into StorageType in one go.
  static constexpr bool FitsInStorage = (uint32_t(BitsPerElement) + 1u) <= uint32_t(sizeof(StorageType) * 8u);
};

template <typename T> struct ObjectElement {
  using Type = T;
  static constexpr bool IsObject = true;
  using UnderlyingType = T;
  using StorageType = T;
};

template <
  // TypeInfo:
  // Per-element type information (PrimitiveElement<...> or
  // ObjectElement<...>).
  typename TypeInfo,

  // TMinElements:
  // Minimum number of elements in the array.
  //
  // Delimited encoding:
  //  - The first TMinElements are written without continuation bits.
  //  - If fewer elements are provided, remaining elements are implicitly
  //    zero-padded and deserialize as default values.
  //
  // Prefixed encoding:
  //  - Reduces the number of bits required to encode the length prefix.
  //  - Missing elements are implicitly zero-padded and deserialize as default
  //  values.
  size_t TMinElements,

  // TMaxElements:
  // Maximum number of elements in the array.
  //
  // Prefixed encoding:
  //  - The array is prefixed with its length.
  //  - Length prefix uses the minimum number of bits required to represent
  //    the range [TMinElements, TMaxElements].
  //  - If TMinElements == TMaxElements, the length prefix is omitted.
  //
  // Delimited encoding:
  //  - Elements are written sequentially, each followed by a continuation
  //  bit.
  //  - If the array reaches TMaxElements, the termination bit is omitted,
  //    saving one bit when the maximum length is known at compile time.
  //
  // Example:
  //  - TMaxElements = 15 requires 4 bits to encode the length in prefixed
  //  mode.
  size_t TMaxElements,

  // TEncoding:
  // Array encoding strategy.
  //
  // Delimited:
  //  - Elements are terminated using continuation bits.
  //  - Most bit-efficient for very small or sparsely populated arrays
  //    (typically fewer than ~3 elements).
  //
  // Prefixed:
  //  - Array is prefixed with an explicit length value.
  //  - More efficient for larger arrays (3+ elements).
  //
  // ThreePlusPrefixed (default):
  //  - Uses Prefixed encoding when (TMaxElements - TMinElements) > 3,
  //    otherwise falls back to Delimited encoding.
  //  - Note: this decision is compile-time (template-based), not based on
  //  runtime length.
  ArrayEncoding TEncoding = ArrayEncoding::ThreePlusPrefixed>
class Array {
public:
  static_assert(TMaxElements >= TMinElements);

  using ElementType = TypeInfo;
  using Type = typename TypeInfo::Type;
  using ValueType = BitIndexValue<Type>;

  #if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
  using InitType = std::initializer_list<Type>;
  #endif

  using UnderlyingType = typename TypeInfo::UnderlyingType;
  using StorageType = typename TypeInfo::StorageType;

  static constexpr bool IsPrimitive = !TypeInfo::IsObject;
  static constexpr bool IsObject = TypeInfo::IsObject;

  static constexpr size_t MinElements = TMinElements;
  static constexpr size_t MaxElements = TMaxElements;
  static constexpr size_t ValueRange = TMaxElements - TMinElements;

  static constexpr ArrayEncoding Encoding = []() constexpr noexcept -> ArrayEncoding {
    if constexpr (TMaxElements == TMinElements) {
      return ArrayEncoding::Prefixed;
    }
    if constexpr (TEncoding != ArrayEncoding::ThreePlusPrefixed) {
      return TEncoding;
    }

    if constexpr (ValueRange <= 3) {
      return ArrayEncoding::Delimited;
    }
    return ArrayEncoding::Prefixed;
  }();

  static constexpr uint8_t LengthPrefixBitWidth = ValueRange <= 1u ? 1u : limits::bitwidth_to_express_max_value(ValueRange);

  static constexpr BitSize ElementMaxPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TypeInfo::IsObject) {
      return Type::MaxPossibleSize;
    } else {
      return BitSize::from_bits(TypeInfo::BitsPerElement);
    }
  }();

  static constexpr BitSize ElementMinPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TypeInfo::IsObject) {
      return Type::MinPossibleSize;
    } else {
      return BitSize::from_bits(TypeInfo::BitsPerElement);
    }
  }();

  static constexpr BitSize MaxPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TMaxElements == TMinElements) {
      return ElementMaxPossibleSize * TMaxElements;
    } else if constexpr (Encoding == ArrayEncoding::Delimited) {
      // Delimited mode uses (ValueRange - 1) continuation bits at most.
      return ElementMaxPossibleSize * TMaxElements + BitSize::from_bits(ValueRange);
    } else {
      // Prefixed mode uses a fixed-width length prefix.
      return ElementMaxPossibleSize * TMaxElements + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }();

  static constexpr BitSize MinPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TMaxElements == TMinElements) {
      return ElementMinPossibleSize * TMinElements;
    } else if constexpr (Encoding == ArrayEncoding::Delimited) {
      // Smallest delimiter overhead is always 1 bit (the terminator),
      // including the (TMinElements == 0 && n == 0) case.
      return ElementMinPossibleSize * TMinElements + BitSize::from_bits(1u);
    } else {
      // Prefixed mode uses a fixed-width length prefix.
      return ElementMinPossibleSize * TMinElements + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }();

  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }
  BitSize actual_size() const noexcept {
    const size_t effective_n = (size_ < TMinElements) ? TMinElements : size_;
    BitSize elements_size = BitSize::from_bits(0);

    if constexpr (IsPrimitive) {
      // Primitive arrays: size is always bits-per-element * effective_n
      elements_size = BitSize::from_bits(TypeInfo::BitsPerElement) * effective_n;
    } else if constexpr (sub8::details_t::has_actual_size_v<Type>) {
      // Object with compile-time known element size
      elements_size = Type::ActualSize * effective_n;
    } else {
      // Object with runtime element sizes:
      // sum actual for present elements...
      for (size_t i = 0; i < size_; ++i) {
        elements_size += value_[i].actual_size();
      }
      // + pad up to effective_n with min size
      if (effective_n > size_) {
        // Note, this assumes default ctor will write MinPossibleSize in length
        elements_size += Type::MinPossibleSize * (effective_n - size_);
      }
    }

    if constexpr (TMaxElements == TMinElements) {
      // Fixed-length array: no length/termination overhead.
      return elements_size;

    } else if constexpr (Encoding == ArrayEncoding::Delimited) {
      // Delimited overhead depends on runtime length.
      size_t delimiter_bits = 0;

      if constexpr (TMinElements == 0) {
        if (effective_n == 0) {
          delimiter_bits = 1; // just terminator
        } else if (effective_n == TMaxElements) {
          delimiter_bits = ValueRange; // no terminator
        } else {
          delimiter_bits = effective_n + 1; // cont bits + terminator
        }
      } else {
        if (effective_n == TMaxElements) {
          delimiter_bits = ValueRange; // no terminator
        } else {
          delimiter_bits = (effective_n - TMinElements) + 1; // cont bits + terminator
        }
      }

      return elements_size + BitSize::from_bits(delimiter_bits);

    } else {
      // Prefixed: fixed length prefix overhead.
      return elements_size + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }

private:
  static constexpr bool CanSkipPrimitiveRangeChecks =
    IsPrimitive && std::is_integral_v<Type> && std::is_unsigned_v<Type> && (TypeInfo::BitsPerElement >= (sizeof(Type) * 8u));

  template <typename U> static constexpr bool CanMemcpy = std::is_same_v<std::remove_cv_t<U>, Type> && std::is_trivially_copyable_v<Type>;

public:
  Array() noexcept = default;
  Array(const Array &) noexcept = default;

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#if !SUB8_ENABLE_INFALLIBLE
  Array(std::initializer_list<Type> init) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, Array, "sub8::Array(std::initializer_list<Type>)");
  }
#endif
#endif

  template <typename U> SUB8_NO_DISCARD static BitFieldResult make(const U *seq, size_t len, Array &out) noexcept {
    return out.set_value(seq, len);
  }

#if !SUB8_ENABLE_INFALLIBLE
  template <typename U> static Array make_or_throw(const U *seq, size_t len) SUB8_OPT_NO_EXCEPT {
    Array out{};
    auto r = out.set_value(seq, len);
    SUB8_THROW_IF_ERROR(r, Array, "Array::make_or_throw(ptr,len)");
    return out;
  }

#endif

  template <typename Seq> SUB8_NO_DISCARD static BitFieldResult make(const Seq &seq, Array &out) { return out.set_value(seq); }


#if !SUB8_ENABLE_INFALLIBLE
  template <typename Seq> static Array make_or_throw(const Seq &seq) SUB8_OPT_NO_EXCEPT {
    Array out{};
    auto r = out.set_value(seq);
    SUB8_THROW_IF_ERROR(r, Array, "Array::make_or_throw(seq)");
    return out;
  }
#endif

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST

  SUB8_NO_DISCARD static BitFieldResult make(std::initializer_list<Type> init, Array &out) noexcept { return out.set_value(init); }

#if !SUB8_ENABLE_INFALLIBLE

  static Array make_or_throw(std::initializer_list<Type> init) SUB8_OPT_NO_EXCEPT {
    Array out{};
    auto r = out.set_value(init);
    SUB8_THROW_IF_ERROR(r, Array, "Array::make_or_throw(init_list)");
    return out;
  }
#endif

#endif

  bool empty() const noexcept { return size_ == 0; }
  size_t size() const noexcept { return size_; }
  const Type &operator[](size_t i) const noexcept { return value_[i]; }

  void clear() noexcept { size_ = 0; }

  BitIndexValue<Type> value() const noexcept { return BitIndexValue<Type>{value_, &size_}; }

  explicit operator BitIndexValue<Type>() const noexcept { return value(); }

  bool operator==(const Array &o) const noexcept {
    if (size_ != o.size_)
      return false;

    if constexpr (!IsObject && std::is_trivially_copyable_v<Type> && std::has_unique_object_representations_v<Type>) {
      return std::memcmp(value_, o.value_, size_ * sizeof(Type)) == 0;
    } else {
      for (size_t i = 0; i < size_; ++i) {
        if (!(value_[i] == o.value_[i]))
          return false;
      }
      return true;
    }
  }

  bool operator!=(const Array &o) const noexcept { return !(*this == o); }
  Array &operator=(const Array &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  template <typename Seq> Array &operator=(const Seq &seq) {
    auto r = set_value(seq);
    SUB8_THROW_IF_ERROR(r, Array, "Array::operator=(const Seq &)");
    return *this;
  }
  #endif

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#if !SUB8_ENABLE_INFALLIBLE
  Array &operator=(std::initializer_list<Type> init) {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, Array, "Array::operator=(std::initializer_list<Type>)");
    return *this;
  }
#endif
#endif

  template <typename Seq> SUB8_NO_DISCARD BitFieldResult set_value(const Seq &seq) {
    // Note Seq doesn't guarantee noexcept, so this method can not be noexcept
    clear();
    const size_t n = seq.size();
    if (n > TMaxElements)
      return BitFieldResult::ErrorTooManyElements;

    for (size_t i = 0; i < n; ++i) {
      auto r = push_back(static_cast<Type>(seq[i]));
      if (r != BitFieldResult::Ok)
        return r;
    }
    return BitFieldResult::Ok;
  }

  template <typename U> SUB8_NO_DISCARD BitFieldResult set_value(const U *seq, size_t len) noexcept {
    clear();
    if (seq == nullptr) {
      return (len == 0) ? BitFieldResult::Ok : BitFieldResult::ErrorInvalidBitFieldValue;
    }
    if (len > TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    // Fast path: exact type match and trivially copyable.
    // Only allowed if we don't need per-element validation.
    if constexpr (CanMemcpy<U>) {
      if constexpr (IsObject) {
        std::memcpy(value_, seq, len * sizeof(Type));
        size_ = len;
        return BitFieldResult::Ok;
      } else {
        // Primitive arrays:
        // If all values fit in Type, we can skip per-element checks.
        // Otherwise we must validate each element (even if memcpy would be
        // possible).
        if constexpr (CanSkipPrimitiveRangeChecks) {
          std::memcpy(value_, seq, len * sizeof(Type));
          size_ = len;
          return BitFieldResult::Ok;
        }
      }
    }

    for (size_t i = 0; i < len; ++i) {
      auto r = push_back(static_cast<Type>(seq[i]));
      if (r != BitFieldResult::Ok) {
        return r;
      }
    }
    return BitFieldResult::Ok;
  }

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST

  SUB8_NO_DISCARD BitFieldResult set_value(std::initializer_list<Type> init) noexcept { return set_value(init.begin(), init.size()); }

#endif

// TODO: needs changes to be   #if !SUB8_ENABLE_INFALLIBLE
  SUB8_NO_DISCARD BitFieldResult push_back(const Type &v) noexcept(std::is_nothrow_copy_assignable_v<Type>) {
    if constexpr (IsPrimitive) {
      if (v < ElementType::MinValue || v > ElementType::MaxValue) {
        return BitFieldResult::ErrorValueOverflow;
      }
    }
    if (size_ >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    value_[size_++] = v;
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult push_back(Type &&v) noexcept(std::is_nothrow_move_assignable_v<Type>) {
    if constexpr (IsPrimitive) {
      if (v < ElementType::MinValue || v > ElementType::MaxValue) {
        return BitFieldResult::ErrorValueOverflow;
      }
    }
    if (size_ >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }
    value_[size_] = std::move(v); // Type could throw exception here
    size_++;                      // increment after move, just incase exception is thrown
    return BitFieldResult::Ok;
  }

private:
  size_t size_{0};
  Type value_[TMaxElements]{};

};

// Delimited read / write implementation

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
write_delimited_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, Encoding> &p) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  using F = Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  const size_t n = p.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : n;
  if (effective_n > TMaxElements) {
    return BitFieldResult::ErrorTooManyElements;
  }

  auto write_prefixed_value = [&](Type v) -> BitFieldResult {
    // Note: the delimiter bit is the MSB of the encoded group.
    if constexpr (F::IsObject) {
      auto r = bw.template put_bits<uint8_t>(1u, 1);
      if (r != BitFieldResult::Ok)
        return r;
      return write_field(bw, v);
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);

      if constexpr (TypeInfo::FitsInStorage) {
        const StorageType masked = code & TypeInfo::DataMask;
        const StorageType group = (StorageType{1} << TypeInfo::BitsPerElement) | masked;
        return bw.template put_bits<StorageType>(group, uint32_t(TypeInfo::BitsPerElement) + 1u);
      } else {
        auto r = bw.template put_bits<uint8_t>(1u, 1);
        if (r != BitFieldResult::Ok)
          return r;
        return bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
      }
    }
  };

  auto write_non_prefixed_value = [&](Type v) -> BitFieldResult {
    if constexpr (F::IsObject) {
      return write_field(bw, v);
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);
      if (code > TypeInfo::MaxCode) {
        return BitFieldResult::ErrorValueOverflow;
      }
      return bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
    }
  };

  size_t i = 0;

  if constexpr (TMinElements == 0) {
    if (n == 0) {
      return bw.template put_bits<uint8_t>(0u, 1);
    }
  } else {
    // Write the first MinElements without delimiter bits.
    // MinElements are always written; if elements are missing they are written
    // as Type{} (i.e. zero-padded / default-constructed).
    for (; i < TMinElements; ++i) {
      const Type v = (i < n) ? p[i] : Type{};
      auto r = write_non_prefixed_value(v);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  // Write remaining elements with a delimiter (continuation) bit.
  for (; i < effective_n; ++i) {
    const Type v = (i < n) ? p[i] : Type{};
    auto r = write_prefixed_value(v);
    if (r != BitFieldResult::Ok)
      return r;
  }

  // Write a terminator unless the array is exactly at max length (termination
  // is implied).
  if (n != TMaxElements) {
    return bw.template put_bits<uint8_t>(0u, 1);
  }
  return BitFieldResult::Ok;
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
read_delimited_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, Encoding> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  out.clear();

  auto read_non_prefixed_value = [&](Type &v) -> BitFieldResult {
    if constexpr (F::IsObject) {
      return read_field(br, v);
    } else {
      StorageType raw = 0;
      auto r = br.template get_bits<StorageType>(raw, uint32_t(TypeInfo::BitsPerElement));
      if (r != BitFieldResult::Ok)
        return r;

      const StorageType code = StorageType(raw & TypeInfo::DataMask);
      v = sub8::packing::unpack<Type>(code);
      return BitFieldResult::Ok;
    }
  };

  auto read_prefixed_value = [&](Type &v) -> BitFieldResult {
    // Continuation bit was already consumed; the next bits are the value.
    if constexpr (F::IsObject) {
      return read_field(br, v);
    } else {
      StorageType raw = 0;
      auto r = br.template get_bits<StorageType>(raw, uint32_t(TypeInfo::BitsPerElement));
      if (r != BitFieldResult::Ok)
        return r;

      const StorageType code = StorageType(raw & TypeInfo::DataMask);
      v = sub8::packing::unpack<Type>(code);
      return BitFieldResult::Ok;
    }
  };

  // Read fixed (non-delimited) minimum portion.
  if constexpr (TMinElements != 0) {
    while (out.size() < TMinElements) {
      Type v{};
      auto r = read_non_prefixed_value(v);
      if (r != BitFieldResult::Ok)
        return r;

      r = out.push_back(std::move(v));
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  // Read delimited portion: [cont][value] ... terminates when cont == 0.
  uint8_t cont = 0;
  auto r = br.template get_bits<uint8_t>(cont, 1);
  if (r != BitFieldResult::Ok)
    return r;

  if (cont == 0) {
    return BitFieldResult::Ok;
  }

  while (true) {
    if (out.size() >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    Type v{};
    auto r = read_prefixed_value(v);
    if (r != BitFieldResult::Ok)
      return r;

    r = out.push_back(std::move(v));
    if (r != BitFieldResult::Ok)
      return r;

    // If we're at max, the trailing terminator bit is implied.
    if (out.size() == TMaxElements) {
      return BitFieldResult::Ok;
    }

    r = br.template get_bits<uint8_t>(cont, 1);
    if (r != BitFieldResult::Ok)
      return r;

    if (cont == 0) {
      return BitFieldResult::Ok;
    }
  }
}

// Prefixed read / write implementation

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
write_prefixed_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, Encoding> &f) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));

  using F = const Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  if (f.size() > TMaxElements) {
    return BitFieldResult::ErrorTooManyElements;
  }

  const size_t n = f.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : f.size(); // Zero-pad up to MinElements.

  // Omit writing the length prefix if TMaxElements == TMinElements.
  if constexpr (TMaxElements != TMinElements) {
    auto r = bw.template put_bits<size_t>(effective_n - TMinElements, F::LengthPrefixBitWidth);
    if (r != BitFieldResult::Ok)
      return r;
  }

  auto values = f.value();

  for (size_t i = 0; i < effective_n; ++i) {
    const Type v = (i < n) ? values[i] : Type{};

    if constexpr (!F::IsPrimitive) {
      auto r = write_field(bw, v);
      if (r != BitFieldResult::Ok)
        return r;
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);
      if (code > TypeInfo::MaxCode)
        return BitFieldResult::ErrorValueOverflow;

      auto r = bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  return BitFieldResult::Ok;
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
read_prefixed_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, Encoding> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  out.clear();

  // Read length prefix.
  size_t n = 0;

  if constexpr (TMaxElements == TMinElements) {
    n = TMaxElements;
  } else {
    auto r = br.template get_bits<size_t>(n, F::LengthPrefixBitWidth);
    if (r != BitFieldResult::Ok)
      return r;

    n = n + TMinElements;

    if (n > TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }
  }

  // Read elements.
  for (size_t i = 0; i < n; ++i) {
    if constexpr (!F::IsPrimitive) {
      Type inner{};
      auto r = read_field(br, inner);
      if (r != BitFieldResult::Ok)
        return r;

      r = out.push_back(inner);
      if (r != BitFieldResult::Ok)
        return r;
    } else {
      StorageType code = 0;

      auto r = br.template get_bits<StorageType>(code, TypeInfo::BitsPerElement);
      if (r != BitFieldResult::Ok)
        return r;

      if (code > TypeInfo::MaxCode) {
        return BitFieldResult::ErrorValueOverflow;
      }

      r = out.push_back(sub8::packing::unpack<Type>(code));
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  return BitFieldResult::Ok;
}

// Delimited read / write
template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Delimited> &p) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  return write_delimited_field(bw, p);
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Delimited> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  return read_delimited_field(br, out);
}

// Prefixed read / write
template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Prefixed> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  return read_prefixed_field(br, out);
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Prefixed> &p) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  return write_prefixed_field(bw, p);
}

// ThreePlusPrefixed read / write
template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::ThreePlusPrefixed> &p) noexcept {

  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  if constexpr (TMaxElements == TMinElements) {
    // Use the prefixed implementation for fixed field sizes.
    // While both methods can support it, behavior is more stable if only one
    // implementation is used.
    return write_prefixed_field(bw, p);
  } else if constexpr ((TMaxElements - TMinElements) <= 3) {
    return write_delimited_field(bw, p);
  } else {
    return write_prefixed_field(bw, p);
  }
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> 
SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::ThreePlusPrefixed> &out) noexcept {

  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));

  if constexpr (TMaxElements == TMinElements) {
    // Use the prefixed implementation for fixed field sizes.
    // While both methods can support it, behavior is more stable if only one
    // implementation is used.
    return read_prefixed_field(br, out);
  } else if constexpr ((TMaxElements - TMinElements) <= 3) {
    return read_delimited_field(br, out);
  } else {
    return read_prefixed_field(br, out);
  }
}

// Object Array
template <typename T, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using ObjectArray =
  Array<ObjectElement<T>, TMinElements, TMaxElements, Encoding>;

template <uint8_t BitWidth, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using BufferArray =
  Array<PrimitiveElement<BitWidth>, TMinElements, TMaxElements, Encoding>;

template <uint8_t BitWidth, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed>
using SignedBufferArray = Array<PrimitiveElement<BitWidth, /*Signed*/ true>, TMinElements, TMaxElements, Encoding>;

template <typename T, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using FixedSizeObjectArray =
  Array<ObjectElement<T>, TMaxElements, TMaxElements, Encoding>;

template <uint8_t BitWidth, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using FixedSizeBufferArray =
  Array<PrimitiveElement<BitWidth>, TMaxElements, TMaxElements, Encoding>;

} // namespace sub8

#endif // SUB8_ENABLE_ARRAY_FIELDS

// ============================================================
// END INLINE ./sub8_arrays.h
// ============================================================



// ============================================================
// BEGIN INLINE ./sub8_enums.h
// ============================================================

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

// ============================================================
// END INLINE ./sub8_enums.h
// ============================================================


using HelloRequestMessage_hello_phrase_T1 = sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>;

using u16 = sub8::Integer<16, false>;

namespace users {
namespace name {
namespace space {
struct HelloRequestMessage {
  using Type = HelloRequestMessage;
  using InitType = HelloRequestMessage;
  using ValueType = HelloRequestMessage;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MaxPossibleSize;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MaxPossibleSize;
    return len;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MinPossibleSize;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MinPossibleSize;
    return len;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  const HelloRequestMessage& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const HelloRequestMessage& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }

  sub8::Integer<16, false> say_it_n_time{};
  sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>> hello_phrase{};

  bool operator==(const HelloRequestMessage& o) const noexcept;
  bool operator!=(const HelloRequestMessage& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::HelloRequestMessage& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = write_field(bw, v.say_it_n_time);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.hello_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::HelloRequestMessage& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = read_field(br, out.say_it_n_time);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.hello_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}
} // namespace sub8

using bfloat16 = sub8::FloatingPoint<8, 7>;

using u32 = sub8::Integer<32, false>;

namespace users {
namespace name {
namespace space {
struct MessageItem {
  using Type = MessageItem;
  using InitType = MessageItem;
  using ValueType = MessageItem;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MaxPossibleSize;
    len += sub8::Integer<32, false>::MaxPossibleSize;
    len += sub8::FloatingPoint<8, 7>::MaxPossibleSize;
    return len;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MinPossibleSize;
    len += sub8::Integer<32, false>::MinPossibleSize;
    len += sub8::FloatingPoint<8, 7>::MinPossibleSize;
    return len;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  const MessageItem& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const MessageItem& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }

  sub8::Integer<16, false> feild_1{};
  sub8::Integer<32, false> feild_2{};
  sub8::FloatingPoint<8, 7> feild_3{};

  bool operator==(const MessageItem& o) const noexcept;
  bool operator!=(const MessageItem& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::MessageItem& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = write_field(bw, v.feild_1);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.feild_2);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.feild_3);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::MessageItem& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = read_field(br, out.feild_1);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.feild_2);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.feild_3);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}
} // namespace sub8

using HelloResponseMessage_list_T3 = sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited>;

using HelloResponseMessage_response_phrase_T2 = sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>;

namespace users {
namespace name {
namespace space {
struct HelloResponseMessage {
  using Type = HelloResponseMessage;
  using InitType = HelloResponseMessage;
  using ValueType = HelloResponseMessage;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MaxPossibleSize;
    len += sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited>::MaxPossibleSize;
    return len;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MinPossibleSize;
    len += sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited>::MinPossibleSize;
    return len;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  const HelloResponseMessage& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const HelloResponseMessage& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }

  sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>> response_phrase{};
  sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited> list{};

  bool operator==(const HelloResponseMessage& o) const noexcept;
  bool operator!=(const HelloResponseMessage& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::HelloResponseMessage& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = write_field(bw, v.response_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.list);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::HelloResponseMessage& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = read_field(br, out.response_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.list);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}
} // namespace sub8

namespace users {
namespace name {
namespace space {
enum class HelloMessageType : uint32_t {
  NullValue = 0,
  hello_request = 1,
  hello_response = 2,
};

struct HelloMessage__VariantTagMeta {
  inline static constexpr uint32_t kCodes[] = {
    0u,
    1u,
    2u,
  };
  inline static constexpr size_t kCount = sizeof(kCodes) / sizeof(kCodes[0]);
  inline static constexpr uint32_t MinCodeU32 = []() constexpr noexcept {
    uint32_t m = kCodes[0];
    for (size_t i = 1; i < kCount; ++i) if (kCodes[i] < m) m = kCodes[i];
    return m;
  }();
  inline static constexpr uint32_t MaxCodeU32 = []() constexpr noexcept {
    uint32_t m = kCodes[0];
    for (size_t i = 1; i < kCount; ++i) if (kCodes[i] > m) m = kCodes[i];
    return m;
  }();
  inline static constexpr HelloMessageType MinEnum = static_cast<HelloMessageType>(MinCodeU32);
  inline static constexpr HelloMessageType MaxEnum = static_cast<HelloMessageType>(MaxCodeU32);
};

using HelloMessageTypeFieldBase = sub8::Enumeration<HelloMessageType, HelloMessage__VariantTagMeta::MinEnum, HelloMessage__VariantTagMeta::MaxEnum>;
struct HelloMessageTypeField : HelloMessageTypeFieldBase {
  using HelloMessageTypeFieldBase::HelloMessageTypeFieldBase;
  HelloMessageTypeField() noexcept = default;
};

struct HelloMessage {
  HelloMessageType type{HelloMessageType::NullValue};
  union VariantValue {
    sub8::NullValue null_v;
    users::name::space::HelloRequestMessage hello_request;
    users::name::space::HelloResponseMessage hello_response;
    VariantValue() {}
    ~VariantValue() {}
  } variant;

  using Type = HelloMessage;
  using InitType = HelloMessage;
  using ValueType = HelloMessage;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize payload = sub8::NullValue::MaxPossibleSize;
    if (users::name::space::HelloRequestMessage::MaxPossibleSize > payload) payload = users::name::space::HelloRequestMessage::MaxPossibleSize;
    if (users::name::space::HelloResponseMessage::MaxPossibleSize > payload) payload = users::name::space::HelloResponseMessage::MaxPossibleSize;
    return payload + HelloMessageTypeField::MaxPossibleSize;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize payload = sub8::NullValue::MinPossibleSize;
    if (users::name::space::HelloRequestMessage::MinPossibleSize < payload) payload = users::name::space::HelloRequestMessage::MinPossibleSize;
    if (users::name::space::HelloResponseMessage::MinPossibleSize < payload) payload = users::name::space::HelloResponseMessage::MinPossibleSize;
    return payload + HelloMessageTypeField::MinPossibleSize;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  void construct_null() noexcept;
  void destroy_active() noexcept;

  HelloMessage() noexcept;
  HelloMessage(const HelloMessage& o);
  HelloMessage(HelloMessage&& o) noexcept;
  HelloMessage& operator=(const HelloMessage& o);
  HelloMessage& operator=(HelloMessage&& o) noexcept;
  ~HelloMessage();

  const HelloMessage& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const HelloMessage& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }
  bool is_null() const noexcept;

  const users::name::space::HelloRequestMessage* get_hello_request() const noexcept;
  bool is_hello_request() const noexcept;
  sub8::BitFieldResult set_hello_request(const users::name::space::HelloRequestMessage& v) noexcept;

  const users::name::space::HelloResponseMessage* get_hello_response() const noexcept;
  bool is_hello_response() const noexcept;
  sub8::BitFieldResult set_hello_response(const users::name::space::HelloResponseMessage& v) noexcept;

  bool operator==(const HelloMessage& o) const noexcept;
  bool operator!=(const HelloMessage& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::HelloMessage& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  users::name::space::HelloMessageTypeField t; t.set_value(static_cast<users::name::space::HelloMessageType>(static_cast<uint32_t>(v.type)));
  r = write_field(bw, t);
  if (r != sub8::BitFieldResult::Ok) return r;
  switch (static_cast<uint32_t>(v.type)) {
    case 0u: return sub8::BitFieldResult::Ok;
    case 1u: return write_field(bw, v.variant.hello_request);
    case 2u: return write_field(bw, v.variant.hello_response);
    default: return sub8::BitFieldResult::Ok;
  }
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::HelloMessage& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  users::name::space::HelloMessageTypeField t;
  r = read_field(br, t);
  if (r != sub8::BitFieldResult::Ok) return r;
  out.destroy_active();
  out.type = static_cast<users::name::space::HelloMessageType>(static_cast<uint32_t>(t.value()));
  switch (static_cast<uint32_t>(out.type)) {
    case 0u: out.construct_null(); return sub8::BitFieldResult::Ok;
    case 1u: new (&out.variant.hello_request) users::name::space::HelloRequestMessage{}; return read_field(br, out.variant.hello_request);
    case 2u: new (&out.variant.hello_response) users::name::space::HelloResponseMessage{}; return read_field(br, out.variant.hello_response);
    default: out.construct_null(); return sub8::BitFieldResult::Ok;
  }
}
} // namespace sub8

