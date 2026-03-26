#pragma once

#ifndef SUB8_ENABLE_OPTIONAL_FIELDS
#define SUB8_ENABLE_OPTIONAL_FIELDS 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_STL_TYPE 1
#endif

#if !SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_USE_NON_ALLOC_OPTIONAL_AS_DEFAULT 1
#endif

#ifndef SUB8_ENABLE_USE_NON_ALLOC_OPTIONAL_AS_DEFAULT
#define SUB8_ENABLE_USE_NON_ALLOC_OPTIONAL_AS_DEFAULT 1
#endif

#if SUB8_ENABLE_OPTIONAL_FIELDS

#include <cstdint> // uintx_t
#include <cstddef> // size_t
#include <utility> // std::declval
#include <new>
#include <type_traits>

#if SUB8_ENABLE_STL_TYPE
#include <optional>
#endif

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_api.h"
#include "sub8_errors.h"
#include "sub8_io.h"
#endif

namespace sub8 {

#if SUB8_ENABLE_STL_TYPE

using nullopt_t = std::nullopt_t;
inline constexpr auto nullopt = std::nullopt;

using in_place_t = std::in_place_t;
inline constexpr auto in_place = std::in_place;

#else

struct nullopt_t {
  constexpr explicit nullopt_t(int) {}
};
inline constexpr nullopt_t nullopt{0};

struct in_place_t {
  constexpr in_place_t() = default;
};
inline constexpr in_place_t in_place{};

#endif

template <class T> class non_alloc_optional {
public:
  using value_type = T;

  constexpr non_alloc_optional() noexcept = default;
  constexpr non_alloc_optional(nullopt_t) noexcept {}

  template <typename U = T, std::enable_if_t<std::is_copy_constructible_v<U>, int> = 0>
  non_alloc_optional(const non_alloc_optional &other) {
    if (other.has_) {
      ::new (ptr()) T(*other.ptr());
      has_ = true;
    }
  }

  template <typename U = T, std::enable_if_t<!std::is_copy_constructible_v<U>, int> = 0>
  non_alloc_optional(const non_alloc_optional &other) = delete;

  template <typename U = T, std::enable_if_t<std::is_move_constructible_v<U>, int> = 0>
  non_alloc_optional(non_alloc_optional &&other) noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (other.has_) {
      ::new (ptr()) T(std::move(*other.ptr()));
      has_ = true;
    }
  }

  template <typename U = T, std::enable_if_t<!std::is_move_constructible_v<U>, int> = 0>
  non_alloc_optional(non_alloc_optional &&other) = delete;

  template <class... Args>
  explicit non_alloc_optional(in_place_t, Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    ::new (ptr()) T(std::forward<Args>(args)...);
    has_ = true;
  }

  template <class U, std::enable_if_t<std::is_constructible_v<T, U &&>, int> = 0>
  explicit non_alloc_optional(U &&v) noexcept(std::is_nothrow_constructible_v<T, U &&>) {
    ::new (ptr()) T(std::forward<U>(v));
    has_ = true;
  }

  ~non_alloc_optional() noexcept { reset(); }

  non_alloc_optional &operator=(nullopt_t) noexcept {
    reset();
    return *this;
  }

