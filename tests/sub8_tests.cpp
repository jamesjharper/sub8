

#include <type_traits>
#include <cstring>
#include <cstdint>

#include <catch2/catch_all.hpp>
#include "./../src/sub8.h"
#include "test_helpers.h"

using namespace sub8;

#define REQUIRE_THAT_WRITE_IS_OK(input, padded_bits, expected_size, expected_vec) \
  BoundedBitWriter<128> bw; \
  { \
    auto expected_result = BitFieldResult::Ok; \
    bw.put_padding(padded_bits); \
    auto r = write_field(bw, input); \
    /* Compare byte buffer */ \
    REQUIRE(to_binary_string(bw.storage()) == to_binary_string(expected_vec)); \
    REQUIRE(bw.size().bit_size() == expected_size + padded_bits); \
\
    /* Compare result using Catch::Matchers */ \
    REQUIRE_THAT(std::string(error::to_string(r)), \
                 Catch::Matchers::Equals(std::string(error::to_string(expected_result)))); \
  }
 

#define REQUIRE_THAT_READ_CORE_IS_OK(output, padded_bits, input_bytes, input_bit_size, CHECK_OUTPUT) \
  do { \
    INFO("Input Bytes        = " << to_binary_string(input_bytes)); \
    INFO("Input BitSize      = " << (input_bit_size)); \
    \
    BoundedBitReader<128> br((input_bytes), BitSize::from_bits((input_bit_size) + (padded_bits))); \
    br.set_cursor_position(BitSize::from_bits((padded_bits))); \
    \
    const auto r = read_field(br, (output)); \
    INFO("Read Cursor Pos    = " << br.cursor_position().bit_size()); \
    const auto expected_result = BitFieldResult::Ok; \
    \
    /* Custom check for output vs expected */ \
    CHECK_OUTPUT; \
    \
    const auto expected_cursor_position = BitSize::from_bits((input_bit_size) + (padded_bits)); \
    REQUIRE(br.cursor_position().bit_size() == expected_cursor_position.bit_size()); \
    \
    REQUIRE_THAT(std::string(error::to_string(r)), \
                 Catch::Matchers::Equals(std::string(error::to_string(expected_result)))); \
  } while (false)

#define REQUIRE_THAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size) \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), \
    REQUIRE((output).value() == (expected_value)) \
  )

#define REQUIRE_THAT_OPT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size) \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), \
    REQUIRE((output) == (expected_value)) \
  )

#define REQUIRE_THAT_FLOAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size) \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), \
    do { \
      REQUIRE(bits_from_float((output).value()) == bits_from_float((expected_value))); \
      if (std::isnan((expected_value))) { \
        REQUIRE(std::isnan((output).value())); \
      } \
    } while (false) \
  )


// Assert const exprs are calculated correctly
#define ASSERT_BITFIELD_MINMAX(T_SUT, ExpectedMinValue, ExpectedMaxValue) \
  do { \
    REQUIRE(T_SUT::MaxValue == static_cast<typename T_SUT::Type>(ExpectedMaxValue)); \
    REQUIRE(T_SUT::MinValue == static_cast<typename T_SUT::Type>(ExpectedMinValue)); \
  } while (false)

#define ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, ExpectedTotalUsableBits, ExpectedMinValue, ExpectedMaxValue) \
  do { \
    REQUIRE(T_SUT::TotalUsableBits == (ExpectedTotalUsableBits)); \
    ASSERT_BITFIELD_MINMAX(T_SUT, (ExpectedMinValue), (ExpectedMaxValue)); \
  } while (false)

#define ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT, ExpectedGroupBitSize, ExpectedMaxGroupCount, ExpectedTotalUsableBits, \
                                         ExpectedMinValue, ExpectedMaxValue) \
  do { \
    REQUIRE(T_SUT::GroupBitSize == (ExpectedGroupBitSize)); \
    REQUIRE(T_SUT::MaxGroupCount == (ExpectedMaxGroupCount)); \
    REQUIRE(T_SUT::TotalUsableBits == (ExpectedTotalUsableBits)); \
    ASSERT_BITFIELD_MINMAX(T_SUT, (ExpectedMinValue), (ExpectedMaxValue)); \
  } while (false)

// clang-format off

// FixedLengthBitField tests

