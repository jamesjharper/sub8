#include <catch2/catch_all.hpp>

#include <cstdint>
#include <cstring>
#include <type_traits>

#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "./../src/sub8.h"
#include "test_helpers.h"

using namespace sub8;

#define REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padded_bits, expected_size_bits, expected_vec)                                             \
  BoundedBitWriter<128> bw;                                                                                                                 \
  {                                                                                                                                         \
    const auto expected_result = BitFieldResult::Ok;                                                                                        \
    bw.put_padding(padded_bits);                                                                                                            \
    const auto r = write_field(bw, (input));                                                                                                \
                                                                                                                                            \
    /* Compare byte buffer */                                                                                                               \
    REQUIRE(to_binary_string(bw.storage()) == to_binary_string((expected_vec)));                                                            \
                                                                                                                                            \
    /* Written size includes padding */                                                                                                     \
    REQUIRE(bw.size().bit_size() == (expected_size_bits) + (padded_bits));                                                                  \
                                                                                                                                            \
    /* Field reports its own written size (excluding padding) */                                                                            \
    REQUIRE((input).actual_size().bit_size() == (expected_size_bits));                                                                      \
                                                                                                                                            \
    /* Compare result using Catch matchers */                                                                                               \
    REQUIRE_THAT(std::string(error::to_string(r)),                                                                                          \
                 Catch::Matchers::Equals(std::string(error::to_string(expected_result))));                                                  \
  }

#define ASSERT_BIT_SIZES_VARIABLE(T_SUT, obj, expected_actual_bits, expected_min_bits, expected_max_bits)                                   \
  do {                                                                                                                                      \
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == (expected_min_bits));                                                                      \
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == (expected_max_bits));                                                                      \
                                                                                                                                           \
    REQUIRE((obj).min_possible_size().bit_size() == (expected_min_bits));                                                                   \
    REQUIRE((obj).max_possible_size().bit_size() == (expected_max_bits));                                                                   \
    REQUIRE((obj).actual_size().bit_size() == (expected_actual_bits));                                                                      \
  } while (false)


#define ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(obj, expected_bits)                                                                           \
  do {                                                                                                                                      \
    REQUIRE((obj).actual_size().bit_size() == (expected_bits));                                                                             \
  } while (false)

#define REQUIRE_THAT_READ_CORE_IS_OK(output, padded_bits, input_bytes, input_bit_size, CHECK_OUTPUT)                                        \
  do {                                                                                                                                      \
    INFO("Input Bytes        = " << to_binary_string((input_bytes)));                                                                       \
    INFO("Input BitSize      = " << (input_bit_size));                                                                                      \
                                                                                                                                           \
    BoundedBitReader<128> br((input_bytes), BitSize::from_bits((input_bit_size) + (padded_bits)));                                          \
    br.set_cursor_position(BitSize::from_bits((padded_bits)));                                                                              \
                                                                                                                                           \
    const auto r = read_field(br, (output));                                                                                                \
    INFO("Read Cursor Pos    = " << br.cursor_position().bit_size());                                                                       \
    const auto expected_result = BitFieldResult::Ok;                                                                                        \
                                                                                                                                           \
    /* Custom check for output vs expected */                                                                                               \
    CHECK_OUTPUT;                                                                                                                           \
                                                                                                                                           \
    const auto expected_cursor_position = BitSize::from_bits((input_bit_size) + (padded_bits));                                             \
    REQUIRE(br.cursor_position().bit_size() == expected_cursor_position.bit_size());                                                        \
                                                                                                                                           \
    REQUIRE_THAT(std::string(error::to_string(r)),                                                                                          \
                 Catch::Matchers::Equals(std::string(error::to_string(expected_result))));                                                  \
  } while (false)

#define REQUIRE_THAT_READ_IS_OK(output, padded_bits, expected_value, input_bytes, input_bit_size)                                           \
  REQUIRE_THAT_READ_CORE_IS_OK((output), (padded_bits), (input_bytes), (input_bit_size), REQUIRE((output).value() == (expected_value)))

// clang-format off