  template <typename U = T, std::enable_if_t<std::is_copy_constructible_v<U> && std::is_copy_assignable_v<U>, int> = 0>
  non_alloc_optional &operator=(const non_alloc_optional &other) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                                          std::is_nothrow_copy_assignable_v<T>) {
    if (this == &other) {
      return *this;
    }

    if (!other.has_) {
      reset();
      return *this;
    }

    if (has_) {
      **this = *other.ptr(); // T& = const T&
    } else {
      ::new (ptr()) T(*other.ptr());
      has_ = true;
    }
    return *this;
  }

  template <typename U = T, std::enable_if_t<!(std::is_copy_constructible_v<U> && std::is_copy_assignable_v<U>), int> = 0>
  non_alloc_optional &operator=(const non_alloc_optional &other) = delete;

  template <typename U = T, std::enable_if_t<std::is_move_constructible_v<U> && std::is_move_assignable_v<U>, int> = 0>
  non_alloc_optional &operator=(non_alloc_optional &&other) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                                     std::is_nothrow_move_assignable_v<T>) {
    if (this == &other) {
      return *this;
    }

    if (!other.has_) {
      reset();
      return *this;
    }

    if (has_) {
      **this = std::move(*other.ptr()); // T& = T&&
    } else {
      ::new (ptr()) T(std::move(*other.ptr()));
      has_ = true;
    }
    return *this;
  }

  template <typename U = T, std::enable_if_t<!(std::is_move_constructible_v<U> && std::is_move_assignable_v<U>), int> = 0>
  non_alloc_optional &operator=(non_alloc_optional &&other) = delete;

  template <class U, std::enable_if_t<std::is_constructible_v<T, U &&> && std::is_assignable_v<T &, U &&>, int> = 0>
  non_alloc_optional &operator=(U &&v) noexcept(noexcept(std::declval<T &>() = std::forward<U>(v)) &&
                                                noexcept(T(std::forward<U>(v)))) {
    if (has_) {
      **this = std::forward<U>(v);
    } else {
      ::new (ptr()) T(std::forward<U>(v));
      has_ = true;
    }
    return *this;
  }

  template <class... Args> T &emplace(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    reset();
    ::new (ptr()) T(std::forward<Args>(args)...);
    has_ = true;
    return **this;
  }

  template <typename U = T, std::enable_if_t<std::is_move_constructible_v<U> && std::is_swappable_v<U>, int> = 0>
  void swap(non_alloc_optional &other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
    using std::swap;
    if (has_ && other.has_) {
      swap(**this, *other);
    } else if (has_ && !other.has_) {
      other.emplace(std::move(**this));
      reset();
    } else if (!has_ && other.has_) {
      emplace(std::move(*other));
      other.reset();
    }
  }

  explicit operator bool() const noexcept { return has_; }
  bool has_value() const noexcept { return has_; }

  T *operator->() noexcept {
    assert(has_);
    return ptr();
  }

  const T *operator->() const noexcept {
    assert(has_);
    return ptr();
  }

  T &operator*() & noexcept {
    assert(has_);
    return *ptr();
  }

  const T &operator*() const & noexcept {
    assert(has_);
    return *ptr();
  }

  T &&operator*() && noexcept {
    assert(has_);
    return std::move(*ptr());
  }

  const T &&operator*() const && noexcept {
    assert(has_);
    return std::move(*ptr());
  }

#if SUB8_ENABLE_STL_TYPE
  T &value() & {
    if (!has_) {
      throw std::bad_optional_access();
    }
    return *ptr();
  }
  const T &value() const & {
    if (!has_) {
      throw std::bad_optional_access();
    }
    return *ptr();
  }

  T &&value() && {
    if (!has_) {
      throw std::bad_optional_access();
    }
    return std::move(*ptr());
  }

  const T &&value() const && {
    if (!has_) {
      throw std::bad_optional_access();
    }
    return std::move(*ptr());
  }
#endif

  template <class U> T value_or(U &&fallback) const & { return has_ ? **this : static_cast<T>(std::forward<U>(fallback)); }
  template <class U> T value_or(U &&fallback) && { return has_ ? std::move(**this) : static_cast<T>(std::forward<U>(fallback)); }

  void reset() noexcept {
    if (has_) {
      ptr()->~T();
      has_ = false;
    }
  }

private:
  using Storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  T *ptr() noexcept {
#if __cplusplus >= 201703L
    return std::launder(reinterpret_cast<T *>(&storage_));
#else
    return reinterpret_cast<T *>(&storage_);
#endif
  }

  const T *ptr() const noexcept {
#if __cplusplus >= 201703L
    return std::launder(reinterpret_cast<const T *>(&storage_));
#else
    return reinterpret_cast<const T *>(&storage_);
#endif
  }

  bool has_{false};
  Storage storage_{};
};

template <class T, class... Args> inline non_alloc_optional<T> make_non_alloc_optional(Args &&...args) {
  return sub8::non_alloc_optional<T>(in_place, std::forward<Args>(args)...);
}