TEST_CASE("FixedLengthBitField: 8-bit Unsigned field", "[FixedLengthBitField][8-bit][unsigned][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"single byte", 5u, 0, 8, {0b00000101}},
           {"single byte non-aligned", 255u, 3, 8, {0b00011111, 0b11100000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = FixedLengthBitField<uint8_t, 8>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, 
      /* Expected Bit Width*/ 8,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 255
    );
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("FixedLengthBitField: 8-bit signed field", "[FixedLengthBitField][8-bit][signed][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"positive number", 1, 0, 8, {0b00000010}},
           {"negative number", -1, 0, 8, {0b0000001}},
           {"max positive number non byte aligned", 127, 3, 8, {0b00011111, 0b11000000}},
           {"min negative number non byte aligned", -128, 3, 8, {0b00011111, 0b11100000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = FixedLengthBitField<int8_t, 8>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, 
      /* Expected Bit Width*/ 8,
      /* Expected Min Value*/ -128,
      /* Expected Max Value*/ 127
    );
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("FixedLengthBitField: 3-bit Unsigned field", "[FixedLengthBitField][3-bit][unsigned][sub-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"sub byte size", 5u, 0, 3, {0b10100000}}, {"sub byte size non-aligned", 7u, 3, 3, {0b00011100}}}));

  DYNAMIC_SECTION(test_name) {

    using T_SUT = FixedLengthBitField<uint8_t, 3>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, 
      /* Expected Bit Width*/ 3,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 7
    );
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("FixedLengthBitField: 10-bit Unsigned field", "[FixedLengthBitField][10-bit][unsigned][over-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"cross byte boundary", 1010u, 0, 10, {0b11111100, 0b10000000}},
           {"cross byte boundary non-aligned", 1023u, 3, 10, {0b00011111, 0b11111000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = FixedLengthBitField<uint32_t, 10>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, 
      /* Expected Bit Width*/ 10,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 1023
    );

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("FixedLengthBitField: 26-bit Unsigned field", "[FixedLengthBitField][26-bit][unsigned][multi-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"multi sub byte size", 67000000u, 0, 26, {0b11111111, 0b10010101, 0b10110000, 0b00000000}},
           {"multi sub byte size non-aligned", 67000000u, 3, 26, {0b00011111, 0b11110010, 0b10110110, 0b00000000}}}));

  DYNAMIC_SECTION(test_name) {\
    using T_SUT = FixedLengthBitField<uint32_t, 26>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, 
      /* Expected Bit Width*/ 26,
      /* Expected Min Value*/ 0,
      /* Expected Max Value*/ 67108863
    );


    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

// VariableLengthBitField

// VariableLengthBitField with groups smaller then 1 byte
TEST_CASE("VariableLengthBitField: 3-bit groups * 4 (9/15 bits useable)",
          "[VariableLengthBitField][3-bit][unsigned][sub-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint16_t, uint8_t, size_t, std::vector<uint8_t>>({

          // --- 1 GROUP (3 usable bits) ---
          {"1 group min", 0u, 0, 4, {0b0000'0000}},
          {"1 group", 5u, 0, 4, {0b0101'0000}},
          {"1 group max", MAX_BITS(3), 0, 4, {0b0111'0000}},

          // --- 2 GROUPS (6 usable bits) ---
          {"2 groups min", MAX_BITS(3) + 1, 0, 8, {0b1001'0000}},
          {"2 groups", 40u, 0, 8, {0b1101'0000}},
          {"2 groups max", MAX_BITS(6), 0, 8, {0b1111'0111}},

          // --- 3 GROUPS (9 usable bits) ---
          {"3 groups min ", MAX_BITS(6) + 1, 0, 12, {0b1001'1000, 0b0000'0000}},
          {"3 groups", 360u, 0, 12, {0b1101'1101, 0b0000'0000}},
          {"3 groups max", MAX_BITS(9), 0, 12, {0b1111'1111, 0b0111'0000}},

          // --- 4 GROUPS (13 usable bits) ---
          {"4 groups min", MAX_BITS(9) + 1, 0, 15, {0b1001'1000, 0b1000'0000}},
          {"4 groups", 621u, 0, 15, {0b1001'1001, 0b1101'1010}},
          {"4 groups max", MAX_BITS(12), 0, 15, {0b1111'1111, 0b1111'1110}},

          // --- NON-BYTE-ALIGNED CASES (padding=3) ---

          // --- 1 GROUPS (3 usable bits) ---
          {"1 group min non-aligned", 0u, 1, 4, {0b0'000'0000}},
          {"1 group non-aligned", 5u, 1, 4, {0b0'010'1000}},
          {"1 group max non-aligned", MAX_BITS(3), 1, 4, {0b0'011'1000}},

          // --- 2 GROUPS (6 usable bits) ---
          {"2 groups min non-aligned", MAX_BITS(3) + 1, 1, 8, {0b0'100'100'0, 0b00'000000}},
          {"2 groups non-aligned", 40u, 1, 8, {0b0'110'100'0, 0b00'000000}},
          {"2 groups max non-aligned", MAX_BITS(6), 1, 8, {0b0'111'101'1, 0b10'000000}},

          // --- 3 GROUPS (9 usable bits) ---
          {"3 groups min non-aligned", MAX_BITS(6) + 1, 1, 12, {0b0'100'110'0, 0b00'000'000}},
          {"3 groups non-aligned", 360u, 1, 12, {0b0'110'111'0, 0b10'000'000}},
          {"3 groups max non-aligned", MAX_BITS(9), 1, 12, {0b0'111'111'1, 0b10'111'000}},

          // --- 4 GROUPS (13 usable bits) ---
          {"4 groups min non-aligned", MAX_BITS(9) + 1, 1, 15, {0b0'1001'100, 0b0'1000'000}},
          {"4 groups non-aligned", 621u, 1, 15, {0b0'1001'100, 0b1'1101'101}},
          {"4 groups max non-aligned", MAX_BITS(12), 1, 15, {0b0'1111'111, 0b1'1111'111}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<uint16_t, 3, 4>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 4,
                                     /*ExpectedMaxGroupCount*/ 4,
                                     /*ExpectedTotalUsableBits*/ 12,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(12));
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VariableLengthBitField: 7-bit groups * 1 (7/7 bits useable)",
          "[VariableLengthBitField][7-bit][unsigned][sub-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"group 1 min", 0u, 0, 8, {0b00000000}},
           {"group 1 ", 5u, 0, 8, {0b00000101}},
           {"group 1 max", MAX_BITS(8), 0, 8, {0b11111111}},
           {"group 1 min - non-aligned", 0u, 3, 8, {0b00000000, 0b00000000}},
           {"group 1 - non-aligned", 5u, 3, 8, {0b00000000, 0b10100000}},
           {"group 1 max - non-aligned", MAX_BITS(8), 3, 8, {0b00011111, 0b11100000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<uint8_t, 7, 1>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 7,
                                     /*ExpectedMaxGroupCount*/ 1,
                                     /*ExpectedTotalUsableBits*/ 8,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(8));

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VariableLengthBitField: 7-bit groups * 2 (14/15 bits useable)",
          "[VariableLengthBitField][7-bit][unsigned][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint16_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"single group", 5u, 0, 8, {0b00000101}},
           {"full group", MAX_BITS(14), 0, 15, {0b11111111, 0b11111110}},
           {"single group non-aligned", 5u, 3, 8, {0b00000000, 0b10100000}},
           {"full group non-aligned", MAX_BITS(14), 3, 15, {0b00011111, 0b11111111, 0b11000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<uint16_t, 7, 2>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 8,
                                     /*ExpectedMaxGroupCount*/ 2,
                                     /*ExpectedTotalUsableBits*/ 14,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(14));
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VariableLengthBitField: 7-bit groups * 2 (14/15 bits useable)",
          "[VariableLengthBitField][7-bit][signed][byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int16_t, uint8_t, size_t, std::vector<uint8_t>>({

          {"single group", 5, 0, 8, {0b00001010}},
          {"full group max", 8191, 0, 15, {0b11111111, 0b11111100}},
          {"single group non-aligned", -5, 3, 8, {0b00000001, 0b00100000}},
          {"full group min non-aligned", -8192, 3, 15, {0b00011111, 0b11111111, 0b11000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<int16_t, 7, 2>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 8,
                                     /*ExpectedMaxGroupCount*/ 2,
                                     /*ExpectedTotalUsableBits*/ 14,
                                     /*ExpectedMinValue*/ -8192,
                                     /*ExpectedMaxValue*/ 8191);
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VariableLengthBitField: 7-bit groups * 4 (28/31 bits useable)",
          "[VariableLengthBitField][7-bit][unsigned][byte]") {
  auto [test_name, value, padding, expected_len,
        expected_bytes] = GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>({
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

      // 2 groups: uses new algorithm output
      {"2 groups min non-aligned", MAX_BITS(7) + 1, 3, 16, {0b00010000, 0b00100000, 0b00000000}},
      {"2 groups non-aligned", 1000u, 3, 16, {0b00010000, 0b11101101, 0b00000000}},
      {"2 groups max non-aligned", MAX_BITS(14), 3, 16, {0b00011111, 0b11101111, 0b11100000}},

      {"3 groups min non-aligned", MAX_BITS(14) + 1, 3, 24, {0b00010000, 0b00110000, 0b00000000, 0b00000000}},
      {"3 groups non-aligned", 100000u, 3, 24, {0b00010000, 0b11010001, 0b10100100, 0b00000000}},
      {"3 groups max non-aligned", MAX_BITS(21), 3, 24, {0b00011111, 0b11111111, 0b11101111, 0b11100000}},

      // 4 groups: [1][7] [1][7] [1][7] [8 pure at MaxIdx] = 29 bits total, max = (1<<29)-1
      {"4 groups min non-aligned",
       MAX_BITS(21) + 1,
       3,
       31,
       {0b00010000, 0b00110000, 0b00010000, 0b00000000, 0b00000000}},
      {"4 groups non-aligned", 6000000u, 3, 31, {0b00010000, 0b01011101, 0b11010011, 0b01100000, 0b00000000}},
      {"4 groups max non-aligned", MAX_BITS(28), 3, 31, {0b00011111, 0b11111111, 0b11111111, 0b11111111, 0b11000000}},
  }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<uint32_t, 7, 4>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 8,
                                     /*ExpectedMaxGroupCount*/ 4,
                                     /*ExpectedTotalUsableBits*/ 28,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(28));
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VariableLengthBitField: 9-bit groups * 2 (18/19 bits useable)",
          "[VariableLengthBitField][19-bit][unsigned][multi-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>({

          // --- 1 GROUP (9 usable bits, 10 written bits) ---
          {"1 group min", 0u, 0, 10, {0b00000000, 0b00'000000}},
          {"1 group", 489u, 0, 10, {0b01111010, 0b01'000000}},
          {"1 group max", MAX_BITS(9), 0, 10, {0b01111111, 0b11'000000}},

          // --- 2 GROUPS (19 usable bits, 19 written bits) ---
          {"2 groups min", MAX_BITS(9) + 1, 0, 19, {0b10000000, 0b01'000000, 0b000'00000}},
          {"2 groups", 162144u, 0, 19, {0b11001111, 0b00'101100, 0b000'00000}},
          {"2 groups max", MAX_BITS(18), 0, 19, {0b11111111, 0b11'111111, 0b111'00000}},

          // --- NON-BYTE-ALIGNED CASES (padding=3) ---

          // --- 1 GROUP (9 usable bits, 10 written bits) ---
          {"1 group min non-aligned", 0u, 2, 10, {0b00'000000, 0b0000'0000}},
          {"1 group non-aligned", 489u, 2, 10, {0b00'011110, 0b1001'0000}},
          {"1 group max non-aligned", MAX_BITS(9), 2, 10, {0b00'011111, 0b1111'0000}},

          // --- 2 GROUPS (19 usable bits, 19 written bits) ---
          {"2 groups min non-aligned", MAX_BITS(9) + 1, 2, 19, {0b00'100000, 0b0001'0000, 0b00000000}},
          {"2 groups non-aligned", 162144u, 2, 19, {0b00'110011, 0b1100'1011, 0b00000000}},
          {"2 groups max non-aligned", MAX_BITS(18), 2, 19, {0b00'111111, 0b1111'1111, 0b11111000}},

      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<uint32_t, 9, 2>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 10,
                                     /*ExpectedMaxGroupCount*/ 2,
                                     /*ExpectedTotalUsableBits*/ 18,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(18));

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("VariableLengthBitField: 15-bit fields * 2 (30/31 bits usable)",
          "[VariableLengthBitField][15-bit][unsigned][multi-byte]") {
  auto [test_name, value, padding, expected_len, expected_bytes] = GENERATE(table<std::string, uint32_t, uint8_t,
                                                                                  size_t, std::vector<uint8_t>>(
      {{"single group", 27238u, 0, 16, {0b01101010, 0b01100110}},
       {"full group", MAX_BITS(30), 0, 31, {0b11111111, 0b11111111, 0b11111111, 0b11111110}},
       {"single byte non-aligned", 27238u, 3, 16, {0b00001101, 0b01001100, 0b11000000}},
       {"full group non-aligned", MAX_BITS(30), 3, 31, {0b00011111, 0b11111111, 0b11111111, 0b11111111, 0b11000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = VariableLengthBitField<uint32_t, 15, 2>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 16,
                                     /*ExpectedMaxGroupCount*/ 2,
                                     /*ExpectedTotalUsableBits*/ 30,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(30));
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

// PathBitField

#if SUB8_ENABLE_ARRAY_FIELDS

// PathBitField

TEST_CASE("PathBitField: 3-bit paths * 1 (3/4 bits usable)", "[PathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"no groups", {}, 0, 1, {0b0}},
          {"1 group", {1}, 0, 4, {0b1'001'0000}},

          {"no groups non-aligned", {}, 2, 1, {0b00'0'00000}},
          {"1 group non-aligned", {1}, 2, 4, {0b00'1'001'00}},
      }));

  DYNAMIC_SECTION(test_name) {
    
    using T_SUT = NumericArrayBitField<uint8_t, 3, 0, 1, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 3-bit paths * 1 (3/4 bits usable)", "[PathBitField][signed][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<int8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"no groups", {}, 0, 1, {0b0}},
          {"1 group", {1}, 0, 4, {0b1'010'0000}},

          {"no groups non-aligned", {}, 2, 1, {0b00'0'00000}},
          {"1 group non-aligned", {-1}, 2, 4, {0b00'1'001'00}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<int8_t,3, 0, 1, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 3-bit paths * 2 (9/12 bits usable)", "[PathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>(
          {{"no groups", {},     0, 1, {0b0}},
           {"1 group",   {1},    0, 5, {0b1'001'0'000}},
           {"2 group",   {1, 2}, 0, 8, {0b1'001'1'010}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint8_t, 3, 0, 2, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 3-bit paths * 3 (9/12 bits usable)", "[PathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>(
          {{"no groups", {}, 0, 1,      {0b0}},
           {"1 group", {1}, 0, 5,        {0b1'001'0'000}},
           {"2 group", {1, 2}, 0, 9,     {0b1'001'1'010, 0b0'0000000}},
           {"3 group", {1, 2, 3}, 0, 12, {0b1'001'1'010, 0b1'011'0000}},

           {"no groups non-aligned", {}, 2, 1, {0b00'000000}},
           {"1 group non-aligned", {1}, 2, 5, {0b00100100}},
           {"2 group non-aligned", {1, 2}, 2, 9, { 0b00100110, 0b10000000}},
           {"3 group non-aligned", {1, 2, 3}, 2, 12, {0b00100110, 0b10101100}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint8_t, 3, 0, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: Non empty, 3-bit paths * 3 (9/11 bits usable)", "[NonEmptyPathBitField][unsigned][3-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>(
          {{"1 group", {1}, 0, 4, {0b001'00000}},
           {"2 group", {1, 2}, 0, 8, {0b001'1'010'0}},
           {"3 group", {1, 2, 3}, 0, 11, {0b001'1'010'1, 0b011'00000}},

           {"1 group non-aligned", {1}, 2, 4, {0b00001000}},
           {"2 group non-aligned", {1, 2}, 2, 8, {0b00001101, 0b00000000}},
           {"3 group non-aligned", {1, 2, 3}, 2, 11, {0b00001101, 0b01011000 }}
          }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint8_t,3, 1, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 7-bit paths * 3 (21/24 bits usable)", "[PathBitField][unsigned][7-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"1 group max", {127}, 0, 9,              { 0b1'1111111, 0b0'0000000 }},
          {"2 group max", {127, 127}, 0, 17,        { 0b1'1111111, 0b1'1111111, 0b0'0000000 }},
          {"3 group max", {127, 127, 127}, 0, 24,   { 0b1'1111111, 0b1'1111111, 0b1'1111111 }},
          {"no groups",   { }, 0, 1,                { 0b0'0000000 }},
          {"1 group",     {1}, 0, 9,                { 0b1'0000001, 0b00000000 }},
          {"2 group",     {1, 2}, 0, 17,            { 0b1'0000001, 0b1'0000010, 0b0'0000000 }},
          {"3 group",     {1, 2, 3}, 0, 24,         { 0b1'0000001, 0b1'0000010, 0b1'0000011 }},
          {"1 group non-aligned", {1}, 2, 9,        { 0b00100000, 0b01000000 }},
          {"2 group non-aligned", {2, 1}, 2, 17,    { 0b00100000, 0b10100000, 0b01000000 }},
          {"3 group non-aligned", {3, 2, 1}, 2, 24, { 0b00100000, 0b11100000, 0b10100000, 0b01000000 }},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint8_t, 7, 0, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField:  Non empty 7-bit paths * 3 (21/23 bits usable)", "[PathBitField][unsigned][7-bit]") {
  auto [test_name, in_value, out_value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"1 group max", {127},              {127, 0},        0, 15, { 0b1111111'0, 0b000000'0'0}},
          {"2 group max", {127, 127},         {127, 127},      0, 15, { 0b1111111'1, 0b111111'0'0}},
          {"3 group max", {127, 127, 127},    {127, 127, 127}, 0, 22, { 0b1111111'1, 0b111111'1'1, 0b111111'00}},
          {"1 group",     {1},                {1, 0},          0, 15, { 0b0000001'0, 0b00000'0'00}},
          {"2 group",     {1, 2},             {1, 2},          0, 15, { 0b0000001'0, 0b000010'0'0}},
          {"3 group",     {1, 2, 3},          {1, 2, 3},       0, 22, { 0b0000001'0, 0b000010'1'0, 0b000011'00}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint8_t, 7, 2, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(in_value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, out_value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 8-bit paths * 3 (21/24 bits usable)", "[PathBitField][unsigned][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"1 group max", {255}, 0, 10,           {0b1'1111111, 0b1'0'000000 }},
          {"2 group max", {255, 255}, 0, 19,      {0b1'1111111, 0b1'1'111111, 0b11'0'00000 }},
          {"3 group max", {255, 255, 255}, 0, 27, {0b1'1111111, 0b1'1'111111, 0b11'1'11111, 0b111'00000}},
          {"1 group min", {1}, 0, 10,             {0b1'0000000, 0b1'0'000000}},
          {"2 group min", {1, 2}, 0, 19,          {0b1'0000000, 0b1'1'000000, 0b10'0'00000}},
          {"3 group min", {1, 2, 3}, 0, 27,       {0b1'0000000, 0b1'1'000000, 0b10'1'00000, 0b011'00000}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint8_t, 8, 0, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("PathBitField: 10-bit paths * 3 (30/33 bits usable)", "[PathBitField][unsigned][7-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint16_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"1 group max", {1023}, 0, 12,             { 0b1'1111111, 0b111'0'0000}},
          {"2 group max", {1023, 1023}, 0, 23,       { 0b1'1111111, 0b111'1'1111, 0b111111'0'0}},
          {"3 group max", {1023, 1023, 1023}, 0, 33, { 0b1'1111111, 0b111'1'1111, 0b111111'1'1, 0b11111111, 0b1'0000000}},
          {"no groups",   {}, 0, 1,                  { 0b0}},
          {"1 group",     {1}, 0, 12,                { 0b1'0000000, 0b001'0'0000}},
          {"2 group",     {1, 2}, 0, 23,             { 0b1'0000000, 0b001'1'0000, 0b000010'0'0}},
          {"3 group",     {1, 2, 3}, 0, 33,          { 0b1'0000000, 0b001'1'0000, 0b000010'1'0, 0b00000001, 0b1'0000000}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint16_t, 10, 0, 3, ArrayEncoding::Delimited>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

// Prefixed Arrays

TEST_CASE("PrefixedArrayBitField: 10-bit strings * 3 (30/32 bits usable)", "[PrefixedArrayBitField][unsigned][10-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint16_t>, uint8_t, size_t, std::vector<uint8_t>>({
          // length = 0
          {"no groups", {}, 0, 2, {0b00000000}},

          // length = 1
          {"1 group", {1}, 0, 12, {0b01000000, 0b00010000}},
          {"1 group max", {1023}, 0, 12, {0b01111111, 0b11110000}},

          // length = 2
          {"2 group", {2, 1}, 0, 22, {0b10000000, 0b00100000, 0b00000100}},
          {"2 group max", {1023, 1023}, 0, 22, {0b10111111, 0b11111111, 0b11111100}},

          // length = 3
          {"3 group", {3, 2, 1}, 0, 32, {0b11000000, 0b00110000, 0b00001000, 0b00000001}},
          {"3 group max", {1023, 1023, 1023}, 0, 32, {0b11111111, 0b11111111, 0b11111111, 0b11111111}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericArrayBitField<uint16_t, 10, 0, 3, ArrayEncoding::Prefixed>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("NonEmptyPrefixedArrayBitField: 8bit fixed size array min 2, max 3", "[NonEmptyPrefixedArrayBitField][unsigned][8-bit]") {
  auto [test_name, in_value, out_value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"empty",          {},        {0,0},   0, 17,   {0b0'0000000,0b0'0000000,0b0'0000000}},
          {"partially used", {1,2},     {1,2},   0, 17,   {0b0'0000000,0b1'0000001,0b0'0000000}},
          {"full",           {1,2,3},   {1,2,3}, 0, 25,   {0b1'0000000,0b1'0000001,0b0'0000001,0b1'0000000}},
      }));

  DYNAMIC_SECTION(test_name) {
    
    using T_SUT = NumericArrayBitField<uint8_t, 8, 2, 3, ArrayEncoding::Prefixed>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(in_value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, out_value, bw.storage(), expected_len);
  }
}

TEST_CASE("FixedSizeArrayBitField: 8bit fixed size array * 3 (30/32 bits usable)", "[FixedSizeArrayBitField][unsigned][8-bit]") {
  auto [test_name, in_value, out_value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, std::vector<uint8_t>, std::vector<uint8_t>, uint8_t, size_t, std::vector<uint8_t>>({
          {"empty",          {},        {0,0,0}, 0, 24,   {0b00000000,0b00000000,0b00000000}},
          {"partially used", {1,2},     {1,2,0}, 0, 24,   {0b00000001,0b00000010,0b00000000}},
          {"full",           {1,2,3},   {1,2,3}, 0, 24,   {0b00000001,0b00000010,0b00000011}},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = NumericFixedArrayBitField<uint16_t, 8, 3>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(in_value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, out_value, bw.storage(), expected_len);
  }
}

TEST_CASE("ArrayBitField operator== should not compares stale tail bytes") {
  // Arrange
  using F = NumericArrayBitField<uint8_t, 3, 0, 3>;

  F a{};
  F b{};

  // Put different junk into the tail of each buffer
  REQUIRE(a.set_value(std::vector<uint8_t>{1, 2, 3}) == BitFieldResult::Ok);
  REQUIRE(b.set_value(std::vector<uint8_t>{7, 7, 7}) == BitFieldResult::Ok);

  // Act
  // Now set both to the same *logical* value with size==1
  REQUIRE(a.set_value(std::vector<uint8_t>{1}) == BitFieldResult::Ok);
  REQUIRE(b.set_value(std::vector<uint8_t>{1}) == BitFieldResult::Ok);

  // Assert
  // Logically they should be equal: same size and same element[0]
  REQUIRE(a.size() == 1);
  REQUIRE(b.size() == 1);
  REQUIRE(a[0] == 1);
  REQUIRE(b[0] == 1);

  REQUIRE(a == b);
}

TEST_CASE("ArrayBitField should use delimited strategy for lengths equal to less then 3") {
  // Arrange
  auto value = std::vector<uint8_t>{1, 2, 3};
  auto expected_bytes = std::vector<uint8_t>{0b1'001'1'010, 0b1'011'0000};
  size_t expected_len = 12;
  // Act
  using T_SUT = NumericArrayBitField<uint8_t, 3, 0, 3>; // Delimited, Min=0, Max=3

  T_SUT input{};
  T_SUT output{};

  REQUIRE(input.set_value(value) == BitFieldResult::Ok);

  REQUIRE_THAT_WRITE_IS_OK(input, 0, expected_len, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_len);
}

TEST_CASE("ArrayBitField should use prefix strategy for lengths greater then 3") {
  // Arrange
  auto value = std::vector<uint8_t>{1, 2, 3, 4};
  auto expected_bytes = std::vector<uint8_t>{0b100'001'01,0b0'011'100'0};
  size_t expected_len = 15;
  // Act
  using T_SUT = NumericArrayBitField<uint8_t, 3, 0, 4>;
  
  T_SUT input{};
  T_SUT output{};

  REQUIRE(input.set_value(value) == BitFieldResult::Ok);

  REQUIRE_THAT_WRITE_IS_OK(input, 0, expected_len, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_len);
}


#endif // SUB8_ENABLE_ARRAY_FIELDS

#if SUB8_ENABLE_ENUM_FIELDS

enum class MsgType : uint8_t {
  Ping = 10,
  Pong = 11,
  Data = 12,
  Ack = 13,
  Nack = 14,
};

TEST_CASE("EnumBitField: 3-bit", "[EnumBitField][unsigned][4-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, MsgType, uint8_t, size_t, std::vector<uint8_t>>(
          {
            {"Ping value", MsgType::Ping, 0, 3, {0b000'00000}},
            {"Pong value", MsgType::Pong, 0, 3, {0b001'00000}},
            {"Data value", MsgType::Data, 0, 3, {0b010'00000}},
            {"Data value", MsgType::Ack,  0, 3, {0b011'00000}},
            {"Nack value", MsgType::Nack, 0, 3, {0b1000'0000}}
          }
      ));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = EnumBitField<MsgType, MsgType::Ping, MsgType::Nack>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif // SUB8_ENABLE_ENUM_FIELDS

#if SUB8_ENABLE_OPTIONAL_FIELDS

TEST_CASE("OptionalBitField: 8-bit", "[OptionalBitField][unsigned][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, sub8::optional<Uint8ValueField>, uint8_t, size_t, std::vector<uint8_t>>(
          {
            {"optional with value",    sub8::optional<Uint8ValueField>(make_or_throw<Uint8ValueField>(3)), 0, 9, {0b10000001, 0b10000000}},
            {"optional without value", sub8::optional<Uint8ValueField>(), 0, 1,  {0b00000000}},
          }
      ));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = OptionalBitField<Uint8ValueField>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_OPT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif // SUB8_ENABLE_OPTIONAL_FIELDS

#if SUB8_ENABLE_BOOL

TEST_CASE("BoolValueField: 1-bit value", "[BoolValueField][1-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, bool, uint8_t, size_t, std::vector<uint8_t>>(
          {{"true", true, 0, 1, {0b10000000}}, {"false", false, 0, 1, {0b0}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BoolValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_BOOL

#if SUB8_ENABLE_UINT4
TEST_CASE("Uint4ValueField: 4-bit value", "[Uint4ValueField][unsigned][4-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] = GENERATE(
      table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>({{"min", 0, 0, 4, {0b00000000}},
                                                                          {"value", 3, 0, 4, {0b00110000}},
                                                                          {"max", MAX_BITS(4), 0, 4, {0b11110000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint4ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT4

#if SUB8_ENABLE_UINT8
TEST_CASE("Uint8ValueField: 8-bit value", "[Uint8ValueField][unsigned][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] = GENERATE(
      table<std::string, uint8_t, uint8_t, size_t, std::vector<uint8_t>>({{"min", 0, 0, 8, {0b00000000}},
                                                                          {"value", 3, 0, 8, {0b00000011}},
                                                                          {"max", MAX_BITS(8), 0, 8, {0b11111111}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint8ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT8

#if SUB8_ENABLE_INT8
TEST_CASE("Int8ValueField: 8-bit value", "[Int8ValueField][signed][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int8_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", -128, 0, 8, {0b11111111}}, {"value", 0, 0, 8, {0b00000000}}, {"max", 127, 0, 8, {0b11111110}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int8ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_INT8

#if SUB8_ENABLE_UINT16
TEST_CASE("Uint16ValueField: 16-bit value", "[Uint16ValueField][unsigned][16-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint16_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 16, {0b00000000, 0b00000000}},
           {"value", 10000, 0, 16, {0b00100111, 0b00010000}},
           {"max", 65535, 0, 16, {0b11111111, 0b11111111}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint16ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT16

#if SUB8_ENABLE_INT16
TEST_CASE("Int16ValueField: 16-bit value", "[Int16ValueField][signed][16-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int16_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", -32768, 0, 16, {0b11111111, 0b11111111}},
           {"value", 0, 0, 16, {0b00000000, 0b00000000}},
           {"max", 32767, 0, 16, {0b11111111, 0b11111110}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int16ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT8

#if SUB8_ENABLE_UINT32
TEST_CASE("Uint32ValueField: 32-bit value", "[Uint32ValueField][unsigned][32-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 32, {0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"value", 65536, 0, 32, {0b00000000, 0b00000001, 0b00000000, 0b00000000}},
           {"max", 4294967295, 0, 32, {0b11111111, 0b11111111, 0b11111111, 0b11111111}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint32ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Uint32Pack8ValueField: 32-bit value [8bit packing]", "[Uint32Pack8ValueField][32-bit][8-bit Packing]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 9, {0b00000000, 0b00000000}},
           {"value", 10000, 0, 18, {0b10010011, 0b10000100, 0b00000000}},
           {"max", MAX_BITS(32), 0, 35, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11100000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint32Pack8ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 9,
                                     /*ExpectedMaxGroupCount*/ 4,
                                     /*ExpectedTotalUsableBits*/ 32,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ 4294967295);
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Uint32Pack16ValueField: 32-bit value [16bit packing]", "[Uint32Pack16ValueField][32-bit][8-bit Packing]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 17, {0b00000000, 0b00000000, 0b00000000}},
           {"value", 10000, 0, 17, {0b00010011, 0b10001000, 0b00000000}},
           {"max", MAX_BITS(32), 0, 33, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b10000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint32Pack16ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 17,
                                     /*ExpectedMaxGroupCount*/ 2,
                                     /*ExpectedTotalUsableBits*/ 32,
                                     /*ExpectedMaxValue*/ 0,
                                     /*ExpectedMaxValue*/ 4294967295);
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT32

#if SUB8_ENABLE_INT32
TEST_CASE("Int32ValueField: 32-bit value", "[Int32ValueField][signed][32-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int32_t, int8_t, size_t, std::vector<uint8_t>>(
          {{"min", -2147483648, 0, 32, {0b11111111, 0b11111111, 0b11111111, 0b11111111}},
           {"value", 0, 0, 32, {0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"max", 2147483647, 0, 32, {0b11111111, 0b11111111, 0b11111111, 0b11111110}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int32ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Int32Pack8ValueField: 32-bit value [8bit packing]", "[Int32Pack8ValueField][32-bit][8-bit Packing]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", -2147483648, 0, 35, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11100000}},
           {"value", 0, 0, 9, {0b00000000, 0b0}},
           {"max", 2147483647, 0, 35, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int32Pack8ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Int32Pack16ValueField: 32-bit value [16bit packing]", "[Int32Pack16ValueField][32-bit][8-bit Packing]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int32_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", -2147483648, 0, 33, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b10000000}},
           {"value", 0, 0, 17, {0b00000000, 0b00000000, 0b0}},
           {"max", 2147483647, 0, 33, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int32Pack16ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT32

#if SUB8_ENABLE_UINT64
TEST_CASE("Uint64ValueField: 64-bit value", "[Uint64ValueField][unsigned][64-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint64_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min",
            0,
            0,
            64,
            {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"value",
            MAX_BITS(32) + 1,
            0,
            64,
            {0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"max",
            MAX_BITS(64),
            0,
            64,
            {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint64ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Uint64Pack32ValueField: 64-bit value [32bit packing]", "[Uint64Pack32ValueField][64-bit][32-bit Packing]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, uint64_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min", 0, 0, 33, {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"value", 10000, 0, 33, {0b00000000, 0b00000000, 0b00010011, 0b10001000, 0b00000000}},
           {"max",
            MAX_BITS(64),
            0,
            65,
            {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
             0b10000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Uint64Pack32ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT,
                                     /*ExpectedGroupBitSize*/ 33,
                                     /*ExpectedMaxGroupCount*/ 2,
                                     /*ExpectedTotalUsableBits*/ 64,
                                     /*ExpectedMinValue*/ 0,
                                     /*ExpectedMaxValue*/ MAX_BITS(64));

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_UINT64

#if SUB8_ENABLE_UINT64
TEST_CASE("Int64ValueField: 64-bit value", "[Int64ValueField][signed][64-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int64_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min",
            std::numeric_limits<int64_t>::min(),
            0,
            64,
            {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}},
           {"value",
            0,
            0,
            64,
            {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"max",
            std::numeric_limits<int64_t>::max(),
            0,
            64,
            {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111110}}}));
  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int64ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

TEST_CASE("Int64Pack32ValueField: 64-bit value [32bit packing]", "[Int64Pack32ValueField][64-bit][32-bit Packing]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, int64_t, uint8_t, size_t, std::vector<uint8_t>>(
          {{"min",
            std::numeric_limits<int64_t>::min(),
            0,
            65,
            {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
             0b10000000}},
           {"value", 0, 0, 33, {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
           {"max",
            std::numeric_limits<int64_t>::max(),
            0,
            65,
            {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
             0b00000000}}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Int64Pack32ValueField;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);
    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}
#endif  // SUB8_ENABLE_INT64

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

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_FLOAT_READ_IS_OK(output, padding, expected_read, bw.storage(), expected_len);

    if (std::isnan(value)) {
      REQUIRE(std::isnan(output.value()));
    }
  }
}

#endif  // SUB8_ENABLE_FLOAT32

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

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_FLOAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_FLOAT32

#if SUB8_ENABLE_FLOAT64
TEST_CASE("Float64ValueField: 64-bit value", "[Float64ValueField][64-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, double, uint8_t, size_t, std::vector<uint8_t>>({
          {"1.0f",
           1.0f,
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

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_FLOAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif  // SUB8_ENABLE_FLOAT64

#if SUB8_ENABLE_FIVE_BIT_STRING && SUB8_ENABLE_STRING_FIELDS__CHAR

TEST_CASE("FiveBitStringBitField<char>: 5-bit strings (header + controls + multibyte)",
          "[FiveBitStringBitField][5-bit][string]") {
  auto [test_name, value, padding, expected_bits,
        expected_bytes] = GENERATE(table<std::string,          // test_name
                                         std::string,          // value
                                         uint8_t,              // padding
                                         size_t,               // expected_bits (no padding)
                                         std::vector<uint8_t>  // expected_bytes
                                         >(
      {// Standard Text encoding
       // =======================

       {"Simple Lower Case Encoding",
        "abc",
        0,
        23,  // 8 header bits + 3 symbols * 5 bits
        {
            0b00000011,  // Length = 15
            0b00000'000, 0b01'00010'0,
            // a   | b      | c   |
        }},
       {"Simple numeric Encoding",
        // Using a T2 control char to switch to the numeric table
        "123.4",
        0,
        38,  // 8 header bits + 6 symbols * 5 bits
        {
            0b00000110,  // Length = 6
            0b11110'000, 0b01'00010'0, 0b0011'1000, 0b1'00100'00
            // T2  |   1    | 2   |   3    |   .    |  4
        }},
       {"Capital case Encoding",
        // Using a T1 control char to switch to the capital table, followed by another to switch back to default table
        // T0
        "Abc",
        0,
        33,  // 8 header bits + 5 symbols * 5 bits
        {
            0b00000101,  // Length = 5
            0b11101'000, 0b00'11101'0, 0b0001'0001, 0b0'00000'00
            // T1  |   A    |  T1 |    b   |   c    |
        }},
       {"Character Encoding with numbers",
        // Using a T2 control char to switch to the numeric table, followed by another to switch back to default table
        // T0
        "h3ll0",
        0,
        48,  // 8 header bits + 5 symbols * 5 bits
        {
            0b00001000,  // Length = 8
            0b00111'111, 0b10'00011'1, 0b1110'0101, 0b1'01011'11, 0b110'00000
            //  h  |   T2    |  3  |    T0   |   l     |  l  |    T2   | 0
        }},
       {"Encoding with character duplicated across tables",
        // Using a '-' char which exists in mutiple tables
        "h-LL-",
        0,
        38,  // 8 header bits + 6 symbols * 5 bits
        {
            0b00000110,  // Length = 6
            0b00111'111, 0b00'11101'0, 0b1011'0101, 0b1'11100'00,
            //  h  |   -     | T1  |    L    |    L    |  -  |
        }},

       // Extended Text encoding using character modifiers
       // ================================================
       {"Encoding with character modifiers",
        // Using a T3 control char to modify the previous char with the subsequent modifier code.
        // The T3 control char acts as a table shift operation, rather then a lock. Ie T3 control char
        // only effects the next char
        "hällo",
        0,
        43,  // 8 header bits + 7 symbols * 5 bits
        {
            0b00000111,  // Length = 6
            0b00111'000, 0b00'11111'0, 0b1111'0101, 0b1'01011'01, 0b110'00000
            //  h  |   a     | T3  |    ◌̈    |    l    |  l  |     o   |
        }},
       {"Encoding capital letters with character modifiers",
        // Same as above, but the previous char originates for the capital case table.
        // which will result an a different character
        "HÄLLO",
        0,
        48,  // 8 header bits + 7 symbols * 5 bits
        {
            0b00001000,  // Length = 8
            0b11101'001, 0b11'00000'1, 0b1111'0111, 0b1'01011'01, 0b011'01110
            // T2  |   H     |  A  |    T3   |    ◌̈    |  L  |     L   | 0
        }},
       {"simple Greek lower case encoding",
        // Using a T3 control char to switch T1 and T2 to a Greek character set
        "αβγ",
        0,
        38,  // 8 header bits + 6 symbols * 5 bits
        {
            0b00000110,  // Length = 6
            0b11100'111, 0b11'00001'0, 0b0000'0000, 0b1'00010'00,
            // 28  |   T3    |  1  |    a    |    b    |  c  |
        }},
       {"simple alpha numeric cyrillic lower case encoding",
        // Using a T3 control char to switch T1 and T2 to a Greek character set
        "абв123",
        0,
        58,  // 8 header bits + 6 symbols * 5 bits
        {
            0b00001010,  // Length = 10
            0b11100'111, 0b11'00010'0, 0b0000'0000, 0b1'00010'11, 0b110'00001, 0b00010'000, 0b11'000000,
            // 28  |   T3    |  2  |    a    |    b    |  c  |    T2   |   1  |   2   |    3    |
        }},
       {"Greek Capital case Encoding",
        // Using a T3 control char to switch T1 and T2 to a Greek character set
        "Αβγ",
        0,
        48,  // 8 header bits + 7 symbols * 5 bits
        {
            0b00001000,  // Length = 7
            0b11101'111, 0b00'11111'0, 0b0001'0000, 0b0'11101'00, 0b001'00010,
            // T1  |    28   | T3  |    1    |   a     |  a  |    b    |  c
        }},
       {"Greek encoding with extended chars",
        // Using Greek character sets, and T3 control char to encode an extended value
        "αβγ!",
        0,
        53,  // 8 header bits + 6 symbols * 5 bits
        {
            0b000001001,  // Length = 9
            0b11100'111, 0b11'00001'0, 0b0000'0000, 0b1'00010'10, 0b001'11111, 0b11001'000,
            // 28  |   T3    |  1  |    a    |    b    |  c  |  r (!)  | T3  |  25    |
        }},
       {"Greek letter inline with a standard alphabet",
        // Using Greek character sets, and T3 control char to encode an extended value
        "10Ωs",
        0,
        53,  // 8 header bits + 6 symbols * 5 bits
        {
            0b000001001,  // Length = 9
            0b11110'000, 0b01'00000'1, 0b1101'1100, 0b0'11111'00, 0b001'11101, 0b10010'000,
            // T2  |    1    |  0  |    T1   |   y(Ω)  |  T3 |    1    |  T1 | s
        }},
       // Double code encoded strings
       // ===========================
       {"Encode strings of double code values",
        // Using T3 table lock to efficiently encode strings of more then 1 double code values
        "だぢづ",
        0,
        48,  // 8 header bits + 6 symbols * 5 bits
        {
            0b000001000,  // Length = 8
            0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b011'01101,
            // T3  |    T1   |  d  |    11   |   d     |  12 |    d    |  13 |
        }},
       {"Encode strings of double code values with table change",
        //  While in multi char mode, table switches are still emitted as per when in single char mode
        "だダヂ",
        0,
        53,  // 8 header bits + 6 symbols * 5 bits
        {
            0b000001001,  // Length = 8
            0b11111'111, 0b01'00011'0, 0b1011'1110, 0b1'00011'01, 0b011'00011, 0b01100000
            // T3  |    T1   |  d  |    11   |   T1     |  d  |    11   |  d  | 13
        }},
       {"Encode strings of double code values and switch back to single code",
        // While in multi char mode, chains of non extended char trigger a return to single char mode
        "だぢづabc",
        0,
        68,  // 8 header bits + 6 symbols * 5 bits
        {
            0b000001100,  // Length = 12
            0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b011'01101, 0b11111'000, 0b00'00001'0, 0b0010'0000
            // T3  |    T1   |  d  |    11   |   d     |  12 |    d    |  13 |   T3   |    a    |   b |   c
        }},
       {
           "Encode strings of always as double code values when interleaved single code",
           // While in multi char mode, chains of non extended char trigger a return to single char mode
           "だぢbづづ",
           0,
           68,  // 8 header bits + 6 symbols * 5 bits
           {
               0b000001100,  // Length = 12
               0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b001'00000, 0b00011'011, 0b01'00011'0,
               0b1101'0000,
               // T3  |    T1   |  d  |    11   |   d     |  12 |    b    |  0 |   d.    | 13      | d   |    13   |
           },
       }

      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = B5StringField<>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_bits, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_bits);
  }
}

TEST_CASE("FiveBitStringBitField<char, Null Terminated>: 5-bit strings (header + controls + multibyte)",
          "[FiveBitStringBitField][5-bit][string]") {
  auto [test_name, value, padding, expected_bits, expected_bytes] = GENERATE(
      table<std::string,          // test_name
            std::string,          // value
            uint8_t,              // padding
            size_t,               // expected_bits (no padding)
            std::vector<uint8_t>  // expected_bytes
            >({                   // Standard Text encoding
               // =======================
               {"Simple Lower Case Encoding",
                "abc",
                0,
                25,  // 8 header bits + 3 symbols * 5 bits
                {
                    0b00000'000, 0b01'00010'1, 0b1111'1111, 0b1'0000000
                    // a   | b      | c   |   T3   |   T3   |
                }},
               {"Simple numeric Encoding",
                // Using a T2 control char to switch to the numeric table
                "123.4",
                0,
                40,  // 8 header bits + 6 symbols * 5 bits
                {
                    0b11110'000, 0b01'00010'0, 0b0011'1000, 0b1'00100'11, 0b111'11111
                    // T2  |   1    | 2   |   3    |   .    |  4  |   T3.  | T3
                }},

               // Extended Text encoding using character modifiers
               // ================================================
               {"Encoding with character modifiers",
                // Using a T3 control char to modify the previous char with the subsequent modifier code.
                // The T3 control char acts as a table shift operation, rather then a lock. Ie T3 control char
                // only effects the next char
                "hällo",
                0,
                45,
                {
                    0b00111'000, 0b00'11111'0, 0b1111'0101, 0b1'01011'01, 0b110'11111, 0b11111'000
                    //  h  |   a     | T3  |    ◌̈    |    l    |  l  |     o   |  T3  |   T3  |
                }},
               // Double code encoded strings
               // ===========================
               {"Encode strings of double code values",
                // Using T3 table lock to efficiently encode strings of more then 1 double code values
                "だぢづ",
                0,
                50,
                {
                    0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b011'01101, 0b11111'111, 0b11'000000
                    // T3  |    T1   |  d  |    11   |   d     |  12 |    d    |  13 |   T3   |   T3    |
                }},
               {"Encode strings of double code values with table change",
                //  While in multi char mode, table switches are still emitted as per when in single char mode
                "だダヂ",
                0,
                55,
                {
                    0b11111'111, 0b01'00011'0, 0b1011'1110, 0b1'00011'01, 0b011'00011, 0b01100'111, 0b11'11111'0
                    // T3  |    T1   |  d  |    11   |   T1    |  d  |    11   |  d  |   13   |   T3    |  T3
                }}}));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = B5StringFieldNullTerminated<>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_bits, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_bits);
  }
}

TEST_CASE("FiveBitStringBitField<char>: 10-bit strings (header + controls + multibyte)",
          "[FiveBitStringBitField][10-bit][string]") {
  auto [test_name, value, padding, expected_bits, expected_bytes] =
      GENERATE(table<std::string,          // test_name
                     std::string,          // value
                     uint8_t,              // padding
                     size_t,               // expected_bits (no padding)
                     std::vector<uint8_t>  // expected_bytes
                     >({
          // Double code encoded strings
          // ===========================
          {"Encode strings of double code values",
           // Using T3 table lock to efficiently encode strings of more then 1 double code values
           "だぢ",
           0,
           28,  // 8 header bits + 6 symbols * 5 bits
           {
               0b000000100,  // Length = 8
               0b00011'010, 0b11'00011'0, 0b1100'0000,
               // d   |    11   |  d  |    12   |
           }},
      }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = B10StringField<255>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_bits, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_bits);
  }
}

TEST_CASE("FiveBitStringBitField<char>: README roundtrip", "[FiveBitStringBitField][5-bit][string][readme]") {
  // Big MaxSymbols so the README fits comfortably
  const size_t max_len = 65535 * 8;
  using T_SUT = B5StringField<max_len>;  // 65535

  // Read ./readmen.md as UTF-8
  std::ifstream in("./readme.md", std::ios::binary);
  REQUIRE(in.good());

  std::string utf8;
  in.seekg(0, std::ios::end);
  const auto file_size = static_cast<std::size_t>(in.tellg());
  in.seekg(0, std::ios::beg);
  utf8.reserve(file_size);
  utf8.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());

  INFO("UTF-8 input size (bytes): " << utf8.size());

  T_SUT input{};
  T_SUT output{};

  REQUIRE(input.set_value(utf8) == BitFieldResult::Ok);

  // Encode into a dynamic bit buffer
  UnboundedBitWriter bw;
  auto write_result = write_field(bw, input);
  REQUIRE(write_result == BitFieldResult::Ok);

  INFO("B5 encoded size: " << bw.storage().size() << " bytes, " << bw.size().bit_size() << " bits");

  // Decode back out
  UnboundedBitReader br(bw.storage(), bw.size());
  auto read_result = read_field(br, output);
  REQUIRE(read_result == BitFieldResult::Ok);

  // Roundtrip: decoded text should match the original UTF-8
  require_strings_match(output.value(), utf8);
}

#endif  // SUB8_ENABLE_FIVE_BIT_STRING && SUB8_ENABLE_STRING_FIELDS__CHAR

// Complex types
// ===========================
 
// Nested Objects 
// ---------------------------

SUB8_DECLARE_DTO(NestedInnerObject, 
  (Uint8ValueField, feild_1), 
  (Uint16ValueField, feild_2));

SUB8_DECLARE_DTO(NestedObject, 
  (NestedInnerObject, inner_1),
  (NestedInnerObject, inner_2)
);

TEST_CASE("Nested Objects", "[Complex-types]") {

  // Arrange
  NestedObject value{
    .inner_1 = NestedInnerObject{
      .feild_1 = {1}, 
      .feild_2 = {2}
    },
    .inner_2 = NestedInnerObject{
      .feild_1 = {3}, 
      .feild_2 = {4}
    }
  };

  size_t expected_length = 48;
  std::vector<uint8_t> expected_bytes = {
    0b00000001, 0b00000000, 0b00000010, 0b00000011, 0b00000000, 0b00000100
  };

  using T_SUT = NestedObject;
  T_SUT input{};
  T_SUT output{};

  // Act + Assert
  REQUIRE(input.set_value(value) == BitFieldResult::Ok);
  REQUIRE_THAT_WRITE_IS_OK(input, 0, expected_length, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_length);
}

// Nested lists of objects 
// ---------------------------


SUB8_DECLARE_DTO(ArrayItem, 
  (Uint8ValueField, feild_1), 
  (Uint16ValueField, feild_2)
);

using ItemArray = ObjectArrayBitField<ArrayItem, 0, 3>;

SUB8_DECLARE_DTO(ObjectArray, 
  (ItemArray, list)
);

TEST_CASE("Nested Array Type", "[Complex-types]") {

  // Arrange
  ObjectArray value {
      .list = ItemArray{
        ArrayItem {
          .feild_1 = {1}, 
          .feild_2 = {2}
        }, 
        ArrayItem {
          .feild_1 = {3}, 
          .feild_2 = {4}
        }
      }
    };

  size_t expected_length = 51;
  std::vector<uint8_t> expected_bytes = {
    0b1'0000000, 0b1'0000000, 0b00000001, 0b0'1'000000, 0b11'000000, 0b00000001, 0b00'0'00000 
    // | 1          | 2                      | | 3          | 4                      | |
  };

  using T_SUT = ObjectArray;
  T_SUT input{};
  T_SUT output{};

  // Act + Assert
  REQUIRE(input.set_value(value) == BitFieldResult::Ok);
  REQUIRE_THAT_WRITE_IS_OK(input, 0, expected_length, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_length);
}
