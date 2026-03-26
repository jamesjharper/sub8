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
