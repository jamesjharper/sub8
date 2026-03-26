// sub8_primitives_tests.cpp
#include <cmath> // std::isnan
#include <cstdint>
#include <cstring>
#include <limits> // std::numeric_limits (used in some helpers/tests)
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_all.hpp>

#include "./../src/sub8_primitives.h"

#include "test_helpers.h"

using namespace sub8;

// ------------------------------------------------------------
// IO helpers
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

// ---- Size verification helpers ----

// Verifies the *instance* size APIs match expectations and the static mins/maxes.
// For fixed-length fields: expected_actual_bits == ExpectedMin == ExpectedMax.
#define ASSERT_BIT_SIZES_FIXED(T_SUT, obj, expected_bits)                                                                                  \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::ActualSize.bit_size() == (expected_bits));                                                                              \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (expected_bits));                                                                         \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == (expected_bits));                                                                         \
                                                                                                                                           \
    REQUIRE((obj).actual_size().bit_size() == (expected_bits));                                                                            \
    REQUIRE((obj).min_possible_size().bit_size() == (expected_bits));                                                                      \
    REQUIRE((obj).max_possible_size().bit_size() == (expected_bits));                                                                      \
  } while (false)

// For variable-length fields, you usually know the *actual* encoded size per test row.
// This also verifies static Min/MaxPossibleSize are correct at the type level.
#define ASSERT_BIT_SIZES_VARIABLE(T_SUT, obj, expected_actual_bits, expected_min_bits, expected_max_bits)                                  \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (expected_min_bits));                                                                     \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == (expected_max_bits));                                                                     \
                                                                                                                                           \
    REQUIRE((obj).min_possible_size().bit_size() == (expected_min_bits));                                                                  \
    REQUIRE((obj).max_possible_size().bit_size() == (expected_max_bits));                                                                  \
    REQUIRE((obj).actual_size().bit_size() == (expected_actual_bits));                                                                     \
  } while (false)

// Convenience: verify write size matches actual_size() (ignores padding).
// Call this after set_value() and before write_field().
#define ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(obj, expected_bits)                                                                          \
  do {                                                                                                                                     \
    REQUIRE((obj).actual_size().bit_size() == (expected_bits));                                                                            \
  } while (false)

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

#define REQUIRE_THAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                          \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), REQUIRE((output).value() == (expected_value)))

#define REQUIRE_THAT_OPT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                      \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), REQUIRE((output) == (expected_value)))

#define REQUIRE_THAT_FLOAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                    \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), do {                                              \
    REQUIRE(bits_from_float((output).value()) == bits_from_float((expected_value)));                                                       \
    if (std::isnan((expected_value))) {                                                                                                    \
      REQUIRE(std::isnan((output).value()));                                                                                               \
    }                                                                                                                                      \
  } while (false))

// ------------------------------------------------------------
// Size + limits assertion helpers
// ------------------------------------------------------------

// Assert constexpr min/max values are calculated correctly
#define ASSERT_BITFIELD_MINMAX(T_SUT, ExpectedMinValue, ExpectedMaxValue)                                                                  \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::MaxValue == static_cast<typename T_SUT::Type>(ExpectedMaxValue));                                                       \
    REQUIRE(T_SUT::MinValue == static_cast<typename T_SUT::Type>(ExpectedMinValue));                                                       \
  } while (false)

// Fixed-length fields: constants + min/max value
#define ASSERT_FIXED_LENGTH_BIT_FIELD_STATIC(T_SUT, ExpectedTotalUsableBits, ExpectedMinValue, ExpectedMaxValue)                           \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::ActualSize.bit_size() == (ExpectedTotalUsableBits));                                                                    \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == (ExpectedTotalUsableBits));                                                               \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (ExpectedTotalUsableBits));                                                               \
    ASSERT_BITFIELD_MINMAX(T_SUT, (ExpectedMinValue), (ExpectedMaxValue));                                                                 \
  } while (false)

