

#include <catch2/catch_all.hpp>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "./../src/sub8.h"
#include "test_helpers.h"

using namespace sub8;

#define REQUIRE_THAT_WRITE_IS_OK(input, padded_bits, expected_size, expected_vec)                                                          \
  BoundedBitWriter<128> bw;                                                                                                                \
  {                                                                                                                                        \
    auto expected_result = BitFieldResult::Ok;                                                                                             \
    bw.put_padding(padded_bits);                                                                                                           \
    auto r = write_field(bw, input);                                                                                                       \
    /* Compare byte buffer */                                                                                                              \
    REQUIRE(to_binary_string(bw.storage()) == to_binary_string(expected_vec));                                                             \
    REQUIRE(bw.size().bit_size() == expected_size + padded_bits);                                                                          \
    /*REQUIRE(bw.size().bit_size() == input.actual_size().bit_size()); */                                                                  \
                                                                                                                                           \
    /* Compare result using Catch::Matchers */                                                                                             \
    REQUIRE_THAT(std::string(error::to_string(r)), Catch::Matchers::Equals(std::string(error::to_string(expected_result))));               \
  }

#define REQUIRE_THAT_READ_CORE_IS_OK(output, padded_bits, input_bytes, input_bit_size, CHECK_OUTPUT)                                       \
  do {                                                                                                                                     \
    INFO("Input Bytes        = " << to_binary_string(input_bytes));                                                                        \
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

// Assert const exprs are calculated correctly
#define ASSERT_BITFIELD_MINMAX(T_SUT, ExpectedMinValue, ExpectedMaxValue)                                                                  \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::MaxValue == static_cast<typename T_SUT::Type>(ExpectedMaxValue));                                                       \
    REQUIRE(T_SUT::MinValue == static_cast<typename T_SUT::Type>(ExpectedMinValue));                                                       \
  } while (false)

#define ASSERT_FIXED_LENGTH_BIT_FIELD(T_SUT, ExpectedTotalUsableBits, ExpectedMinValue, ExpectedMaxValue)                                  \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::ActualSize.bit_size() == (ExpectedTotalUsableBits));                                                                    \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == (ExpectedTotalUsableBits));                                                               \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (ExpectedTotalUsableBits));                                                               \
    ASSERT_BITFIELD_MINMAX(T_SUT, (ExpectedMinValue), (ExpectedMaxValue));                                                                 \
  } while (false)

#define ASSERT_VARIABLE_LENGTH_BIT_FIELD(T_SUT, ExpectedGroupBitSize, ExpectedMaxGroupCount, ExpectedTotalUsableBits, ExpectedMinValue,    \
  ExpectedMaxValue)                                                                                                                        \
  do {                                                                                                                                     \
    REQUIRE(T_SUT::MaxSegmentsCount == (ExpectedMaxGroupCount));                                                                           \
    REQUIRE(T_SUT::BitWidth == (ExpectedTotalUsableBits));                                                                                 \
    ASSERT_BITFIELD_MINMAX(T_SUT, (ExpectedMinValue), (ExpectedMaxValue));                                                                 \
  } while (false)

// clang-format off

// PathBitField


#if SUB8_ENABLE_ENUM_FIELDS

enum class MsgType : uint8_t {
  Ping = 10,
  Pong = 11,
  Data = 12,
  Ack = 13,
  Nack = 14,
};

TEST_CASE("Enumeration: 3-bit", "[Enumeration][unsigned][4-bit]") {
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
    using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif // SUB8_ENABLE_ENUM_FIELDS

#if SUB8_ENABLE_OPTIONAL_FIELDS

TEST_CASE("Optional: 8-bit", "[Optional][unsigned][8-bit]") {
  auto [test_name, value, padding, expected_len, expected_bytes] =
      GENERATE(table<std::string, sub8::optional<U8>, uint8_t, size_t, std::vector<uint8_t>>(
          {
            {"optional with value",    sub8::optional<U8>(U8(3u)), 0, 9, {0b10000001, 0b10000000}},
            {"optional without value", sub8::optional<U8>(), 0, 1,  {0b00000000}},
          }
      ));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = Optional<U8>;
    T_SUT input{};
    T_SUT output{};
    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    REQUIRE_THAT_WRITE_IS_OK(input, padding, expected_len, expected_bytes);
    REQUIRE_THAT_OPT_READ_IS_OK(output, padding, value, bw.storage(), expected_len);
  }
}

#endif // SUB8_ENABLE_OPTIONAL_FIELDS


// Complex types
// ===========================
 
// Nested Objects 
// ---------------------------

SUB8_DECLARE_DTO(NestedInnerObject, 
  (U8, feild_1), 
  (U16, feild_2));

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
  (U8, feild_1), 
  (U16, feild_2)
);

using ItemArray = ObjectArray<ArrayItem, 0, 3>;

SUB8_DECLARE_DTO(CustomObjectWithArray, 
  (ItemArray, list)
);

TEST_CASE("Nested Array Type", "[Complex-types]") {

  // Arrange
  CustomObjectWithArray value {
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

  using T_SUT = CustomObjectWithArray;
  T_SUT input{};
  T_SUT output{};

  // Act + Assert
  REQUIRE(input.set_value(value) == BitFieldResult::Ok);
  REQUIRE_THAT_WRITE_IS_OK(input, 0, expected_length, expected_bytes);
  REQUIRE_THAT_READ_IS_OK(output, 0, value, bw.storage(), expected_length);
}