#if SUB8_ENABLE_FIVE_BIT_STRING && SUB8_ENABLE_STRING_FIELDS__CHAR

// -------------------------------------------------------------
// 5-bit strings (length-prefixed): golden vectors + size checks
// -------------------------------------------------------------
TEST_CASE("FiveBitStringBitField<char>: 5-bit strings (header + controls + multibyte) + size APIs",
          "[FiveBitStringBitField][5-bit][string][size]") {
  auto [test_name, value, padding, expected_bits, expected_bytes] =
    GENERATE(table<std::string,          // test_name
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
          0b00000011,  // Length = 3
          0b00000'000, 0b01'00010'0,
          // a   | b       | c
        }},

       {"Simple numeric Encoding",
        "123.4",
        0,
        38,  // 8 header bits + 6 symbols * 5 bits
        {
          0b00000110,  // Length = 6
          0b11110'000, 0b01'00010'0, 0b0011'1000, 0b1'00100'00
          // T2  |   1     | 2   |   3     |   .     |  4
        }},

       {"Capital case Encoding",
        "Abc",
        0,
        33,  // 8 header bits + 5 symbols * 5 bits
        {
          0b00000101,  // Length = 5
          0b11101'000, 0b00'11101'0, 0b0001'0001, 0b0'00000'00
          // T1  |   A     |  T1 |    b    |   c
        }},

       {"Character Encoding with numbers",
        "h3ll0",
        0,
        48,  // 8 header bits + 8 symbols * 5 bits
        {
          0b00001000,  // Length = 8
          0b00111'111, 0b10'00011'1, 0b1110'0101, 0b1'01011'11, 0b110'00000
          //  h  |   T2    |  3  |    T0   |   l     |  l  |    T2   | 0
        }},

       {"Encoding with character duplicated across tables",
        "h-LL-",
        0,
        38,  // 8 header bits + 6 symbols * 5 bits
        {
          0b00000110,  // Length = 6
          0b00111'111, 0b00'11101'0, 0b1011'0101, 0b1'11100'00,
          //  h  |   -     | T1  |    L    |    L    |  -
        }},

       // Extended Text encoding using character modifiers
       // ================================================
       {"Encoding with character modifiers",
        "hällo",
        0,
        43,  // 8 header bits + 7 symbols * 5 bits
        {
          0b00000111,  // Length = 7
          0b00111'000, 0b00'11111'0, 0b1111'0101, 0b1'01011'01, 0b110'00000
          //  h  |   a     | T3  |    ◌̈    |    l    |  l  |     o
        }},

       {"Encoding capital letters with character modifiers",
        "HÄLLO",
        0,
        48,  // 8 header bits + 8 symbols * 5 bits
        {
          0b00001000,  // Length = 8
          0b11101'001, 0b11'00000'1, 0b1111'0111, 0b1'01011'01, 0b011'01110
          // T1  |   H     |  A  |    T3   |    ◌̈    |  L  |     L   |  O
        }},

       {"simple Greek lower case encoding",
        "αβγ",
        0,
        38,  // 8 header bits + 6 symbols * 5 bits
        {
          0b00000110,  // Length = 6
          0b11100'111, 0b11'00001'0, 0b0000'0000, 0b1'00010'00,
          // 28  |   T3    |  1  |    a    |    b    |  c
        }},

       {"simple alpha numeric cyrillic lower case encoding",
        "абв123",
        0,
        58,  // 8 header bits + 10 symbols * 5 bits
        {
          0b00001010,  // Length = 10
          0b11100'111, 0b11'00010'0, 0b0000'0000, 0b1'00010'11, 0b110'00001, 0b00010'000, 0b11'000000,
          // 28  |   T3    |  2  |    a    |    b    |  c  |    T2   |   1  |   2   |    3
        }},

       {"Greek Capital case Encoding",
        "Αβγ",
        0,
        48,  // 8 header bits + 8 symbols * 5 bits
        {
          0b00001000,  // Length = 8
          0b11101'111, 0b00'11111'0, 0b0001'0000, 0b0'11101'00, 0b001'00010,
          // T1  |    28   | T3  |    1    |   a     |  a  |    b    |  c
        }},

       {"Greek encoding with extended chars",
        "αβγ!",
        0,
        53,  // 8 header bits + 9 symbols * 5 bits
        {
          0b000001001,  // Length = 9 (note: fits in 8-bit header for this type)
          0b11100'111, 0b11'00001'0, 0b0000'0000, 0b1'00010'10, 0b001'11111, 0b11001'000,
          // 28  |   T3    |  1  |    a    |    b    |  c  |  r (!)  | T3  |  25
        }},

       {"Greek letter inline with a standard alphabet",
        "10Ωs",
        0,
        53,  // 8 header bits + 9 symbols * 5 bits
        {
          0b000001001,  // Length = 9
          0b11110'000, 0b01'00000'1, 0b1101'1100, 0b0'11111'00, 0b001'11101, 0b10010'000,
          // T2  |    1    |  0  |    T1   |   y(Ω)  |  T3 |    1    |  T1 | s
        }},

       // Double code encoded strings
       // ===========================
       {"Encode strings of double code values",
        "だぢづ",
        0,
        48,  // 8 header bits + 8 symbols * 5 bits
        {
          0b000001000,  // Length = 8
          0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b011'01101,
          // T3  |    T1   |  d  |    11   |   d     |  12 |    d    |  13
        }},

       {"Encode strings of double code values with table change",
        "だダヂ",
        0,
        53,  // 8 header bits + 9 symbols * 5 bits
        {
          0b000001001,  // Length = 9
          0b11111'111, 0b01'00011'0, 0b1011'1110, 0b1'00011'01, 0b011'00011, 0b01100000
          // T3  |    T1   |  d  |    11   |   T1     |  d  |    11   |  d  | 13
        }},

       {"Encode strings of double code values and switch back to single code",
        "だぢづabc",
        0,
        68,  // 8 header bits + 12 symbols * 5 bits
        {
          0b000001100,  // Length = 12
          0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b011'01101, 0b11111'000, 0b00'00001'0, 0b0010'0000
          // T3  |    T1   |  d  |    11   |   d     |  12 |    d    |  13 |   T3   |    a    |   b |   c
        }},

       {"Encode strings always as double code values when interleaved single code",
        "だぢbづづ",
        0,
        68,  // 8 header bits + 12 symbols * 5 bits
        {
          0b000001100,  // Length = 12
          0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b001'00000, 0b00011'011, 0b01'00011'0, 0b1101'0000
          // T3  |    T1   |  d  |    11   |   d     |  12 |    b    |  0  |   d     |  13     | d   |  13
        }},

       // Same vectors, but with padding to exercise cursor math (and internal 0-bit decode steps).
       {"Simple Lower Case Encoding (padding=3)",
        "abc",
        3,
        23,
        {
          // without padding
          // 0b00000011, 
          // 0b00000'000, 0b01'00010'0,
          0b000'00000, 0b011'00000, 0b00001'000, 0b10'000000,
        }},

       {"Numeric Encoding (padding=7)",
        "123.4",
        7,
        38,
        {
          // without padding
          //0b00000110,
          //0b11110'000, 0b01'00010'0, 0b0011'1000, 0b1'00100'00
          0b0000000'0, 0b0000110'1, 0b1110'0000, 0b1'00010'00, 0b011'10001, 0b00100'000
        }},
      }
    ));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = UnboundedB5String<128>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    // Type-level size bounds sanity (variable-length field)
    // For UnboundedB5String<128> (MaxSymbols=128, TerminatedSequence=false)
    // MinPossible = header bits only (LengthPrefixBitWidth) ; MaxPossible = header + 5*MaxSymbols
    {
      constexpr uint8_t len_bits = limits::bitwidth_to_express_max_value(size_t{128});
      const size_t expected_min = len_bits;                 // empty string length prefix only
      const size_t expected_max = len_bits + (5u * 128u);   // worst-case
      ASSERT_BIT_SIZES_VARIABLE(T_SUT, input, expected_bits, expected_min, expected_max);
    }

    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_bits);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_bits, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_bits);

    // After reading, actual_size() should equal what was read (and match original expected bits for this test row).
    REQUIRE(output.actual_size().bit_size() == expected_bits);
  }
}