// Fixed-length fields: instance size methods match constants
#define ASSERT_FIXED_LENGTH_BIT_FIELD_METHODS(instance, ExpectedTotalUsableBits)                                                           \
  do {                                                                                                                                     \
    REQUIRE((instance).actual_size().bit_size() == (ExpectedTotalUsableBits));                                                             \
    REQUIRE((instance).max_possible_size().bit_size() == (ExpectedTotalUsableBits));                                                       \
    REQUIRE((instance).min_possible_size().bit_size() == (ExpectedTotalUsableBits));                                                       \
  } while (false)

// Variable-length fields: validate core constexpr properties + min/max bounds
#define ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT, ExpectedMaxGroupCount, ExpectedTotalUsableBits, ExpectedMinValue, ExpectedMaxValue) \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::MaxSegmentsCount == (ExpectedMaxGroupCount));                                                                           \
    REQUIRE(T_SUT::BitWidth == (ExpectedTotalUsableBits));                                                                                 \
    /* min is first segment + 1 continuation bit (canonical encoding) */                                                                   \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (T_SUT::segment_length(0) + 1));                                                          \
    /* max is all data bits plus continuation bits for all but last group */                                                               \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == ((ExpectedTotalUsableBits) + (ExpectedMaxGroupCount - 1)));                               \
    ASSERT_BITFIELD_MINMAX(T_SUT, (ExpectedMinValue), (ExpectedMaxValue));                                                                 \
  } while (false)

// Variable-length fields: instance methods for min/max and actual_size per value
#define ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(instance, ExpectedActualBits)                                                             \
  do {                                                                                                                                     \
    REQUIRE((instance).min_possible_size().bit_size() == std::remove_reference_t<decltype(instance)>::MinPossibleSize.bit_size());         \
    REQUIRE((instance).max_possible_size().bit_size() == std::remove_reference_t<decltype(instance)>::MaxPossibleSize.bit_size());         \
    REQUIRE((instance).actual_size().bit_size() == (ExpectedActualBits));                                                                  \
  } while (false)

// ------------------------------------------------------------
// Tests
// ------------------------------------------------------------

// clang-format off

// --------------------
// NullValue
// --------------------
TEST_CASE("NullValue: read/write always errors + sizes", "[NullValue]") {
  using T = sub8::NullValue;

  SECTION("write_field returns ErrorCanNotWriteNullValue and does not advance") {
    BoundedBitWriter<128> bw;
    T n{};
    const auto r = write_field(bw, n);
    REQUIRE(r == BitFieldResult::ErrorCanNotWriteNullValue);
    REQUIRE(bw.size().bit_size() == 0);
  }

  SECTION("read_field returns ErrorCanNotReadNullValue and does not advance") {
    std::vector<uint8_t> bytes{0xFF};
    auto buffer = ByteBufferView(bytes.data(), 8);
    BitReader br(buffer, BitSize::from_bits(8));
    T n{};
    const auto before = br.cursor_position().bit_size();
    const auto r = read_field(br, n);
    REQUIRE(r == BitFieldResult::ErrorCanNotReadNullValue);
    REQUIRE(br.cursor_position().bit_size() == before);
  }

  SECTION("size constants/methods are zero") {
    T n{};
    REQUIRE(T::ActualSize.bit_size() == 0);
    REQUIRE(T::MinPossibleSize.bit_size() == 0);
    REQUIRE(T::MaxPossibleSize.bit_size() == 0);
    REQUIRE(n.actual_size().bit_size() == 0);
    REQUIRE(n.min_possible_size().bit_size() == 0);
    REQUIRE(n.max_possible_size().bit_size() == 0);
  }
}

// --------------------
// Boolean
// --------------------
#if SUB8_ENABLE_BOOL
TEST_CASE("Boolean: write/read + size APIs", "[Boolean]") {
  using T_SUT = sub8::Boolean;

  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, bool, uint8_t, size_t, std::vector<uint8_t>>(
          {
              {"false aligned", false, 0, 1, {0b00000000}},
              {"true aligned",  true,  0, 1, {0b10000000}},
              {"true non-aligned (pad=3)", true, 3, 1, {0b00010000}},
              {"false non-aligned (pad=3)", false, 3, 1, {0b00000000}},
          }));

  DYNAMIC_SECTION(test_name) {
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    // Static size constants
    REQUIRE(T_SUT::ActualSize.bit_size() == 1);
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == 1);
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == 1);

    // Instance size methods
    REQUIRE(input.actual_size().bit_size() == 1);
    REQUIRE(input.min_possible_size().bit_size() == 1);
    REQUIRE(input.max_possible_size().bit_size() == 1);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif // SUB8_ENABLE_BOOL