template <template <class> class TOptionalContainer, typename TOptionalValue> class OptionalT {
public:
  using Type = TOptionalValue;
  using ValueType = Type;
  using InnerType = typename Type::Type;
  using InitType = typename Type::InitType;

  static constexpr BitSize MaxPossibleSize = Type::MaxPossibleSize + BitSize::from_bits(1);
  static constexpr BitSize MinPossibleSize = BitSize::from_bits(1);

  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  BitSize actual_size() const noexcept {
    const BitSize flag = BitSize::from_bits(1);
    if (!value_) {
      return flag;
    }
    if constexpr (sub8::details_t::has_actual_size_v<Type>) {
      return flag + Type::ActualSize;
    } else {
      return flag + (*value_).actual_size();
    }
  }

  OptionalT() noexcept = default;
  OptionalT(const OptionalT &) noexcept(std::is_nothrow_copy_constructible_v<TOptionalContainer<Type>>) = default;

#if !SUB8_ENABLE_INFALLIBLE
  explicit OptionalT(const InnerType &init) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, OptionalT, "sub8::OptionalT(const InnerType&)");
  }
#endif

#if !SUB8_ENABLE_INFALLIBLE
  template <typename U = InitType, std::enable_if_t<!std::is_same_v<U, InnerType>, int> = 0>
  explicit OptionalT(const InitType &init) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, OptionalT, "sub8::OptionalT(const InitType&)");
  }
#endif

#if !SUB8_ENABLE_INFALLIBLE
  template <typename U = InitType, std::enable_if_t<!std::is_same_v<U, InnerType>, int> = 0>
  static OptionalT make_or_throw(const InitType &v) SUB8_OPT_NO_EXCEPT {
    OptionalT out{};
    auto r = out.set_value(v);
    if (r != BitFieldResult::Ok) {
      SUB8_THROW_BITFIELD_ERROR(r, OptionalT, "sub8::OptionalT::make_or_throw(InitType)");
    }
    return out;
  }
#endif

  OptionalT &operator=(const OptionalT &) noexcept(std::is_nothrow_copy_assignable_v<TOptionalContainer<Type>>) = default;

  OptionalT &operator=(nullopt_t) noexcept(noexcept(value_.reset())) {
    reset();
    return *this;
  }

  OptionalT &operator=(const Type &v) noexcept(noexcept(value_ = v)) {
    value_ = v;
    return *this;
  }

  OptionalT &operator=(Type &&v) noexcept(noexcept(value_ = std::move(v))) {
    value_ = std::move(v);
    return *this;
  }

  static OptionalT make_none() noexcept { return OptionalT(); }

#if !SUB8_ENABLE_INFALLIBLE
  static OptionalT make_or_throw(const InnerType &v) SUB8_OPT_NO_EXCEPT {
    OptionalT out{};
    auto r = out.set_value(v);
    if (r != BitFieldResult::Ok) {
      SUB8_THROW_BITFIELD_ERROR(r, OptionalT, "sub8::OptionalT::make_or_throw(InnerType)");
    }
    return out;
  }
#endif

  SUB8_NO_DISCARD static BitFieldResult make(const InnerType &v, OptionalT &out) noexcept(noexcept(out.set_value(v))) {
    return out.set_value(v);
  }

  bool has_value() const noexcept { return value_.has_value(); }
  explicit operator bool() const noexcept { return has_value(); }

  void reset() noexcept(noexcept(value_.reset())) { value_.reset(); }

  const TOptionalContainer<Type> &optional() const noexcept { return value_; }
  TOptionalContainer<Type> &optional() noexcept { return value_; }

  const ValueType *try_get() const noexcept {
    if (!value_.has_value())
      return nullptr;
    return &(*value_);
  }

  ValueType *try_get() noexcept {
    if (!value_.has_value())
      return nullptr;
    return &(*value_);
  }

#if SUB8_ENABLE_INFALLIBLE
  template <typename U = ValueType, std::enable_if_t<std::is_nothrow_copy_assignable_v<U>, int> = 0>
  SUB8_NO_DISCARD BitFieldResult try_unwrap(U &out) const noexcept