// -------------------------------------------------------------
// 5-bit strings (null terminated): golden vectors + size checks
// -------------------------------------------------------------
TEST_CASE("FiveBitStringBitField<char, Null Terminated>: 5-bit strings (no header, terminator) + size APIs",
          "[FiveBitStringBitField][5-bit][string][null_terminated][size]") {
  auto [test_name, value, padding, expected_bits, expected_bytes] = GENERATE(
    table<std::string, std::string, uint8_t, size_t, std::vector<uint8_t>>({
      {"Simple Lower Case Encoding",
       "abc",
       0,
       25,  // 3 symbols * 5 + 2 terminator symbols * 5
       {
         0b00000'000, 0b01'00010'1, 0b1111'1111, 0b1'0000000
         // a | b | c | T3 | T3
       }},

      {"Simple numeric Encoding",
       "123.4",
       0,
       40 , // 6 symbols * 5 + 2 terminator symbols * 5
       {
         0b11110'000, 0b01'00010'0, 0b0011'1000, 0b1'00100'11, 0b111'11111
         // T2 | 1 | 2 | 3 | . | 4 | T3 | T3
       }},

      {"Encoding with character modifiers",
       "hällo",
       0,
       45, // 7 symbols * 5 + 2 terminator symbols * 5
       {
         0b00111'000, 0b00'11111'0, 0b1111'0101, 0b1'01011'01, 0b110'11111, 0b11111'000
         // h | a | T3 | diaeresis | l | l | o | T3 | T3
       }},

      {"Encode strings of double code values",
       "だぢづ",
       0,
       50 , // 8 symbols * 5 + 2 terminator symbols * 5
       {
         0b11111'111, 0b01'00011'0, 0b1011'0001, 0b1'01100'00, 0b011'01101, 0b11111'111, 0b11'000000
         // ... | T3 | T3
       }},

      {"Encode strings of double code values with table change",
       "だダヂ",
       0,
       55, // 9 symbols * 5 + 2 terminator symbols * 5
       {
         0b11111'111, 0b01'00011'0, 0b1011'1110, 0b1'00011'01, 0b011'00011, 0b01100'111, 0b11'11111'0
         // ... | T3 | T3
       }},

      {"Simple Lower Case Encoding (padding=5)",
       "abc",
       5,
       25,
       {
         0b00000'000, 0b00'00001'0, 0b0010'1111, 0b1'11111'00 
       }},
    })
  );

  DYNAMIC_SECTION(test_name) {
    using T_SUT = BoundedB5StringNullTerminated<64>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    // For TerminatedSequence=true: MinPossible includes the 2x 5-bit terminator codes.
    {
      constexpr size_t expected_min = 10;            // 2 terminator symbols * 5
      constexpr size_t expected_max = 5u * 64u;      // worst-case uses up to MaxSymbols data symbols; MinPossible already accounts for terminator
      ASSERT_BIT_SIZES_VARIABLE(T_SUT, input, expected_bits, expected_min, expected_max);
    }

    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_bits);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_bits, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_bits);
    REQUIRE(output.actual_size().bit_size() == expected_bits);
  }
}

