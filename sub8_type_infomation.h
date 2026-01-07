#pragma once

#include <limits>
#include <type_traits>
#include <string_view>

namespace sub8 {

// Template helpers for resolving the inner type of an enum
namespace unpack_t {
template<typename T, bool IsEnum = std::is_enum<T>::value> struct underlying_or_self {
  using type = T;
};

template<typename T> struct underlying_or_self<T, true> {
  using type = typename std::underlying_type<T>::type;
};
}  // namespace unpack_t

namespace limits {

template<typename T> using underlying_or_self_t = typename unpack_t::underlying_or_self<T>::type;

template<typename T, uint32_t Bits> struct bits_limits {
  using UnderlyingType = underlying_or_self_t<T>;
  using StorageType = std::make_unsigned_t<UnderlyingType>;

  static_assert(Bits >= 1, "Bits must be >= 1");
  static_assert(std::is_integral_v<UnderlyingType> || std::is_enum_v<T>,
                "bits_limits requires integral (or enum) types");

  static constexpr uint32_t BitsWidth = Bits;
  static constexpr uint32_t StorageTypeWidth = sizeof(StorageType) * 8;

  // Max code representable in Bits
  static constexpr StorageType MaxCode = (BitsWidth >= StorageTypeWidth)
                                             ? std::numeric_limits<StorageType>::max()
                                             : (StorageType{1} << BitsWidth) - StorageType{1};

  static constexpr T MinValue = []() constexpr noexcept -> T {
    if constexpr (std::is_signed_v<UnderlyingType>) {
      if constexpr (BitsWidth >= StorageTypeWidth) {
        return static_cast<T>(std::numeric_limits<UnderlyingType>::min());
      } else {
        const StorageType mag = (StorageType{1} << (BitsWidth - 1));
        return static_cast<T>(-static_cast<std::make_signed_t<StorageType>>(mag));
      }
    } else {
      return static_cast<T>(0);
    }
  }();

  static constexpr T MaxValue = []() constexpr noexcept -> T {
    if constexpr (std::is_signed_v<UnderlyingType>) {
      if constexpr (BitsWidth >= StorageTypeWidth) {
        return static_cast<T>(std::numeric_limits<UnderlyingType>::max());
      } else {
        const StorageType mag = (StorageType{1} << (BitsWidth - 1));
        return static_cast<T>(static_cast<std::make_signed_t<StorageType>>(mag - 1));
      }
    } else {
      if constexpr (BitsWidth >= StorageTypeWidth) {
        return static_cast<T>(std::numeric_limits<UnderlyingType>::max());
      } else {
        return static_cast<T>(static_cast<UnderlyingType>(MaxCode));
      }
    }
  }();
};

}  // namespace limits

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

template<class T> constexpr std::string_view inferred_type_name() noexcept {
#if defined(__clang__) || defined(__GNUC__)
  return _no_rtti::extract_type_name(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
  return _no_rtti::extract_type_name(__FUNCSIG__);
#else
  return "type";
#endif
}

}  // namespace _no_rtti

template<typename T> inline std::string_view name() noexcept {
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
}  // namespace type_info
}  // namespace sub8