#else
  SUB8_NO_DISCARD BitFieldResult try_unwrap(ValueType &out) const noexcept(std::is_nothrow_copy_assignable_v<ValueType>)
#endif
  {
    if (!value_.has_value()) {
      return BitFieldResult::ErrorOptionalValueIsEmpty;
    }
    out = *value_;
    return BitFieldResult::Ok;
  }

#if !SUB8_ENABLE_INFALLIBLE
  const ValueType &unwrap_value() const SUB8_OPT_NO_EXCEPT {
    if (!value_.has_value()) {
      SUB8_THROW_BITFIELD_ERROR(BitFieldResult::ErrorOptionalValueIsEmpty, OptionalT,
                                "sub8::OptionalT::unwrap_value() const");
    }
    return *value_;
  }
#endif

#if !SUB8_ENABLE_INFALLIBLE
  ValueType &unwrap_value() SUB8_OPT_NO_EXCEPT {
    if (!value_.has_value()) {
      SUB8_THROW_BITFIELD_ERROR(BitFieldResult::ErrorOptionalValueIsEmpty, OptionalT, "sub8::OptionalT::unwrap_value()");
    }
    return *value_;
  }
#endif

#if SUB8_ENABLE_INFALLIBLE
  template <typename U = Type,
            std::enable_if_t<noexcept(std::declval<U &>().set_value(std::declval<const InitType &>())), int> = 0,
            std::enable_if_t<noexcept(std::declval<TOptionalContainer<U> &>() = std::declval<U &&>()), int> = 0>
  SUB8_NO_DISCARD BitFieldResult set_value(const InitType &init) noexcept
#else
  SUB8_NO_DISCARD BitFieldResult set_value(const InitType &init) noexcept(noexcept(value_ = std::declval<Type &&>()))
#endif
  {
    Type tmp{};
    auto r = tmp.set_value(init);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    value_ = std::move(tmp);
    return BitFieldResult::Ok;
  }

#if SUB8_ENABLE_INFALLIBLE
  template <typename U = InitType,
            std::enable_if_t<!std::is_same_v<U, InnerType>, int> = 0,
            typename V = Type,
            std::enable_if_t<noexcept(std::declval<TOptionalContainer<V> &>() = std::declval<V &&>()), int> = 0,
            std::enable_if_t<noexcept(std::declval<V &>().set_value(std::declval<const InnerType &>())), int> = 0>
  SUB8_NO_DISCARD auto set_value(const InnerType &v) noexcept -> decltype(std::declval<Type &>().set_value(v), BitFieldResult{})
#else
  template <typename U = InitType, std::enable_if_t<!std::is_same_v<U, InnerType>, int> = 0>
  SUB8_NO_DISCARD auto set_value(const InnerType &v) noexcept(noexcept(value_ = std::declval<Type &&>()))
    -> decltype(std::declval<Type &>().set_value(v), BitFieldResult{})
#endif
  {
    Type tmp{};
    auto r = tmp.set_value(v);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    value_ = std::move(tmp);
    return BitFieldResult::Ok;
  }

#if SUB8_ENABLE_INFALLIBLE
  template <typename V = Type,
            std::enable_if_t<noexcept(std::declval<TOptionalContainer<V> &>().reset()), int> = 0,
            std::enable_if_t<noexcept(std::declval<TOptionalContainer<V> &>() = std::declval<V &&>()), int> = 0>
  SUB8_NO_DISCARD BitFieldResult set_value(const TOptionalContainer<InnerType> &init) noexcept
#else
  SUB8_NO_DISCARD BitFieldResult set_value(const TOptionalContainer<InnerType> &init) noexcept(
    noexcept(value_.reset()) && noexcept(value_ = std::declval<Type &&>()))
#endif
  {
    if (!init.has_value()) {
      value_.reset();
      return BitFieldResult::Ok;
    }
    if constexpr (std::is_same_v<InitType, InnerType>) {
      return set_value(static_cast<const InitType &>(init.value()));
    } else {
      return set_value(init.value());
    }
  }

