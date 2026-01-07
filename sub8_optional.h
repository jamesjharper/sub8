#pragma once

#ifndef SUB8_ENABLE_OPTIONAL_FIELDS
#define SUB8_ENABLE_OPTIONAL_FIELDS 1
#endif

#include "sub8_basic.h"
#include "sub8_errors.h"
#include "sub8_io.h"

#include <optional>
#include <new>  // placement new
#include <type_traits>
#include <utility>

#if SUB8_ENABLE_OPTIONAL_FIELDS

#include <type_traits>
#include <utility>
#include <new>
#include <cassert>
#include <optional>  // only for std::bad_optional_access (you can replace if you want)

namespace sub8 {

struct nullopt_t {
  explicit constexpr nullopt_t(int) {}
};
inline constexpr nullopt_t nullopt{0};

struct in_place_t {
  explicit constexpr in_place_t() = default;
};
inline constexpr in_place_t in_place{};

template<class T> class non_alloc_optional {
 public:
  using value_type = T;

  // constructors
  constexpr non_alloc_optional() noexcept = default;
  constexpr non_alloc_optional(nullopt_t) noexcept {}

  template<typename U = T, typename std::enable_if<std::is_copy_constructible<U>::value, int>::type = 0>
  non_alloc_optional(const non_alloc_optional &other) {
    if (other.has_) {
      ::new (ptr()) T(*other.ptr());
      has_ = true;
    }
  }

  // delete copy ctor if not copy-constructible (prevents implicit generation)
  template<typename U = T, typename std::enable_if<!std::is_copy_constructible<U>::value, int>::type = 0>
  non_alloc_optional(const non_alloc_optional &other) = delete;

  // move ctor (only if T is move-constructible)
  template<typename U = T, typename std::enable_if<std::is_move_constructible<U>::value, int>::type = 0>
  non_alloc_optional(non_alloc_optional &&other) noexcept(std::is_nothrow_move_constructible<T>::value) {
    if (other.has_) {
      ::new (ptr()) T(std::move(*other.ptr()));
      has_ = true;
    }
  }

  template<typename U = T, typename std::enable_if<!std::is_move_constructible<U>::value, int>::type = 0>
  non_alloc_optional(non_alloc_optional &&other) = delete;

  // in_place ctor
  template<class... Args>
  explicit non_alloc_optional(in_place_t, Args &&...args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
    ::new (ptr()) T(std::forward<Args>(args)...);
    has_ = true;
  }

  template<class U, typename std::enable_if<std::is_constructible<T, U &&>::value, int>::type = 0>
  explicit non_alloc_optional(U &&v) noexcept(std::is_nothrow_constructible<T, U &&>::value) {
    ::new (ptr()) T(std::forward<U>(v));
    has_ = true;
  }

  // destructor
  ~non_alloc_optional() { reset(); }

  // assignment
  non_alloc_optional &operator=(nullopt_t) noexcept {
    reset();
    return *this;
  }

  // copy assignment (only if T copy-constructible + copy-assignable)
  template<
      typename U = T,
      typename std::enable_if<std::is_copy_constructible<U>::value && std::is_copy_assignable<U>::value, int>::type = 0>
  non_alloc_optional &operator=(const non_alloc_optional &other) {
    if (this == &other)
      return *this;

    if (!other.has_) {
      reset();
      return *this;
    }

    if (has_) {
      **this = *other;
    } else {
      ::new (ptr()) T(*other.ptr());
      has_ = true;
    }
    return *this;
  }

  template<typename U = T,
           typename std::enable_if<!(std::is_copy_constructible<U>::value && std::is_copy_assignable<U>::value),
                                   int>::type = 0>
  non_alloc_optional &operator=(const non_alloc_optional &other) = delete;

  // move assignment (only if T move-constructible + move-assignable)
  template<
      typename U = T,
      typename std::enable_if<std::is_move_constructible<U>::value && std::is_move_assignable<U>::value, int>::type = 0>
  non_alloc_optional &operator=(non_alloc_optional &&other) noexcept(std::is_nothrow_move_constructible<T>::value &&
                                                                     std::is_nothrow_move_assignable<T>::value) {
    if (this == &other)
      return *this;

    if (!other.has_) {
      reset();
      return *this;
    }

    if (has_) {
      **this = std::move(*other);
    } else {
      ::new (ptr()) T(std::move(*other.ptr()));
      has_ = true;
    }
    return *this;
  }

  template<typename U = T,
           typename std::enable_if<!(std::is_move_constructible<U>::value && std::is_move_assignable<U>::value),
                                   int>::type = 0>
  non_alloc_optional &operator=(non_alloc_optional &&other) = delete;

  // assign from value (only if constructible and assignable)
  template<class U, typename std::enable_if<
                        std::is_constructible<T, U &&>::value && std::is_assignable<T &, U &&>::value, int>::type = 0>
  non_alloc_optional &operator=(U &&v) {
    if (has_) {
      **this = std::forward<U>(v);
    } else {
      ::new (ptr()) T(std::forward<U>(v));
      has_ = true;
    }
    return *this;
  }

  // emplace
  template<class... Args> T &emplace(Args &&...args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
    reset();
    ::new (ptr()) T(std::forward<Args>(args)...);
    has_ = true;
    return **this;
  }

  // swap (only if T move-constructible and swappable)
  template<typename U = T,
           typename std::enable_if<std::is_move_constructible<U>::value && std::is_swappable<U>::value, int>::type = 0>
  void swap(non_alloc_optional &other) noexcept(std::is_nothrow_move_constructible<T>::value &&
                                                std::is_nothrow_swappable<T>::value) {
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

  // observers
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

  T &value() & {
    if (!has_)
      throw std::bad_optional_access();
    return *ptr();
  }
  const T &value() const & {
    if (!has_)
      throw std::bad_optional_access();
    return *ptr();
  }
  T &&value() && {
    if (!has_)
      throw std::bad_optional_access();
    return std::move(*ptr());
  }
  const T &&value() const && {
    if (!has_)
      throw std::bad_optional_access();
    return std::move(*ptr());
  }

  template<class U> T value_or(U &&fallback) const & {
    return has_ ? **this : static_cast<T>(std::forward<U>(fallback));
  }

  template<class U> T value_or(U &&fallback) && {
    return has_ ? std::move(**this) : static_cast<T>(std::forward<U>(fallback));
  }

  // modifiers
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

template<class T, class... Args> non_alloc_optional<T> make_optional(Args &&...args) {
  return sub8::non_alloc_optional<T>(in_place, std::forward<Args>(args)...);
}

template<template<class> class TOptional, typename TField> class OptionalBitFieldT {
 public:
  TOptional<TField> opt{};

  using Type = TField;
  using InnerType = typename TField::Type;
  using ValueType = TField;
  using InitType = TOptional<InnerType>;

  OptionalBitFieldT() noexcept = default;
  OptionalBitFieldT(const OptionalBitFieldT &) noexcept = default;

  explicit OptionalBitFieldT(const InitType &init) {
    auto r = set_value(init);
    assert(r == BitFieldResult::Ok);
    (void) r;
  }

  static OptionalBitFieldT make_none() { return OptionalBitFieldT(); }

  static OptionalBitFieldT make(InnerType t) {
    auto n = OptionalBitFieldT();
    n.set_value(t);
    return n;
  }

  OptionalBitFieldT &operator=(const OptionalBitFieldT &) noexcept = default;

  bool has_value() const noexcept { return opt.has_value(); }
  explicit operator bool() const noexcept { return has_value(); }

  void reset() noexcept { opt.reset(); }

  const TOptional<TField>& value() const noexcept {
    return opt;
  }

  TOptional<TField>& value() noexcept {
    return opt;
  }

  const ValueType &unwrap_value() const noexcept {
    assert(opt.has_value());
    return *opt;
  }

  ValueType &unwrap_value() noexcept {
    assert(opt.has_value());
    return *opt;
  }

  ValueType value_or(ValueType fallback) const noexcept { return opt.has_value() ? *opt : fallback; }

  BitFieldResult set_value(const typename TField::InitType &init) noexcept {
    TField tmp{};
    auto r = tmp.set_value(init);
    if (r != BitFieldResult::Ok) {
      return r;
    }

    opt = TOptional<TField>(tmp);
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(const InitType &init) noexcept {
    if (!init.has_value()) {
      opt.reset();
      return BitFieldResult::Ok;
    }

    return set_value(init.value());
  }

  BitFieldResult set_value(const TOptional<TField> &init) noexcept {
    opt = init;
    return BitFieldResult::Ok;
  }

  OptionalBitFieldT &operator=(const typename TField::InitType &v) noexcept {
    opt = OptionalBitFieldT::make(v);
    return *this;
  }

  OptionalBitFieldT &operator=(const InitType &v) noexcept {
    opt = v;
    return *this;
  }

  OptionalBitFieldT &operator=(const Type &v) noexcept {
    opt = v;
    return *this;
  }

  OptionalBitFieldT &operator=(Type &&v) noexcept {
    opt = std::move(v);
    return *this;
  }

  bool operator==(const OptionalBitFieldT &o) const noexcept {
    return opt.has_value() == o.opt.has_value() && (!opt.has_value() || (*opt == *o.opt));
  }
  bool operator!=(const OptionalBitFieldT &o) const noexcept { return !(*this == o); }

  bool operator==(const TOptional<TField> &o) const noexcept {
    return opt.has_value() == o.has_value() && (!opt.has_value() || (*opt == *o));
  }
  bool operator!=(const TOptional<TField> &o) const noexcept { return !(*this == o); }

  bool operator==(const Type &o) const noexcept { return opt.has_value() && (*opt == o); }
  bool operator!=(const Type &o) const noexcept { return !(*this == o); }

  template<typename U = TField, typename = decltype(std::declval<const U &>() == std::declval<const InnerType &>())>
  bool operator==(const InnerType &o) const noexcept {
    return opt.has_value() && (*opt == o);
  }

  template<typename U = TField, typename = decltype(std::declval<const U &>() == std::declval<const InnerType &>())>
  bool operator!=(const InnerType &o) const noexcept {
    return !(*this == o);
  }
};

template<typename TField> using StdOptionalBitField = OptionalBitFieldT<std::optional, TField>;

template<typename TField> using OptionalBitField = OptionalBitFieldT<sub8::non_alloc_optional, TField>;

template<typename T> using optional = sub8::non_alloc_optional<T>;

template<typename Storage, template<class> class TOptional, typename TField>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const OptionalBitFieldT<TOptional, TField> &opt) {
  const uint8_t present = opt.has_value() ? 1u : 0u;
  auto r = bw.template put_bits<uint8_t>(present, 1);
  if (r != BitFieldResult::Ok)
    return r;

  if (!present)
    return BitFieldResult::Ok;

  return write_field(bw, opt.unwrap_value());
}

template<typename Storage, template<class> class TOptional, typename TField>
inline BitFieldResult read_field(BasicBitReader<Storage> &br, OptionalBitFieldT<TOptional, TField> &out) {
  out.reset();

  static_assert(std::is_default_constructible_v<TField>,
                "read_field(OptionalBitFieldT<...,TField>) currently requires TField default ctor.");

  uint8_t present = 0;
  if (!br.template get_bits<uint8_t>(present, 1))
    return BitFieldResult::ErrorExpectedMoreBits;

  if (!present)
    return BitFieldResult::Ok;

  TField tmp{};
  auto r = read_field(br, tmp);
  if (r != BitFieldResult::Ok)
    return r;

  out = std::move(tmp);
  return BitFieldResult::Ok;
}

#endif

}  // namespace sub8