// -------------------------------------------------------------
// 10-bit mode: roundtrip + size check
// -------------------------------------------------------------
TEST_CASE("FiveBitStringBitField<char>: 10-bit strings (header + controls + multibyte) + size APIs",
          "[FiveBitStringBitField][10-bit][string][size]") {
  auto [test_name, value, padding, expected_bits, expected_bytes] =
    GENERATE(table<std::string, std::string, uint8_t, size_t, std::vector<uint8_t>>({
      {"Encode strings of double code values",
       "だぢ",
       0,
       28,  // 8 header bits + 4 symbols * 5 bits
       {
         0b000000100,  // Length = 4
         0b00011'010, 0b11'00011'0, 0b1100'0000,
         // d | 11 | d | 12
       }},
      {"Encode strings of double code values (padding=2)",
       "だぢ",
       2,
       28,
       {
         // add 2 bits padding
         0b000000001,
         0b00'00011'0, 0b1011'0001, 0b1'01100'00,
       }},
    }));

  DYNAMIC_SECTION(test_name) {
    using T_SUT = UnboundedB10String<255>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value(value) == BitFieldResult::Ok);

    // Basic bounds sanity
    {
      constexpr uint8_t len_bits = limits::bitwidth_to_express_max_value(size_t{255});
      const size_t expected_min = len_bits;
      const size_t expected_max = len_bits + (5u * 255u);
      ASSERT_BIT_SIZES_VARIABLE(T_SUT, input, expected_bits, expected_min, expected_max);
    }

    ASSERT_WRITE_SIZE_MATCHES_ACTUAL_SIZE(input, expected_bits);

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, padding, expected_bits, expected_bytes);
    REQUIRE_THAT_READ_IS_OK(output, padding, value, bw.storage(), expected_bits);
    REQUIRE(output.actual_size().bit_size() == expected_bits);
  }
}

