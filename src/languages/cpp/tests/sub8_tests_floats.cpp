// tests/sub8_floats.cpp
#include <catch2/catch_all.hpp>

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <limits>
#include <cmath>
#include <vector>
#include <string>


#include "./../src/sub8_floats.h"
#include "test_helpers.h"

using namespace sub8;

// ------------------------------------------------------------
// IO helpers (write/read + size verification)
// ------------------------------------------------------------

#define REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padded_bits, expected_size_bits, expected_vec)                                            \
  BoundedBitWriter<128> bw;                                                                                                                \
  {                                                                                                                                        \
    const auto expected_result = BitFieldResult::Ok;                                                                                       \
    bw.put_padding(padded_bits);                                                                                                           \
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

// ---- Read helper ----
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
    /* Custom check for output vs expected */                                                                                              \
    CHECK_OUTPUT;                                                                                                                          \
                                                                                                                                           \
    const auto expected_cursor_position = BitSize::from_bits((input_bit_size) + (padded_bits));                                            \
    REQUIRE(br.cursor_position().bit_size() == expected_cursor_position.bit_size());                                                       \
                                                                                                                                           \
    REQUIRE_THAT(std::string(error::to_string(r)), Catch::Matchers::Equals(std::string(error::to_string(expected_result))));               \
  } while (false)

// Float bit-compare read helper (float)
#define REQUIRE_THAT_FLOAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                    \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), do {                                              \
    REQUIRE(bits_from_float((output).value()) == bits_from_float((expected_value)));                                                       \
    if (std::isnan((expected_value))) {                                                                                                    \
      REQUIRE(std::isnan((output).value()));                                                                                               \
    }                                                                                                                                      \
  } while (false))



#define REQUIRE_THAT_DOUBLE_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                   \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), do {                                               \
    REQUIRE(bits_from_float((output).value()) == bits_from_float((expected_value)));                                                      \
    if (std::isnan((expected_value))) {                                                                                                     \
      REQUIRE(std::isnan((output).value()));                                                                                                \
    }                                                                                                                                       \
  } while (false))

// ------------------------------------------------------------
// Size API helpers (float-safe: sizes only, not MinValue/MaxValue)
// ------------------------------------------------------------

#define ASSERT_FIXED_LENGTH_BIT_FIELD_SIZES_ONLY(T_SUT, instance, ExpectedBits)                                                             \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::ActualSize.bit_size() == (ExpectedBits));                                                                               \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (ExpectedBits));                                                                          \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == (ExpectedBits));                                                                          \
                                                                                                                                           \
    REQUIRE((instance).actual_size().bit_size() == (ExpectedBits));                                                                        \
    REQUIRE((instance).min_possible_size().bit_size() == (ExpectedBits));                                                                  \
    REQUIRE((instance).max_possible_size().bit_size() == (ExpectedBits));                                                                  \
  } while (false)

// clang-format off

// ============================================================
// Float16
// ============================================================
#if SUB8_ENABLE_FLOAT16

