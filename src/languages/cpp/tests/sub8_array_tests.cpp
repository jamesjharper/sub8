// sub8_tests_arrays.cpp
//
// Complete test file for sub8 array fields (Delimited / Prefixed / ThreePlusPrefixed)
// Includes:
//  - Existing “PathBitField” style byte-pattern tests (from your snippet)
//  - Added size-accounting tests for actual_size()/MinPossibleSize/MaxPossibleSize
//  - Added error-path tests (overflow / too many elements / state after failure)
//  - Uses macros that also assert actual_size() matches emitted bits

#include <cstdint>
#include <cstring>
#include <type_traits>

#include <catch2/catch_all.hpp>

#include "./../src/sub8_arrays.h"
#include "test_helpers.h"

using namespace sub8;

// ------------------------------------------------------------
// Test helpers / macros
// ------------------------------------------------------------

#define REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padded_bits, expected_size_bits, expected_vec)                                            \
  BoundedBitWriter<128> bw;                                                                                                                \
  {                                                                                                                                        \
    const auto expected_result = BitFieldResult::Ok;                                                                                       \
    bw.put_padding((padded_bits));                                                                                                         \
    const auto r = write_field(bw, (input));                                                                                               \
                                                                                                                                           \
    /* Compare byte buffer */                                                                                                              \
    REQUIRE(to_binary_string(bw.storage()) == to_binary_string((expected_vec)));                                                           \
                                                                                                                                           \
    /* Written size includes padding */                                                                                                    \
    REQUIRE(bw.size().bit_size() == (expected_size_bits) + (padded_bits));                                                                 \
                                                                                                                                           \
    /* Field reports its own written size (excluding padding) */                                                                           \
    REQUIRE((input).actual_size().bit_size() == (expected_size_bits));                                                                     \
                                                                                                                                           \
    /* Compare result using Catch matchers */                                                                                              \
    REQUIRE_THAT(std::string(error::to_string(r)), Catch::Matchers::Equals(std::string(error::to_string(expected_result))));               \
  }

#define REQUIRE_THAT_READ_CORE_IS_OK(output, padded_bits, input_bytes, input_bit_size, CHECK_OUTPUT)                                       \
  do {                                                                                                                                     \
    INFO("Input Bytes        = " << to_binary_string((input_bytes)));                                                                      \
    INFO("Input BitSize      = " << (input_bit_size));                                                                                     \
                                                                                                                                           \
    BoundedBitReader<128> br((input_bytes), BitSize::from_bits((input_bit_size) + (padded_bits)));                                         \
    br.set_cursor_position(BitSize::from_bits((padded_bits)));                                                                             \
                                                                                                                                           \
    const auto r = read_field(br, (output));                                                                                               \
    INFO("Read Cursor Pos    = " << br.cursor_position().bit_size());                                                                      \
    const auto expected_result = BitFieldResult::Ok;                                                                                       \
                                                                                                                                           \
    CHECK_OUTPUT;                                                                                                                          \
                                                                                                                                           \
    const auto expected_cursor_position = BitSize::from_bits((input_bit_size) + (padded_bits));                                            \
    REQUIRE(br.cursor_position().bit_size() == expected_cursor_position.bit_size());                                                       \
                                                                                                                                           \
    REQUIRE_THAT(std::string(error::to_string(r)), Catch::Matchers::Equals(std::string(error::to_string(expected_result))));               \
  } while (false)

#define REQUIRE_THAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                          \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), REQUIRE((output).value() == (expected_value)))

// ------------------------------------------------------------
// Tests
// ------------------------------------------------------------

#if SUB8_ENABLE_ARRAY_FIELDS

// ------------------------------------------------------------
// Byte-patterns
// ------------------------------------------------------------