#if SUB8_ENABLE_INFALLIBLE
  template <typename V = Type,
            std::enable_if_t<noexcept(std::declval<TOptionalContainer<V> &>() = std::declval<const TOptionalContainer<V> &>()), int> = 0>
  SUB8_NO_DISCARD BitFieldResult set_value(const TOptionalContainer<Type> &init) noexcept
#else
  SUB8_NO_DISCARD BitFieldResult set_value(const TOptionalContainer<Type> &init) noexcept(noexcept(set_value_unchecked(init)))
#endif
  {
    return set_value_unchecked(init);
  }

#if SUB8_ENABLE_INFALLIBLE
  template <typename V = Type,
            std::enable_if_t<noexcept(std::declval<TOptionalContainer<V> &>() = std::declval<const TOptionalContainer<V> &>()), int> = 0>
  BitFieldResult set_value_unchecked(const TOptionalContainer<Type> &init) noexcept
#else
  BitFieldResult set_value_unchecked(const TOptionalContainer<Type> &init) noexcept(noexcept(value_ = init))
#endif
  {
    value_ = init;
    return BitFieldResult::Ok;
  }

  bool operator==(const OptionalT &o) const noexcept {
    return value_.has_value() == o.value_.has_value() && (!value_.has_value() || (*value_ == *o.value_));
  }
  bool operator!=(const OptionalT &o) const noexcept { return !(*this == o); }

  bool operator==(const TOptionalContainer<Type> &o) const noexcept {
    return value_.has_value() == o.has_value() && (!value_.has_value() || (*value_ == *o));
  }
  bool operator!=(const TOptionalContainer<Type> &o) const noexcept { return !(*this == o); }

  bool operator==(const Type &o) const noexcept { return value_.has_value() && (*value_ == o); }
  bool operator!=(const Type &o) const noexcept { return !(*this == o); }

  template <typename U = Type, typename = decltype(std::declval<const U &>() == std::declval<const InnerType &>())>
  bool operator==(const InnerType &o) const noexcept {
    return value_.has_value() && (*value_ == o);
  }

  template <typename U = Type, typename = decltype(std::declval<const U &>() == std::declval<const InnerType &>())>
  bool operator!=(const InnerType &o) const noexcept {
    return !(*this == o);
  }

private:
  TOptionalContainer<Type> value_{};
};

template <typename TOptionalValue> using OptionalNonAlloc = OptionalT<sub8::non_alloc_optional, TOptionalValue>;

#if SUB8_ENABLE_STL_TYPE
template <typename TOptionalValue> using StdOptional = OptionalT<std::optional, TOptionalValue>;
#endif

#if SUB8_ENABLE_USE_NON_ALLOC_OPTIONAL_AS_DEFAULT
template <typename TOptionalValue> using Optional = OptionalT<sub8::non_alloc_optional, TOptionalValue>;
#else
template <typename TOptionalValue> using Optional = OptionalT<std::optional, TOptionalValue>;
#endif

// NOTE: This alias is convenient but can be confused with std::optional.
template <typename T> using optional = sub8::non_alloc_optional<T>;

template <typename Storage, template <class> class TOptionalContainer, typename TOptionalValue>
SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const OptionalT<TOptionalContainer, TOptionalValue> &opt_field) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  const auto *pv = opt_field.try_get();
  const uint8_t present = (pv != nullptr) ? 1u : 0u;

  auto r = bw.template put_bits<uint8_t>(present, 1);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  if (!present) {
    return BitFieldResult::Ok;
  }

  return write_field(bw, *pv);
}

template <typename Storage, template <class> class TOptionalContainer, typename TOptionalValue>
SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, OptionalT<TOptionalContainer, TOptionalValue> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  out.reset();

  static_assert(std::is_default_constructible_v<TOptionalValue>,
                "read_field(OptionalT<...,TOptionalValue>) requires TOptionalValue default ctor.");

  uint8_t present = 0;
  auto r = br.template get_bits<uint8_t>(present, 1);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  if (!present) {
    return BitFieldResult::Ok;
  }

  TOptionalValue tmp{};
  r = read_field(br, tmp);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  out = std::move(tmp);
  return BitFieldResult::Ok;
}

} // namespace sub8

#endif // SUB8_ENABLE_OPTIONAL_FIELDS