TEST_CASE("FiveBitStringBitField<char>: empty string encodes/decodes correctly",
          "[FiveBitStringBitField][5-bit][string][empty]") {
  SECTION("Length-prefixed (empty)") {
    using T_SUT = UnboundedB5String<128>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value("") == BitFieldResult::Ok);

    // For MaxSymbols=128 -> header bitwidth = 8, so empty string actual_size == 8
    REQUIRE(input.actual_size().bit_size() == 8);

    // Expect just a length=0 byte (8 bits), no symbol payload.
    const std::vector<uint8_t> expected = { 0b00000000 };

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, /*padding*/0, /*expected_bits*/8, expected);
    REQUIRE_THAT_READ_IS_OK(output, /*padding*/0, std::string(""), bw.storage(), /*input_bit_size*/8);
  }

  SECTION("Null-terminated (empty)") {
    using T_SUT = BoundedB5StringNullTerminated<64>;
    T_SUT input{};
    T_SUT output{};

    REQUIRE(input.set_value("") == BitFieldResult::Ok);

    // Empty terminated sequence should be just terminator (T3|T3) => 2 symbols => 10 bits
    REQUIRE(input.actual_size().bit_size() == 10);

    // Terminator is emitted as T3 then T3; exact bit packing depends on writer ordering.
    // This is a small, stable golden vector consistent with existing tests:
    const std::vector<uint8_t> expected = {
      0b11111'111, 0b11'000000
      // T3 | T3
    };

    REQUIRE_THAT_WRITE_IS_OK_AND_SIZE(input, /*padding*/0, /*expected_bits*/10, expected);
    REQUIRE_THAT_READ_IS_OK(output, /*padding*/0, std::string(""), bw.storage(), /*input_bit_size*/10);
  }
}

TEST_CASE("FiveBitStringBitField<char>: length-prefixed overflow returns ErrorValueOverflow",
          "[FiveBitStringBitField][5-bit][string][overflow]") {
  using T_SUT = UnboundedB5String<5>; // MaxSymbols=5

  T_SUT input{};
  REQUIRE(input.set_value("abcdef") == BitFieldResult::Ok); // 6 chars -> at least 6 symbols in simplest case

  UnboundedBitWriter bw;
  const auto r = write_field(bw, input);

  REQUIRE(r == BitFieldResult::ErrorValueOverflow);
}


