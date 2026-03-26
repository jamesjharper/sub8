#include <catch2/catch_all.hpp>

#include <cstdint>
#include <type_traits>
#include <utility>

#include "./../src/sub8.h"

using namespace sub8;

// ------------------------------------------------------------
// SFINAE helpers (C++17)
// ------------------------------------------------------------
namespace test_macros {

template <typename, typename = void>
struct has_set_value : std::false_type {};
template <typename T>
struct has_set_value<T, std::void_t<decltype(std::declval<T &>().set_u8_value(std::declval<uint8_t>()))>> : std::true_type {};

template <typename, typename = void>
struct has_make_value : std::false_type {};
template <typename T>
struct has_make_value<T, std::void_t<decltype(T::make_u8_value(std::declval<uint8_t>()))>> : std::true_type {};

} // namespace test_macros

// ------------------------------------------------------------
// Dummy FieldTypes to probe your trait gating
// ------------------------------------------------------------

// Safe: value() returns a stable lvalue reference.
struct SafeU8Field {
  using Type = uint8_t;
  using InitType = uint8_t;
  using ValueType = uint8_t;
  using StorageType = uint8_t;

  static constexpr BitSize ActualSize = BitSize::from_bits(8);
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept { return ActualSize; }

  SafeU8Field() noexcept = default;

  BitFieldResult set_value(uint8_t v) noexcept {
    v_ = v;
    return BitFieldResult::Ok;
  }

  const uint8_t &value() const noexcept { return v_; }
  bool operator==(const SafeU8Field &o) const noexcept { return v_ == o.v_; }
  bool operator!=(const SafeU8Field &o) const noexcept { return !(*this == o); }
private:
  uint8_t v_{0};
};

// Unsafe for set_*_value: value() returns by value.
struct ByValueU8Field {
  using Type = uint8_t;
  using InitType = uint8_t;
  using ValueType = uint8_t;
  using StorageType = uint8_t;

  static constexpr BitSize ActualSize = BitSize::from_bits(8);
  static constexpr BitSize MaxPossibleSize = ActualSize;
  static constexpr BitSize MinPossibleSize = ActualSize;

  BitSize actual_size() const noexcept { return ActualSize; }

  ByValueU8Field() noexcept = default;

  BitFieldResult set_value(uint8_t v) noexcept {
    v_ = v;
    return BitFieldResult::Ok;
  }

  uint8_t value() const noexcept { return v_; }

    bool operator==(const ByValueU8Field &o) const noexcept { return v_ == o.v_; }
  bool operator!=(const ByValueU8Field &o) const noexcept { return !(*this == o); }
private:
  uint8_t v_{0};
};

// ------------------------------------------------------------
// Variant schemas
// ------------------------------------------------------------
#define SAFE_VARIANT_CASES(X)   X(u8, 1u, uint8_t, SafeU8Field)
#define UNSAFE_VARIANT_CASES(X) X(u8, 1u, uint8_t, ByValueU8Field)

SUB8_DECLARE_VARIANT_FIELD(SafeVariant, SAFE_VARIANT_CASES);
SUB8_DECLARE_VARIANT_FIELD(UnsafeVariant, UNSAFE_VARIANT_CASES);

// ------------------------------------------------------------
// Tests
// ------------------------------------------------------------

TEST_CASE("Variant: safe field enables set_*_value + make_*_value, and field getter/setter works",
          "[sub8][macros][variant][safe]") {
  // set_u8_value and make_u8_value should exist
  static_assert(test_macros::has_set_value<SafeVariant>::value, "SafeVariant::set_u8_value(uint8_t) should exist");
  static_assert(test_macros::has_make_value<SafeVariant>::value, "SafeVariant::make_u8_value(uint8_t) should exist");

  SafeVariant v = SafeVariant::make_null();
  REQUIRE(v.is_null());

  // Use the value-based setter
  REQUIRE(v.set_u8_value(uint8_t{42}) == BitFieldResult::Ok);
  REQUIRE(v.is_u8());

  // Always-safe FieldType pointer getter
  const SafeU8Field *f = v.get_u8();
  REQUIRE(f != nullptr);

  // try_get_*_value copies out
  uint8_t out = 0;
  REQUIRE(v.try_get_u8_value(out));
  REQUIRE(out == 42);

  // FieldType setter (copy)
  SafeU8Field tmp;
  REQUIRE(tmp.set_value(uint8_t{77}) == BitFieldResult::Ok);
  REQUIRE(v.set_u8(tmp) == BitFieldResult::Ok);
  REQUIRE(v.is_u8());
  REQUIRE(v.try_get_u8_value(out));
  REQUIRE(out == 77);

  // FieldType setter (move)
  SafeU8Field tmp2;
  REQUIRE(tmp2.set_value(uint8_t{99}) == BitFieldResult::Ok);
  REQUIRE(v.set_u8(std::move(tmp2)) == BitFieldResult::Ok);
  REQUIRE(v.try_get_u8_value(out));
  REQUIRE(out == 99);

  // Factory: make_u8_value
  SafeVariant v2 = SafeVariant::make_u8_value(uint8_t{11});
  REQUIRE(v2.is_u8());
  REQUIRE(v2.try_get_u8_value(out));
  REQUIRE(out == 11);
}

TEST_CASE("Variant: by-value field disables set_*_value + make_*_value; must set via FieldType",
          "[sub8][macros][variant][unsafe]") {
  // set_u8_value and make_u8_value should NOT exist
  static_assert(!test_macros::has_set_value<UnsafeVariant>::value,
                "UnsafeVariant::set_u8_value(uint8_t) must NOT exist (value() by value)");
  static_assert(!test_macros::has_make_value<UnsafeVariant>::value,
                "UnsafeVariant::make_u8_value(uint8_t) must NOT exist (value() by value)");

  UnsafeVariant v = UnsafeVariant::make_null();
  REQUIRE(v.is_null());

  // Must set via FieldType setter
  ByValueU8Field tmp;
  REQUIRE(tmp.set_value(uint8_t{55}) == BitFieldResult::Ok);
  REQUIRE(v.set_u8(tmp) == BitFieldResult::Ok);
  REQUIRE(v.is_u8());

  // get_u8 gives FieldType*
  const ByValueU8Field *f = v.get_u8();
  REQUIRE(f != nullptr);

  // try_get still works because it assigns out from value()
  uint8_t out = 0;
  REQUIRE(v.try_get_u8_value(out));
  REQUIRE(out == 55);
}