TEST_CASE("PathBitField: 3-bit paths * 1 (3/4 bits usable)", "[PathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
      {"no groups", {}, 0, 1, {0b0}},
      {"1 group", {1}, 0, 4, {0b1'001'0000}},

      {"no groups non-aligned", {}, 2, 1, {0b00'0'00000}},
      {"1 group non-aligned", {1}, 2, 4, {0b00'1'001'00}},
    }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BufferArray<3, 0, 1, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 3-bit paths * 1 (signed)", "[PathBitField][signed][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<int8_t>, uint8_t, size_t, std::vector<uint8_t>>({
      {"no groups", {}, 0, 1, {0b0'0000000}},
      {"1 group", {1}, 0, 4, {0b1'010'0000}},

      {"no groups non-aligned", {}, 2, 1, {0b00'0'00000}},
      {"1 group non-aligned", {-1}, 2, 4, {0b00'1'001'00}},
    }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = SignedBufferArray<3, 0, 1, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 3-bit paths * 2", "[PathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>(
      {{"no groups", {}, 0, 1, {0b0}}, {"1 group", {1}, 0, 5, {0b1'001'0'000}}, {"2 group", {1, 2}, 0, 8, {0b1'001'1'010}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BufferArray<3, 0, 2, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 3-bit paths * 3", "[PathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>(
      {{"no groups", {}, 0, 1, {0b0}}, {"1 group", {1}, 0, 5, {0b1'001'0'000}}, {"2 group", {1, 2}, 0, 9, {0b1'001'1'010, 0b0'0000000}},
        {"3 group", {1, 2, 3}, 0, 12, {0b1'001'1'010, 0b1'011'0000}},

        {"no groups non-aligned", {}, 2, 1, {0b00'000000}}, {"1 group non-aligned", {1}, 2, 5, {0b00100100}},
        {"2 group non-aligned", {1, 2}, 2, 9, {0b00100110, 0b10000000}},
        {"3 group non-aligned", {1, 2, 3}, 2, 12, {0b00100110, 0b10101100}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BufferArray<3, 0, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("NonEmptyPathBitField: 3-bit paths min 1 max 3", "[NonEmptyPathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({{"1 group", {1}, 0, 4, {0b001'00000}},
      {"2 group", {1, 2}, 0, 8, {0b001'1'010'0}}, {"3 group", {1, 2, 3}, 0, 11, {0b001'1'010'1, 0b011'00000}},

      {"1 group non-aligned", {1}, 2, 4, {0b00001000}}, {"2 group non-aligned", {1, 2}, 2, 8, {0b00001101, 0b00000000}},
      {"3 group non-aligned", {1, 2, 3}, 2, 11, {0b00001101, 0b01011000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BufferArray<3, 1, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

// Prefixed arrays

TEST_CASE("PrefixedArray: 10-bit values * 3", "[PrefixedArray][unsigned][10-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint16_t>, uint8_t, size_t, std::vector<uint8_t>>({
      {"no groups", {}, 0, 2, {0b00000000}},
      {"1 group", {1}, 0, 12, {0b01000000, 0b00010000}},
      {"1 group max", {1023}, 0, 12, {0b01111111, 0b11110000}},
      {"2 group", {2, 1}, 0, 22, {0b10000000, 0b00100000, 0b00000100}},
      {"2 group max", {1023, 1023}, 0, 22, {0b10111111, 0b11111111, 0b11111100}},
      {"3 group", {3, 2, 1}, 0, 32, {0b11000000, 0b00110000, 0b00001000, 0b00000001}},
      {"3 group max", {1023, 1023, 1023}, 0, 32, {0b11111111, 0b11111111, 0b11111111, 0b11111111}},
    }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BufferArray<10, 0, 3, ArrayEncoding::Prefixed>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("NonEmptyPrefixedArray: 8-bit min 2 max 3", "[NonEmptyPrefixedArray][unsigned][8-bit]") {
  auto [test_name, in_value, out_value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint8_t>, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
      {"empty", {}, {0, 0}, 0, 17, {0b0'0000000, 0b0'0000000, 0b0'0000000}},
      {"partially used", {1, 2}, {1, 2}, 0, 17, {0b0'0000000, 0b1'0000001, 0b0'0000000}},
      {"full", {1, 2, 3}, {1, 2, 3}, 0, 25, {0b1'0000000, 0b1'0000001, 0b0'0000001, 0b1'0000000}},
    }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BufferArray<8, 2, 3, ArrayEncoding::Prefixed>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(in_value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, out_value, bw.storage(), expected_len);
  }
}

TEST_CASE("FixedSizeArray: 8-bit fixed size array * 3", "[FixedSizeArray][unsigned][8-bit]") {
  auto [test_name, in_value, out_value, padding, expected_len, expected_bytes] =
    GENERATE(table<std::string, std::vector<uint8_t>, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
      {"empty", {}, {0, 0, 0}, 0, 24, {0b00000000, 0b00000000, 0b00000000}},
      {"partially used", {1, 2}, {1, 2, 0}, 0, 24, {0b00000001, 0b00000010, 0b00000000}},
      {"full", {1, 2, 3}, {1, 2, 3}, 0, 24, {0b00000001, 0b00000010, 0b00000011}},
    }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = FixedSizeBufferArray<8, 3>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(in_value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, out_value, bw.storage(), expected_len);
  }
}

TEST_CASE("Array operator== should not compare stale tail bytes", "[Array][equality]") {
  using F = BufferArray<3, 0, 3>;

  F a{};
  F b{};

  REQUIRE(a.set_value(std::vector<uint8_t>{1, 2, 3}) == BitFieldResult::Ok);
  REQUIRE(b.set_value(std::vector<uint8_t>{7, 7, 7}) == BitFieldResult::Ok);

  REQUIRE(a.set_value(std::vector<uint8_t>{1}) == BitFieldResult::Ok);
  REQUIRE(b.set_value(std::vector<uint8_t>{1}) == BitFieldResult::Ok);

  REQUIRE(a.size() == 1);
  REQUIRE(b.size() == 1);
  REQUIRE(a[0] == 1);
  REQUIRE(b[0] == 1);

  REQUIRE(a == b);
}

TEST_CASE("Array ThreePlusPrefixed chooses Delimited when (Max-Min)<=3", "[Array][strategy]") {
  auto value = std::vector<uint8_t>{1, 2, 3};
  auto expected_bytes = std::vector<uint8_t>{0b1'001'1'010, 0b1'011'0000};
  size_t expected_len = 12;

  using T_SUT = BufferArray<3, 0, 3>; // ThreePlusPrefixed default => Delimited here

  T_SUT input{};
  T_SUT output{};

  REQUIRE(input.set_value(value) == BitFieldResult::Ok);

  REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, 0, expected_len, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_len);
}

TEST_CASE("Array ThreePlusPrefixed chooses Prefixed when (Max-Min)>3", "[Array][strategy]") {
  auto value = std::vector<uint8_t>{1, 2, 3, 4};
  auto expected_bytes = std::vector<uint8_t>{0b100'001'01, 0b0'011'100'0};
  size_t expected_len = 15;

  using T_SUT = BufferArray<3, 0, 4>; // ThreePlusPrefixed default => Prefixed here

  T_SUT input{};
  T_SUT output{};

  REQUIRE(input.set_value(value) == BitFieldResult::Ok);

  REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, 0, expected_len, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_len);
}

TEST_CASE("Delimited sizes: Min=0 Max=1 terminator vs implied end", "[Array][Delimited][sizes]") {
  using F = BufferArray<3, 0, 1, ArrayEncoding::Delimited>; // ValueRange=1

  SECTION("empty: terminator only") {
    F f{};
    REQUIRE(f.set_value((const uint8_t *)nullptr, 0) == BitFieldResult::Ok);

    REQUIRE(f.actual_size().bit_size() == 1);
    REQUIRE(F::MinPossibleSize.bit_size() == 1);
    // Max: elements (3) + delimiter bits (ValueRange = 1)
    REQUIRE(F::MaxPossibleSize.bit_size() == 4);
  }

  SECTION("full: cont + data, no terminator") {
    F f{};
    uint8_t v[1] = {1};
    REQUIRE(f.set_value(v, 1) == BitFieldResult::Ok);
    REQUIRE(f.actual_size().bit_size() == 4); // 1 + 3
  }
}

TEST_CASE("Delimited sizes: Min>0 requires terminator unless at max", "[Array][Delimited][sizes]") {
  using F = BufferArray<3, 1, 3, ArrayEncoding::Delimited>; // ValueRange=2

  SECTION("size=0 => effective_n=1 includes terminator") {
    F f{};
    REQUIRE(f.set_value((const uint8_t *)nullptr, 0) == BitFieldResult::Ok);
    // 1 element (3 bits) + terminator (1)
    REQUIRE(f.actual_size().bit_size() == 4);
  }

  SECTION("size=3 => implied end (no terminator), overhead=ValueRange") {
    F f{};
    uint8_t v[3] = {1, 2, 3};
    REQUIRE(f.set_value(v, 3) == BitFieldResult::Ok);
    // elements: 9 bits, overhead: ValueRange=2
    REQUIRE(f.actual_size().bit_size() == 11);
  }
}

TEST_CASE("Prefixed sizes: MinElements padding included", "[Array][Prefixed][sizes]") {
  using F = BufferArray<8, 2, 3, ArrayEncoding::Prefixed>; // ValueRange=1 => prefix=1

  SECTION("empty => effective_n=2") {
    F f{};
    REQUIRE(f.set_value((const uint8_t *)nullptr, 0) == BitFieldResult::Ok);

    REQUIRE(f.actual_size().bit_size() == 17); // 1 + 16
    REQUIRE(F::MinPossibleSize.bit_size() == 17);
    REQUIRE(F::MaxPossibleSize.bit_size() == 25); // 1 + 24
  }

  SECTION("len=3") {
    F f{};
    uint8_t v[3] = {1, 2, 3};
    REQUIRE(f.set_value(v, 3) == BitFieldResult::Ok);
    REQUIRE(f.actual_size().bit_size() == 25);
  }
}

TEST_CASE("actual_size monotonicity (Delimited)", "[Array][sizes]") {
  using F = BufferArray<3, 0, 3, ArrayEncoding::Delimited>;
  F f{};

  uint8_t a1[1] = {1};
  uint8_t a2[2] = {1, 2};
  uint8_t a3[3] = {1, 2, 3};

  REQUIRE(f.set_value((const uint8_t *)nullptr, 0) == BitFieldResult::Ok);
  auto s0 = f.actual_size().bit_size();

  REQUIRE(f.set_value(a1, 1) == BitFieldResult::Ok);
  auto s1 = f.actual_size().bit_size();

  REQUIRE(f.set_value(a2, 2) == BitFieldResult::Ok);
  auto s2 = f.actual_size().bit_size();

  REQUIRE(f.set_value(a3, 3) == BitFieldResult::Ok);
  auto s3 = f.actual_size().bit_size();

  REQUIRE(s0 <= s1);
  REQUIRE(s1 <= s2);
  REQUIRE(s2 <= s3);
}

TEST_CASE("Array constexpr size bounds", "[Array][constexpr][sizes]") {
  using D = BufferArray<3, 0, 3, ArrayEncoding::Delimited>; // ValueRange=3

  STATIC_REQUIRE(D::MinPossibleSize.bit_size() == 1);
  STATIC_REQUIRE(D::MaxPossibleSize.bit_size() == (3 * 3 + 3)); // 9 + 3

  using P = BufferArray<8, 2, 3, ArrayEncoding::Prefixed>; // prefix=1
  STATIC_REQUIRE(P::MinPossibleSize.bit_size() == (1 + 2 * 8));
  STATIC_REQUIRE(P::MaxPossibleSize.bit_size() == (1 + 3 * 8));

  STATIC_REQUIRE(P::MinElements == 2);
  STATIC_REQUIRE(P::MaxElements == 3);
  STATIC_REQUIRE(P::ValueRange == 1);
  STATIC_REQUIRE(P::LengthPrefixBitWidth == 1);
}

// ------------------------------------------------------------
// Error semantics
// ------------------------------------------------------------

TEST_CASE("Array errors: overflow + too many elements + state", "[Array][errors]") {
  using F = BufferArray<3, 0, 3, ArrayEncoding::Delimited>; // values must fit 0..7

  SECTION("overflow returns ErrorValueOverflow and leaves object in valid state") {
    F f{};
    uint8_t v[1] = {8}; // overflow
    REQUIRE(f.set_value(v, 1) == BitFieldResult::ErrorValueOverflow);
    // Recommended invariant: no partial update
    REQUIRE(f.size() == 0);
  }

  SECTION("too many elements returns ErrorTooManyElements and leaves object in valid state") {
    F f{};
    uint8_t v[4] = {1, 2, 3, 4};
    REQUIRE(f.set_value(v, 4) == BitFieldResult::ErrorTooManyElements);
    REQUIRE(f.size() == 0);
  }

  SECTION("null pointer with non-zero len returns ErrorInvalidBitFieldValue") {
    F f{};
    REQUIRE(f.set_value((const uint8_t *)nullptr, 1) == BitFieldResult::ErrorInvalidBitFieldValue);
    REQUIRE(f.size() == 0);
  }
}

#endif // SUB8_ENABLE_ARRAY_FIELDS