TEST_CASE("Float16ValueField: 16-bit value", "[Float16ValueField][16-bit]") {
  auto [test_name, value, expected_read, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, float, float, uint8_t, size_t, std::vector<uint8_t>>({

          // normals
          {"1.0f", 1.0f, 1.0f, 0, 16, {0b00111100, 0b00000000}},  // 0x3C00
          {"2.0f", 2.0f, 2.0f, 0, 16, {0b01000000, 0b00000000}},  // 0x4000

          // pi quantizes to 3.140625 in float16 (0x4248)
          {"pi (quantizes to 3.140625)", 3.14159265359f, 3.140625f, 0, 16, {0b01000010, 0b01001000}},  // 0x4248

          {"1.0009765625 should round down", 1.0009765625f, 1.0009765625f, 0, 16, {0b00111100, 0b00000001}},  // 0x3C01
          {"1.0009765625 should round up", 1.00146484375f, 1.001953125f, 0, 16, {0b00111100, 0b00000010}},    // 0x3C02
          {"1.99951171875 should carry over", 1.99951171875f, 2.0f, 0, 16, {0b01000000, 0b00000000}},         // 0x4000

          // f16 subnormals + min normal
          {"smallest positive f16 subnormal",
           5.960464477539063e-08f,
           5.960464477539063e-08f,
           0,
           16,
           {0b00000000, 0b00000001}},  // 0x0001
          {"largest positive f16 subnormal",
           6.097555160522461e-05f,
           6.097555160522461e-05f,
           0,
           16,
           {0b00000011, 0b11111111}},                                                                        // 0x03FF
          {"min positive normal f16", 6.103515625e-05f, 6.103515625e-05f, 0, 16, {0b00000100, 0b00000000}},  // 0x0400

          // inf
          {"inf",
           std::numeric_limits<float>::infinity(),
           std::numeric_limits<float>::infinity(),
           0,
           16,
           {0b01111100, 0b00000000}},                                                       // 0x7C00
          {"max finite f16 (65504)", 65504.0f, 65504.0f, 0, 16, {0b01111011, 0b11111111}},  // 0x7BFF

          // nan
          {"qNaN with payload (mapped)",
           float_from_bits(0x7FC02000),
           float_from_bits(0x7FC02000),
           0,
           16,
           {0b01111110, 0b00000001}},
          {"qNaN with payload (un-mapped)",
           float_from_bits(0x7FC12345),
           float_from_bits(0x7FC02000),
           0,
           16,
           {0b01111110, 0b00000001}},
          {"sNaN with payload (quieted)",
           float_from_bits(0x7F800001),
           std::numeric_limits<float>::quiet_NaN(),
           0,
           16,
           {0b01111110, 0b00000000}},

          // float32 subnormals underflow to 0 in float16
          {"float32 smallest subnormal -> 0", float_from_bits(0x00000001), 0.0f, 0, 16, {0b00000000, 0b00000000}},
          {"float32 largest subnormal -> 0", float_from_bits(0x007FFFFF), 0.0f, 0, 16, {0b00000000, 0b00000000}},
          {"float16 smallest positive subnormal",
           float16_subnormal_value(1),
           float16_subnormal_value(1),
           0,
           16,
           {0b00000000, 0b00000001}},  // 0x0001
          {"float16 largest positive subnormal",
           float16_subnormal_value(1023),
           float16_subnormal_value(1023),
           0,
           16,
           {0b00000011, 0b11111111}}  // 0x03FF
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Float16ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    // size API checks
    ASSERT_FIXED_LENGTH_BIT_FIELD_SIZES_ONLY(T_SUT, input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_FLOAT_READ_IS_OK(output, padding, expected_read, bw.storage(), expected_len);

    if (std::isnan(value)) {
      REQUIRE(std::isnan(output.value()));
    }
  }
}

TEST_CASE("Float16ValueField: set_value overflow returns ErrorValueOverflow and preserves prior value",
          "[Float16ValueField][overflow][set_value]") {
  using T = Float16ValueField;
  T f{};

  REQUIRE(f.set_value(1.0f) == BitFieldResult::Ok);
  const auto before = bits_from_float(f.value());

  // 70000f is > max finite float16 (65504)
  const auto r = f.set_value(70000.0f);
  REQUIRE(r == BitFieldResult::ErrorValueOverflow);

  // Intended invariant: preserve previous value on error
  REQUIRE(bits_from_float(f.value()) == before);
}

#endif  // SUB8_ENABLE_FLOAT16

// ============================================================
// Float32
// ============================================================
#if SUB8_ENABLE_FLOAT32

TEST_CASE("Float32ValueField: 32-bit value", "[Float32ValueField][32-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, float, uint8_t, size_t, std::vector<uint8_t>>({
          {"1.0f", 1.0f, 0, 32, {0b00111111, 0b10000000, 0b00000000, 0b00000000}},
          {"3.14159265359f", 3.14159265359f, 0, 32, {0b01000000, 0b01001001, 0b00001111, 0b11011011}},
          {"1.0009765625f should round up", 1.0009765625f, 0, 32, {0b00111111, 0b10000000, 0b00100000, 0b00000000}},
          {"smallest positive subnormal",
           float_from_bits(0x00000001),
           0,
           32,
           {0b00000000, 0b00000000, 0b00000000, 0b00000001}},
          {"largest positive subnormal",
           float_from_bits(0x007FFFFF),
           0,
           32,
           {0b00000000, 0b01111111, 0b11111111, 0b11111111}},
          {"qNaN with payload", float_from_bits(0x7FC12345), 0, 32, {0b01111111, 0b11000001, 0b00100011, 0b01000101}},
          {"sNaN with payload", float_from_bits(0x7F800001), 0, 32, {0b01111111, 0b10000000, 0b00000000, 0b00000001}},
          {"inf", std::numeric_limits<float>::infinity(), 0, 32, {0b01111111, 0b10000000, 0b00000000, 0b00000000}},
          {"max", std::numeric_limits<float>::max(), 0, 32, {0b01111111, 0b01111111, 0b11111111, 0b11111111}},
          {"min", std::numeric_limits<float>::min(), 0, 32, {0b00000000, 0b10000000, 0b00000000, 0b00000000}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Float32ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_SIZES_ONLY(T_SUT, input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_FLOAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_FLOAT32

// ============================================================
// Float64
// ============================================================
#if SUB8_ENABLE_FLOAT64

TEST_CASE("Float64ValueField: 64-bit value", "[Float64ValueField][64-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, double, uint8_t, size_t, std::vector<uint8_t>>({
          {"1.0",
           1.0,
           0,
           64,
           {0b00111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
          {"inf",
           std::numeric_limits<double>::infinity(),
           0,
           64,
           {0b01111111, 0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
          {"max",
           std::numeric_limits<double>::max(),
           0,
           64,
           {0b01111111, 0b11101111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}},
          {"min",
           std::numeric_limits<double>::min(),
           0,
           64,
           {0b00000000, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Float64ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_SIZES_ONLY(T_SUT, input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_DOUBLE_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_FLOAT64

// clang-format on
