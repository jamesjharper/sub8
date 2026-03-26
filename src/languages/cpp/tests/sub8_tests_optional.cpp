

#include <catch2/catch_all.hpp>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "./../src/sub8_optional.h"
#include "./../src/sub8_primitives.h"
#include "test_helpers.h"

using namespace sub8;


#if SUB8_ENABLE_OPTIONAL_FIELDS

// ---------------------------------------------
// Extra helpers: round-trip and direct read checks
// ---------------------------------------------

#define REQUIRE_OPTIONAL_ROUNDTRIP_IS_OK(field_type, opt_value_expr, padding_bits)                                                         \
  do {                                                                                                                                     \
    using T_SUT = Optional<field_type>;                                                                                                    \
    T_SUT input{};                                                                                                                         \
    T_SUT output{};                                                                                                                        \
    REQUIRE(input.set_value((opt_value_expr)) == BitFieldResult::Ok);                                                                      \
                                                                                                                                           \
    BoundedBitWriter<128> bw;                                                                                                               \
    bw.put_padding((padding_bits));                                                                                                         \
    const auto wr = write_field(bw, input);                                                                                                 \
    REQUIRE(wr == BitFieldResult::Ok);                                                                                                     \
                                                                                                                                           \
    BoundedBitReader<128> br(bw.storage(), bw.size());                                                                                      \
    br.set_cursor_position(BitSize::from_bits((padding_bits)));                                                                             \
    const auto rr = read_field(br, output);                                                                                                 \
    REQUIRE(rr == BitFieldResult::Ok);                                                                                                     \
                                                                                                                                           \
    /* round-trip equality */                                                                                                               \
    REQUIRE(output == (opt_value_expr));                                                                                                   \
    /* cursor must end exactly at written size */                                                                                           \
    REQUIRE(br.cursor_position().bit_size() == bw.size().bit_size());                                                                       \
    /* actual_size should match what was written (minus padding) */                                                                         \
    REQUIRE(input.actual_size().bit_size() + (padding_bits) == bw.size().bit_size());                                                      \
  } while (false)

#define REQUIRE_OPTIONAL_READ_FAILS_AND_REMAINS_EMPTY(field_type, padding_bits, bb, input_bits)                                             \
  do {                                                                                                                                     \
    using T_SUT = Optional<field_type>;                                                                                                    \
    T_SUT output{};                                                                                                                        \
                                                                                                                                           \
    BoundedBitReader<128> br((bb), BitSize::from_bits((input_bits) + (padding_bits)));                                                     \
    br.set_cursor_position(BitSize::from_bits((padding_bits)));                                                                            \
    const auto r = read_field(br, output);                                                                                                 \
    REQUIRE(r != BitFieldResult::Ok);                                                                                                      \
    REQUIRE_FALSE(output.has_value());                                                                                                     \
  } while (false)


TEST_CASE("Optional<U8>: round-trip across paddings and values", "[Optional][U8][roundtrip][padding]") {
  using V = sub8::optional<U8>;

  auto [name, value] = GENERATE(table<std::string, V>({
      {"none", V{}},
      {"zero", V{U8(0u)}},
      {"one",  V{U8(1u)}},
      {"three", V{U8(3u)}},
      {"max",  V{U8(255u)}},
  }));

  auto padding = GENERATE(range<uint8_t>(0, 8)); // 0..7

  DYNAMIC_SECTION(name << " padding=" << int(padding)) {
    REQUIRE_OPTIONAL_ROUNDTRIP_IS_OK(U8, value, padding);
  }
}


// ---------------------------------------------
// 2) Round-trip for multi-byte primitives (U16/U32)
// ---------------------------------------------
TEST_CASE("Optional<U16>: round-trip across paddings and boundaries", "[Optional][U16][roundtrip][padding]") {
  using V = sub8::optional<U16>;
  auto [name, value] = GENERATE(table<std::string, V>({
      {"none", V{}},
      {"small", V{U16(1u)}},
      {"mid", V{U16(0x1234u)}},
      {"max", V{U16(0xFFFFu)}},
  }));

  auto padding = GENERATE(range<uint8_t>(0, 8));

  DYNAMIC_SECTION(name << " padding=" << int(padding)) {
    REQUIRE_OPTIONAL_ROUNDTRIP_IS_OK(U16, value, padding);
  }
}

TEST_CASE("Optional<U32>: round-trip across paddings and boundaries", "[Optional][U32][roundtrip][padding]") {
  using V = sub8::optional<U32>;
  auto [name, value] = GENERATE(table<std::string, V>({
      {"none", V{}},
      {"small", V{U32(1u)}},
      {"mid", V{U32(0x12345678u)}},
      {"max", V{U32(0xFFFFFFFFu)}},
  }));

  auto padding = GENERATE(range<uint8_t>(0, 8));

  DYNAMIC_SECTION(name << " padding=" << int(padding)) {
    REQUIRE_OPTIONAL_ROUNDTRIP_IS_OK(U32, value, padding);
  }
}


// ---------------------------------------------
// Malformed input: present bit set but payload truncated
//    This should fail and output should remain empty.
// ---------------------------------------------
TEST_CASE("Optional<U8>: decoder fails on present=1 but missing payload", "[Optional][U8][decoder][truncated]") {
  // present=1, stream ends immediately (only 1 bit available)
  using T_SUT = Optional<U8>;
  T_SUT output{};

  uint8_t buff[] = {0b10000000};

  auto br = make_reader(buff, BitSize::from_bits(1));
  const auto r = read_field(br, output);
  REQUIRE(r != BitFieldResult::Ok);
  REQUIRE_FALSE(output.has_value());
}


// ---------------------------------------------
// 4) present=0 should consume only the flag bit (even if stream has extra bits)
//    Ensures Optional doesn't accidentally read more than needed.
// ---------------------------------------------
TEST_CASE("Optional<U8>: present=0 consumes only 1 bit", "[Optional][U8][decoder][cursor]") {
  using T_SUT = Optional<U8>;
  T_SUT out{};

  // Provide 16 bits total, but Optional should only consume 1 bit because present=0.
  // First bit is 0, remaining bits are garbage and should remain unread.
  uint8_t bytes[] = {0b00000000, 0b11111111};

  auto br = make_reader(bytes, BitSize::from_bits(16));
  const auto r = read_field(br, out);
  REQUIRE(r == BitFieldResult::Ok);
  REQUIRE_FALSE(out.has_value());
  REQUIRE(br.cursor_position().bit_size() == 1);
}


// ---------------------------------------------
// 5) Optional actual_size sanity (none vs some)
// ---------------------------------------------
TEST_CASE("Optional<U8>: actual_size matches flag + payload rule", "[Optional][U8][size]") {
  using T_SUT = Optional<U8>;

  {
    T_SUT x{};
    REQUIRE_FALSE(x.has_value());
    REQUIRE(x.actual_size().bit_size() == 1); // flag only
  }

  {
    T_SUT x{};
    REQUIRE(x.set_value(sub8::optional<U8>(U8(7u))) == BitFieldResult::Ok);
    REQUIRE(x.has_value());
    // 1 flag bit + 8 bits payload
    REQUIRE(x.actual_size().bit_size() == 9);
  }
}

#endif // SUB8_ENABLE_OPTIONAL_FIELDS