TEST_CASE("FiveBitStringBitField<char>: decoder clamps overlarge header to MaxSymbols (by decoding larger stream)",
          "[FiveBitStringBitField][5-bit][string][decoder][header_clamp]") {
  // We'll produce a stream with MaxSymbols=15 (header width = 4 bits),
  // containing 15 symbols, then decode it with MaxSymbols=10 and ensure:
  //  - output is first 10 symbols
  //  - reader stops after consuming 10 symbols (not 15)

  using T_ENC = UnboundedB5String<15>; // MaxSymbols=15 -> LengthPrefixBitWidth=4
  using T_DEC = UnboundedB5String<10>; // MaxSymbols=10 -> LengthPrefixBitWidth=4

  // 15 symbols of plain lower-case (each is 1 symbol in your table)
  const std::string s15 = "abcdefghijklmno";
  REQUIRE(s15.size() == 15);

  T_ENC in{};
  REQUIRE(in.set_value(s15) == BitFieldResult::Ok);

  UnboundedBitWriter bw;
  REQUIRE(write_field(bw, in) == BitFieldResult::Ok);

  // Decode using smaller MaxSymbols=10 -> decoder clamps header length to 10.
  UnboundedBitReader br(bw.storage(), bw.size());
  T_DEC out{};
  REQUIRE(read_field(br, out) == BitFieldResult::Ok);

  // Expect first 10 characters only
  require_strings_match(out.view(), std::string_view(s15).substr(0, 10));

  // And ensure we *only consumed* 4(header) + 10*5 bits from the stream.
  // (Header width is 4 for both MaxSymbols=10 and 15)
  constexpr size_t header_bits = 4;
  constexpr size_t symbol_bits = 5;
  const size_t expected_consumed = header_bits + (10 * symbol_bits);
  REQUIRE(br.cursor_position().bit_size() == expected_consumed);
}

TEST_CASE("FiveBitStringBitField<char>: invalid UTF-8 bytes roundtrip as U+FFFD",
          "[FiveBitStringBitField][5-bit][string][utf8][invalid]") {
  using T_SUT = UnboundedB5String<128>;

  // Construct: "A" + invalid byte 0xFF + "B"
  std::string s;
  s.push_back('A');
  s.push_back(static_cast<char>(0xFF));
  s.push_back('B');

  T_SUT input{};
  REQUIRE(input.set_value(s) == BitFieldResult::Ok);

  UnboundedBitWriter bw;
  REQUIRE(write_field(bw, input) == BitFieldResult::Ok);

  UnboundedBitReader br(bw.storage(), bw.size());
  T_SUT out{};
  REQUIRE(read_field(br, out) == BitFieldResult::Ok);

  // Expected UTF-8: "A" + U+FFFD (EF BF BD) + "B"
  const std::string expected = std::string("A") + "\xEF\xBF\xBD" + "B";
  require_strings_match(out.view(), std::string_view(expected));
}

// -------------------------------------------------------------
// README roundtrip
// -------------------------------------------------------------
TEST_CASE("FiveBitStringBitField<char>: README roundtrip", "[FiveBitStringBitField][5-bit][string][readme]") {
  // Big MaxSymbols so the README fits comfortably
  const size_t max_len = 65535 * 8;
  using T_SUT = BoundedB5String<max_len>;

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

  // Type-level bounds should be consistent
  {
    constexpr uint8_t len_bits = limits::bitwidth_to_express_max_value(max_len);
    const size_t expected_min = len_bits;
    const size_t expected_max = len_bits + (5u * max_len);
    REQUIRE(T_SUT::MinPossibleSize.bit_size() == expected_min);
    REQUIRE(T_SUT::MaxPossibleSize.bit_size() == expected_max);
  }

  // Encode into a dynamic bit buffer
  UnboundedBitWriter bw;
  const auto write_result = write_field(bw, input);
  REQUIRE(write_result == BitFieldResult::Ok);

  INFO("B5 encoded size: " << bw.storage().size() << " bytes, " << bw.size().bit_size() << " bits");
  REQUIRE(input.actual_size().bit_size() == bw.size().bit_size());

  // Decode back out
  UnboundedBitReader br(bw.storage(), bw.size());
  const auto read_result = read_field(br, output);
  REQUIRE(read_result == BitFieldResult::Ok);

  require_strings_match(output.view(), std::string_view(utf8));
  REQUIRE(output.actual_size().bit_size() == bw.size().bit_size());
}

#endif  // SUB8_ENABLE_FIVE_BIT_STRING && SUB8_ENABLE_STRING_FIELDS__CHAR
