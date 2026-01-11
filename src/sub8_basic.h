#pragma once

#include "sub8_errors.h"
#include "sub8_io.h"

#define SUB8_DECLARE_BITFIELD_ALIAS(ALIAS, ...) \
  extern template struct __VA_ARGS__; \
  using ALIAS = __VA_ARGS__

namespace sub8 {

// make helpers

template<typename TFeildType> inline BitFieldResult make(typename TFeildType::InitType t_val, TFeildType &out) {
  TFeildType tmp{};
  auto r = tmp.set_value(t_val);
  if (r == BitFieldResult::Ok) {
    out = tmp;
  }
  return r;
}

template<typename TFeildType> inline TFeildType make_or_throw(typename TFeildType::InitType t_val) {
  TFeildType tmp{};
  auto r = tmp.set_value(t_val);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType, "sub8::make_or_throw<TFeildType>(TFeildType::InitType)");
  }
  return tmp;
}

// Read

// Reads and returns the value as the feild type
template<typename TFeildType, typename Storage>
inline BitFieldResult read(BasicBitReader<Storage> &br, TFeildType &out) {
  TFeildType f{};
  auto r = read_field(br, f);
  if (r == BitFieldResult::Ok) {
    out = f;
  }
  return r;
}

template<typename TFeildType, typename Storage> inline TFeildType read_or_throw(BasicBitReader<Storage> &br) {
  TFeildType ret{};
  auto r = read_field(br, ret);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType, "sub8::read_or_throw<TFeildType>(BasicBitReader<Storage>)");
  }
  return ret;
}

// Write

template<typename T, typename Storage> inline BitFieldResult write(BasicBitWriter<Storage> &br, T in) {
  T f{};
  auto r = make<T>(in, f);
  if (r != BitFieldResult::Ok)
    return r;
  return write_field(br, f);
}

template<typename T, typename Storage>
inline BitFieldResult write(BasicBitWriter<Storage> &br, typename T::InitType in) {
  T f{};
  auto r = make<T>(in, f);
  if (r != BitFieldResult::Ok)
    return r;
  return write_field(br, f);
}

template<typename TFeildType, typename Storage> inline void write_or_throw(BasicBitWriter<Storage> &br, TFeildType in) {
  auto r = write_field(br, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType, "sub8::write_or_throw<TFeildType>(BasicBitWriter<Storage>, TFeildType)");
  }
}

template<typename TFeildType, typename Storage>
inline void write_or_throw(BasicBitWriter<Storage> &br, typename TFeildType::InitType in) {
  auto r = write<TFeildType>(br, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
                              "sub8::write_or_throw<TFeildType>(BasicBitWriter<Storage>, TFeildType::InitType)");
  }
}

// Basic Field
// ------------
template<
    // T: storage type, must be equal or greater than the wire bit size
    typename T,

    // BitLength: the number of bits the type can utilize
    uint32_t BitLength>
struct BasicValueField {
  using Type = T;
  using InitType = T;
  using ValueType = T;
  using StorageType = typename unpack_t::underlying_or_self<T>::type;

  static constexpr uint32_t TotalUsableBits = BitLength;

  T value_{};
  operator T() const noexcept { return value_; }
  const T &value() const noexcept { return value_; }

  bool operator==(const BasicValueField &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const BasicValueField &o) const noexcept { return !(*this == o); }

  BasicValueField &operator=(T v) noexcept {
    value_ = v;
    return *this;
  }

  BitFieldResult set_value(InitType v) noexcept {
    value_ = v;
    return BitFieldResult::Ok;
  }
};

#include <type_traits>
#include <utility>  // std::declval
namespace detect {

template<class...>
struct dependent_false : std::false_type {};

template<class Storage, class T, class = void>
struct has_write_field : std::false_type {};

template<class Storage, class T>
struct has_write_field<
    Storage, T,
    std::void_t<decltype(
        write_field(std::declval<BasicBitWriter<Storage>&>(),
                    std::declval<const T&>())
    )>
> : std::bool_constant<
      std::is_same_v<
          decltype(write_field(std::declval<BasicBitWriter<Storage>&>(),
                               std::declval<const T&>())),
          BitFieldResult
      >
    > {};

// --- read_field detection ---
template<class Storage, class T, class = void>
struct has_read_field : std::false_type {};

template<class Storage, class T>
struct has_read_field<
    Storage, T,
    std::void_t<decltype(
        read_field(std::declval<BasicBitReader<Storage>&>(),
                   std::declval<T&>())
    )>
> : std::bool_constant<
      std::is_same_v<
          decltype(read_field(std::declval<BasicBitReader<Storage>&>(),
                              std::declval<T&>())),
          BitFieldResult
      >
    > {};

template<class Storage, class T>
inline constexpr bool has_write_field_v = has_write_field<Storage, T>::value;

template<class Storage, class T>
inline constexpr bool has_read_field_v = has_read_field<Storage, T>::value;

template<class Storage, class T>
inline constexpr bool has_rw_field_v = has_write_field_v<Storage, T> && has_read_field_v<Storage, T>;

} // namespace sub8::detect


}  // namespace sub8