// --------------------
// Integer tests
// --------------------

TEST_CASE("Integer: 8-bit Unsigned field", "[Integer][8-bit][unsigned][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"single byte", 5u, 0, 8, {0b00000101}},
           {"single byte non-aligned", 255u, 3, 8, {0b00011111, 0b11100000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = UnsignedInteger<8>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /* Expected Bit Width*/ 8,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 255
    );
    ASSERT_FIXED_LENGTH_BIT_FIELD_METHODS(input, 8);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Integer: 8-bit signed field", "[Integer][8-bit][signed][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"positive number", 1, 0, 8, {0b00000010}},
           {"negative number", -1, 0, 8, {0b00000001}},
           {"max positive number non byte aligned", 127, 3, 8, {0b00011111, 0b11000000}},
           {"min negative number non byte aligned", -128, 3, 8, {0b00011111, 0b11100000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = SignedInteger<8>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /* Expected Bit Width*/ 8,
      /* Expected Min Value*/ -128,
      /* Expected Max Value*/ 127
    );
    ASSERT_FIXED_LENGTH_BIT_FIELD_METHODS(input, 8);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Integer: 3-bit Unsigned field", "[Integer][3-bit][unsigned][sub-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"sub byte size", 5u, 0, 3, {0b10100000}},
           {"sub byte size non-aligned", 7u, 3, 3, {0b00011100}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = UnsignedInteger<3>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /* Expected Bit Width*/ 3,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 7
    );
    ASSERT_FIXED_LENGTH_BIT_FIELD_METHODS(input, 3);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Integer: 10-bit Unsigned field", "[Integer][10-bit][unsigned][over-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"cross byte boundary", 1010u, 0, 10, {0b11111100, 0b10000000}},
           {"cross byte boundary non-aligned", 1023u, 3, 10, {0b00011111, 0b11111000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = UnsignedInteger<10>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /* Expected Bit Width*/ 10,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 1023
    );
    ASSERT_FIXED_LENGTH_BIT_FIELD_METHODS(input, 10);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Integer: 26-bit Unsigned field", "[Integer][26-bit][unsigned][multi-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"multi sub byte size", 67000000u, 0, 26, {0b11111111, 0b10010101, 0b10110000, 0b00000000}},
           {"multi sub byte size non-aligned", 67000000u, 3, 26, {0b00011111, 0b11110010, 0b10110110, 0b00000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = UnsignedInteger<26>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /* Expected Bit Width*/ 26,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 67108863
    );
    ASSERT_FIXED_LENGTH_BIT_FIELD_METHODS(input, 26);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

// Arithmetic behavior tests remain as-is (already strong coverage)

TEST_CASE("Integer<4,false> unsigned add/sub/mul/div") {
  using U4 = sub8::Integer<4, false>; // range 0..15

  SECTION("set_value bounds") {
    U4 x;
    REQUIRE(x.set_value(0) == BitFieldResult::Ok);
    REQUIRE(x.set_value(15) == BitFieldResult::Ok);
    REQUIRE(x.set_value(16) == BitFieldResult::ErrorValueOverflow);
  }

  SECTION("add happy path") {
    U4 x; REQUIRE(x.set_value(3) == BitFieldResult::Ok);
    REQUIRE(x.add(4) == BitFieldResult::Ok);
    REQUIRE(static_cast<U4::Type>(x) == 7);
  }

  SECTION("add overflow") {
    U4 x; REQUIRE(x.set_value(15) == BitFieldResult::Ok);
    REQUIRE(x.add(1) == BitFieldResult::ErrorValueOverflow);
    REQUIRE(static_cast<U4::Type>(x) == 15);
  }

  SECTION("sub happy path") {
    U4 x; REQUIRE(x.set_value(10) == BitFieldResult::Ok);
    REQUIRE(x.sub(7) == BitFieldResult::Ok);
    REQUIRE(static_cast<U4::Type>(x) == 3);
  }

  SECTION("sub underflow") {
    U4 x; REQUIRE(x.set_value(0) == BitFieldResult::Ok);
    REQUIRE(x.sub(1) == BitFieldResult::ErrorValueOverflow);
    REQUIRE(static_cast<U4::Type>(x) == 0);
  }

  SECTION("mul happy path") {
    U4 x; REQUIRE(x.set_value(3) == BitFieldResult::Ok);
    REQUIRE(x.mul(5) == BitFieldResult::Ok);
    REQUIRE(static_cast<U4::Type>(x) == 15);
  }

  SECTION("mul overflow") {
    U4 x; REQUIRE(x.set_value(4) == BitFieldResult::Ok);
    REQUIRE(x.mul(4) == BitFieldResult::ErrorValueOverflow); // 16 > 15
    REQUIRE(static_cast<U4::Type>(x) == 4);
  }

  SECTION("div happy path") {
    U4 x; REQUIRE(x.set_value(15) == BitFieldResult::Ok);
    REQUIRE(x.div(3) == BitFieldResult::Ok);
    REQUIRE(static_cast<U4::Type>(x) == 5);
  }

  SECTION("div by zero") {
    U4 x; REQUIRE(x.set_value(7) == BitFieldResult::Ok);
    REQUIRE(x.div(0) == BitFieldResult::ErrorInvalidBitFieldValue);
    REQUIRE(static_cast<U4::Type>(x) == 7);
  }
}

TEST_CASE("Integer<4,true> signed add/sub/mul/div") {
  using S4 = sub8::Integer<4, true>; // range -8..7

  SECTION("set_value bounds") {
    S4 x;
    REQUIRE(x.set_value(-8) == BitFieldResult::Ok);
    REQUIRE(x.set_value(7) == BitFieldResult::Ok);
    REQUIRE(x.set_value(8) == BitFieldResult::ErrorValueOverflow);
    REQUIRE(x.set_value(-9) == BitFieldResult::ErrorValueOverflow);
  }

  SECTION("add happy path") {
    S4 x; REQUIRE(x.set_value(-3) == BitFieldResult::Ok);
    REQUIRE(x.add(5) == BitFieldResult::Ok);
    REQUIRE(static_cast<S4::Type>(x) == 2);
  }

  SECTION("add overflow high") {
    S4 x; REQUIRE(x.set_value(7) == BitFieldResult::Ok);
    REQUIRE(x.add(1) == BitFieldResult::ErrorValueOverflow);
    REQUIRE(static_cast<S4::Type>(x) == 7);
  }

  SECTION("add overflow low") {
    S4 x; REQUIRE(x.set_value(-8) == BitFieldResult::Ok);
    REQUIRE(x.add(-1) == BitFieldResult::ErrorValueOverflow);
    REQUIRE(static_cast<S4::Type>(x) == -8);
  }

  SECTION("sub happy path") {
    S4 x; REQUIRE(x.set_value(3) == BitFieldResult::Ok);
    REQUIRE(x.sub(5) == BitFieldResult::Ok);
    REQUIRE(static_cast<S4::Type>(x) == -2);
  }

  SECTION("sub overflow high") {
    S4 x; REQUIRE(x.set_value(7) == BitFieldResult::Ok);
    REQUIRE(x.sub(-1) == BitFieldResult::ErrorValueOverflow); // 7 - (-1) = 8
    REQUIRE(static_cast<S4::Type>(x) == 7);
  }

  SECTION("sub overflow low") {
    S4 x; REQUIRE(x.set_value(-8) == BitFieldResult::Ok);
    REQUIRE(x.sub(1) == BitFieldResult::ErrorValueOverflow); // -9
    REQUIRE(static_cast<S4::Type>(x) == -8);
  }

  SECTION("mul happy path (pos*pos)") {
    S4 x; REQUIRE(x.set_value(2) == BitFieldResult::Ok);
    REQUIRE(x.mul(3) == BitFieldResult::Ok);
    REQUIRE(static_cast<S4::Type>(x) == 6);
  }

  SECTION("mul happy path (neg*pos)") {
    S4 x; REQUIRE(x.set_value(-2) == BitFieldResult::Ok);
    REQUIRE(x.mul(3) == BitFieldResult::Ok);
    REQUIRE(static_cast<S4::Type>(x) == -6);
  }

  SECTION("mul happy path (neg*neg)") {
    S4 x; REQUIRE(x.set_value(-2) == BitFieldResult::Ok);
    REQUIRE(x.mul(-3) == BitFieldResult::Ok);
    REQUIRE(static_cast<S4::Type>(x) == 6);
  }

  SECTION("mul overflow high") {
    S4 x; REQUIRE(x.set_value(4) == BitFieldResult::Ok);
    REQUIRE(x.mul(2) == BitFieldResult::ErrorValueOverflow); // 8
    REQUIRE(static_cast<S4::Type>(x) == 4);
  }

  SECTION("mul overflow low") {
    S4 x; REQUIRE(x.set_value(-8) == BitFieldResult::Ok);
    REQUIRE(x.mul(2) == BitFieldResult::ErrorValueOverflow); // -16
    REQUIRE(static_cast<S4::Type>(x) == -8);
  }

  SECTION("div happy path") {
    S4 x; REQUIRE(x.set_value(6) == BitFieldResult::Ok);
    REQUIRE(x.div(3) == BitFieldResult::Ok);
    REQUIRE(static_cast<S4::Type>(x) == 2);
  }

  SECTION("div by zero") {
    S4 x; REQUIRE(x.set_value(6) == BitFieldResult::Ok);
    REQUIRE(x.div(0) == BitFieldResult::ErrorInvalidBitFieldValue);
    REQUIRE(static_cast<S4::Type>(x) == 6);
  }

  SECTION("div min/-1 overflow (2's complement corner)") {
    // -8 / -1 = 8 (out of range)
    S4 x; REQUIRE(x.set_value(-8) == BitFieldResult::Ok);
    REQUIRE(x.div(-1) == BitFieldResult::ErrorValueOverflow);
    REQUIRE(static_cast<S4::Type>(x) == -8);
  }
}

TEST_CASE("Integer<8,true> signed operations avoid UB on wider underlying") {
  using S8 = sub8::Integer<8, true>; // range -128..127

  SECTION("add boundary") {
    S8 x; REQUIRE(x.set_value(127) == BitFieldResult::Ok);
    REQUIRE(x.add(1) == BitFieldResult::ErrorValueOverflow);
  }

  SECTION("mul boundary") {
    S8 x; REQUIRE(x.set_value(64) == BitFieldResult::Ok);
    REQUIRE(x.mul(2) == BitFieldResult::ErrorValueOverflow); // 128 out of range
  }
}

// --------------------
// VbrInteger tests
// --------------------

// VbrInteger with groups smaller than 1 byte
TEST_CASE("VbrInteger: 3-bit groups * 4 (12 usable bits, 15 written max)",
          "[VbrInteger][3-bit][unsigned][sub-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint16_t, uint8_t, size_t, std::vector<uint8_t>>({

          // --- 1 GROUP (3 usable bits => 4 written) ---
          {"1 group min", 0u, 0, 4, {0b0000'0000}},
          {"1 group", 5u, 0, 4, {0b0101'0000}},
          {"1 group max", MAX_BITS(3), 0, 4, {0b0111'0000}},

          // --- 2 GROUPS (6 usable bits => 8 written) ---
          {"2 groups min", MAX_BITS(3) + 1, 0, 8, {0b1001'0000}},
          {"2 groups", 40u, 0, 8, {0b1101'0000}},
          {"2 groups max", MAX_BITS(6), 0, 8, {0b1111'0111}},

          // --- 3 GROUPS (9 usable bits => 12 written) ---
          {"3 groups min ", MAX_BITS(6) + 1, 0, 12, {0b1001'1000, 0b0000'0000}},
          {"3 groups", 360u, 0, 12, {0b1101'1101, 0b0000'0000}},
          {"3 groups max", MAX_BITS(9), 0, 12, {0b1111'1111, 0b0111'0000}},

          // --- 4 GROUPS (12 usable bits => 15 written, last group no cont) ---
          {"4 groups min", MAX_BITS(9) + 1, 0, 15, {0b1001'1000, 0b1000'0000}},
          {"4 groups", 621u, 0, 15, {0b1001'1001, 0b1101'1010}},
          {"4 groups max", MAX_BITS(12), 0, 15, {0b1111'1111, 0b1111'1110}},

          // --- NON-BYTE-ALIGNED CASES (padding=1) ---
          {"1 group min non-aligned", 0u, 1, 4, {0b0'000'0000}},
          {"1 group non-aligned", 5u, 1, 4, {0b0'010'1000}},
          {"1 group max non-aligned", MAX_BITS(3), 1, 4, {0b0'011'1000}},

          {"2 groups min non-aligned", MAX_BITS(3) + 1, 1, 8, {0b0'100'100'0, 0b00'000000}},
          {"2 groups non-aligned", 40u, 1, 8, {0b0'110'100'0, 0b00'000000}},
          {"2 groups max non-aligned", MAX_BITS(6), 1, 8, {0b0'111'101'1, 0b10'000000}},

          {"3 groups min non-aligned", MAX_BITS(6) + 1, 1, 12, {0b0'100'110'0, 0b00'000'000}},
          {"3 groups non-aligned", 360u, 1, 12, {0b0'110'111'0, 0b10'000'000}},
          {"3 groups max non-aligned", MAX_BITS(9), 1, 12, {0b0'111'111'1, 0b10'111'000}},

          {"4 groups min non-aligned", MAX_BITS(9) + 1, 1, 15, {0b0'1001'100, 0b0'1000'000}},
          {"4 groups non-aligned", 621u, 1, 15, {0b0'1001'100, 0b1'1101'101}},
          {"4 groups max non-aligned", MAX_BITS(12), 1, 15, {0b0'1111'111, 0b1'1111'111}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VbrInteger</* signed */ false, 3,3,3,3>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /*ExpectedMaxGroupCount*/ 4,
      /*ExpectedTotalUsableBits*/ 12,
      /*ExpectedMinValue*/ 0,
      /*ExpectedMaxValue*/ MAX_BITS(12));

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VbrInteger: 7-bit groups * 2 (14 usable bits, 15 written max)",
          "[VbrInteger][7-bit][unsigned][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint16_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"single group", 5u, 0, 8, {0b00000101}},
           {"full group", MAX_BITS(14), 0, 15, {0b11111111, 0b11111110}},
           {"single group non-aligned", 5u, 3, 8, {0b00000000, 0b10100000}},
           {"full group non-aligned", MAX_BITS(14), 3, 15, {0b00011111, 0b11111111, 0b11000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VbrInteger<false, 7, 7>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /*ExpectedMaxGroupCount*/ 2,
      /*ExpectedTotalUsableBits*/ 14,
      /*ExpectedMinValue*/ 0,
      /*ExpectedMaxValue*/ MAX_BITS(14));

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VbrInteger: 7-bit groups * 2 (14 usable bits, signed)",
          "[VbrInteger][7-bit][signed][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int16_t, uint8_t, size_t, std::vector<uint8_t>>({
          {"single group", 5, 0, 8, {0b00001010}},
          {"full group max", 8191, 0, 15, {0b11111111, 0b11111100}},
          {"single group non-aligned", -5, 3, 8, {0b00000001, 0b00100000}},
          {"full group min non-aligned", -8192, 3, 15, {0b00011111, 0b11111111, 0b11000000}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VbrInteger</* signed */ true, 7, 7>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /*ExpectedMaxGroupCount*/ 2,
      /*ExpectedTotalUsableBits*/ 14,
      /*ExpectedMinValue*/ -8192,
      /*ExpectedMaxValue*/ 8191);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VbrInteger: 7-bit groups * 4 (28 usable bits, 31 written max)",
          "[VbrInteger][7-bit][unsigned][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>({

      {"1 group min", 0u, 0, 8, {0b00000000}},
      {"1 group", 5u, 0, 8, {0b00000101}},
      {"1 group max", MAX_BITS(7), 0, 8, {0b01111111}},

      {"2 groups min", MAX_BITS(7) + 1, 0, 16, {0b10000001, 0b00000000}},
      {"2 groups", 1000u, 0, 16, {0b10000111, 0b01101000}},
      {"2 groups max", MAX_BITS(14), 0, 16, {0b11111111, 0b01111111}},

      {"3 groups min", MAX_BITS(14) + 1, 0, 24, {0b10000001, 0b10000000, 0b00000000}},
      {"3 groups", 100000u, 0, 24, {0b10000110, 0b10001101, 0b00100000}},
      {"3 groups max", MAX_BITS(21), 0, 24, {0b11111111, 0b11111111, 0b01111111}},

      {"4 groups min", MAX_BITS(21) + 1, 0, 31, {0b10000001, 0b10000000, 0b10000000, 0b00000000}},
      {"4 groups", 6000000u, 0, 31, {0b10000010, 0b11101110, 0b10011011, 0b00000000}},
      {"4 groups max", MAX_BITS(28), 0, 31, {0b11111111, 0b11111111, 0b11111111, 0b11111110}},

      // Non Byte aligned
      {"1 group min non-aligned", 0u, 3, 8, {0b00000000, 0b00000000}},
      {"1 group non-aligned", 5u, 3, 8, {0b00000000, 0b10100000}},
      {"1 group max non-aligned", MAX_BITS(7), 3, 8, {0b00001111, 0b11100000}},

      {"2 groups min non-aligned", MAX_BITS(7) + 1, 3, 16, {0b00010000, 0b00100000, 0b00000000}},
      {"2 groups non-aligned", 1000u, 3, 16, {0b00010000, 0b11101101, 0b00000000}},
      {"2 groups max non-aligned", MAX_BITS(14), 3, 16, {0b00011111, 0b11101111, 0b11100000}},

      {"3 groups min non-aligned", MAX_BITS(14) + 1, 3, 24, {0b00010000, 0b00110000, 0b00000000, 0b00000000}},
      {"3 groups non-aligned", 100000u, 3, 24, {0b00010000, 0b11010001, 0b10100100, 0b00000000}},
      {"3 groups max non-aligned", MAX_BITS(21), 3, 24, {0b00011111, 0b11111111, 0b11101111, 0b11100000}},

      {"4 groups min non-aligned", MAX_BITS(21) + 1, 3, 31, {0b00010000, 0b00110000, 0b00010000, 0b00000000, 0b00000000}},
      {"4 groups non-aligned", 6000000u, 3, 31, {0b00010000, 0b01011101, 0b11010011, 0b01100000, 0b00000000}},
      {"4 groups max non-aligned", MAX_BITS(28), 3, 31, {0b00011111, 0b11111111, 0b11111111, 0b11111111, 0b11000000}},
  }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VbrInteger</* signed */ false, 7,7,7,7>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /*ExpectedMaxGroupCount*/ 4,
      /*ExpectedTotalUsableBits*/ 28,
      /*ExpectedMinValue*/ 0,
      /*ExpectedMaxValue*/ MAX_BITS(28));

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VbrInteger: 9-bit groups * 2 (18 usable bits, 19 written max)",
          "[VbrInteger][9-bit][unsigned][multi-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>({

          {"1 group min", 0u, 0, 10, {0b00000000, 0b00'000000}},
          {"1 group", 489u, 0, 10, {0b01111010, 0b01'000000}},
          {"1 group max", MAX_BITS(9), 0, 10, {0b01111111, 0b11'000000}},

          {"2 groups min", MAX_BITS(9) + 1, 0, 19, {0b10000000, 0b01'000000, 0b000'00000}},
          {"2 groups", 162144u, 0, 19, {0b11001111, 0b00'101100, 0b000'00000}},
          {"2 groups max", MAX_BITS(18), 0, 19, {0b11111111, 0b11'111111, 0b111'00000}},

          {"1 group min non-aligned", 0u, 2, 10, {0b00'000000, 0b0000'0000}},
          {"1 group non-aligned", 489u, 2, 10, {0b00'011110, 0b1001'0000}},
          {"1 group max non-aligned", MAX_BITS(9), 2, 10, {0b00'011111, 0b1111'0000}},

          {"2 groups min non-aligned", MAX_BITS(9) + 1, 2, 19, {0b00'100000, 0b0001'0000, 0b00000000}},
          {"2 groups non-aligned", 162144u, 2, 19, {0b00'110011, 0b1100'1011, 0b00000000}},
          {"2 groups max non-aligned", MAX_BITS(18), 2, 19, {0b00'111111, 0b1111'1111, 0b11111000}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VbrInteger<false, 9, 9>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /*ExpectedMaxGroupCount*/ 2,
      /*ExpectedTotalUsableBits*/ 18,
      /*ExpectedMinValue*/ 0,
      /*ExpectedMaxValue*/ MAX_BITS(18));

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VbrInteger: 15-bit fields * 2 (30 usable bits, 31 written max)",
          "[VbrInteger][15-bit][unsigned][multi-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"single group", 27238u, 0, 16, {0b01101010, 0b01100110}},
           {"full group", MAX_BITS(30), 0, 31, {0b11111111, 0b11111111, 0b11111111, 0b11111110}},
           {"single byte non-aligned", 27238u, 3, 16, {0b00001101, 0b01001100, 0b11000000}},
           {"full group non-aligned", MAX_BITS(30), 3, 31, {0b00011111, 0b11111111, 0b11111111, 0b11111111, 0b11000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VbrInteger</* signed */ false, 15, 15>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_STATIC(T_SUT,
      /*ExpectedMaxGroupCount*/ 2,
      /*ExpectedTotalUsableBits*/ 30,
      /*ExpectedMinValue*/ 0,
      /*ExpectedMaxValue*/ MAX_BITS(30));

    ASSERT_VARIABLE_LENGTH_BIT_FIELD_METHODS(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#if SUB8_ENABLE_BOOL

TEST_CASE("Boolean: 1-bit value", "[Boolean][1-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, bool, uint8_t, size_t, std::vector<uint8_t>>(
          {{"true", true, 0, 1, {0b10000000}},
           {"false", false, 0, 1, {0b00000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Boolean;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    // Bool is fixed-length 1-bit
    ASSERT_BIT_SIZES_FIXED(T_SUT, input, /*expected_bits*/ 1);
    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_BOOL


#if SUB8_ENABLE_UINT4
TEST_CASE("U4: 4-bit value", "[U4][unsigned][4-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 4, {0b00000000}},
           {"value", 3, 0, 4, {0b00110000}},
           {"max", MAX_BITS(4), 0, 4, {0b11110000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = U4;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_BIT_SIZES_FIXED(T_SUT, input, 4);
    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif  // SUB8_ENABLE_UINT4


#if SUB8_ENABLE_UINT8
TEST_CASE("U8: 8-bit value", "[U8][unsigned][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 8, {0b00000000}},
           {"value", 3, 0, 8, {0b00000011}},
           {"max", MAX_BITS(8), 0, 8, {0b11111111}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = U8;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_BIT_SIZES_FIXED(T_SUT, input, 8);
    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif  // SUB8_ENABLE_UINT8


#if SUB8_ENABLE_INT8
TEST_CASE("I8: 8-bit value", "[I8][signed][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", -128, 0, 8, {0b11111111}},
           {"value", 0, 0, 8, {0b00000000}},
           {"max", 127, 0, 8, {0b11111110}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = I8;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_BIT_SIZES_FIXED(T_SUT, input, 8);
    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif  // SUB8_ENABLE_INT8


#if SUB8_ENABLE_UINT16
TEST_CASE("U16: 16-bit value", "[U16][unsigned][16-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint16_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 16, {0b00000000, 0b00000000}},
           {"value", 10000, 0, 16, {0b00100111, 0b00010000}},
           {"max", 65535, 0, 16, {0b11111111, 0b11111111}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = U16;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_BIT_SIZES_FIXED(T_SUT, input, 16);
    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif  // SUB8_ENABLE_UINT16


#if SUB8_ENABLE_INT16
TEST_CASE("I16: 16-bit value", "[I16][signed][16-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int16_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", -32768, 0, 16, {0b11111111, 0b11111111}},
           {"value", 0, 0, 16, {0b00000000, 0b00000000}},
           {"max", 32767, 0, 16, {0b11111111, 0b11111110}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = I16;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_BIT_SIZES_FIXED(T_SUT, input, 16);
    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_len);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif  // SUB8_ENABLE_INT16
