#include <catch2/catch_all.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "./../src/sub8_enums.h"
#include "test_helpers.h"

using namespace sub8;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

// Convert std::vector<uint8_t> -> BoundedByteBuffer<N>
template <size_t N>
static BoundedByteBuffer<N> make_bounded_buf(const std::vector<uint8_t> &v) noexcept {
  BoundedByteBuffer<N> out{};
  for (auto b : v) {
    (void)out.push_back(b); // tests will fail later if overflow happens; keep noexcept
  }
  return out;
}

template <size_t NBytes, typename Field>
static BoundedByteBuffer<NBytes> write_with_padding(const Field &input,
                                                    uint8_t padded_bits,
                                                    BitFieldResult &out_r,
                                                    size_t &out_total_bits) noexcept {
  BoundedBitWriter<NBytes> bw;
  (void)bw.put_padding(padded_bits);
  out_r = write_field(bw, input);
  out_total_bits = bw.size().bit_size();
  return bw.storage(); // returns by value (BoundedByteBuffer is trivially copyable)
}

template <size_t NBytes, typename Field>
static BitFieldResult read_with_padding(Field &output,
                                        uint8_t padded_bits,
                                        BoundedByteBuffer<NBytes> &input_storage,
                                        size_t payload_bits) noexcept {
  // total bits includes padding
  BoundedBitReader<NBytes> br(input_storage, BitSize::from_bits(payload_bits + padded_bits));
  br.set_cursor_position(BitSize::from_bits(padded_bits));

  const auto r = read_field(br, output);
  REQUIRE(br.cursor_position().bit_size() == payload_bits + padded_bits);
  return r;
}

#define REQUIRE_RESULT_IS(r, expected)                                                                                                     \
  REQUIRE_THAT(std::string(error::to_string((r))), Catch::Matchers::Equals(std::string(error::to_string((expected)))))

#define REQUIRE_OK(r) REQUIRE_RESULT_IS((r), BitFieldResult::Ok)

#if SUB8_ENABLE_ENUM_FIELDS

// ------------------------------------------------------------
// Test enums
// ------------------------------------------------------------
enum class MsgType : uint8_t {
  Ping = 10,
  Pong = 11,
  Data = 12,
  Ack  = 13,
  Nack = 14,
};

// ------------------------------------------------------------
// Metadata / constexpr checks
// ------------------------------------------------------------
TEST_CASE("Enumeration: metadata (Ping..Nack => 5 values => 3 bits)", "[Enumeration][meta]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;

  STATIC_REQUIRE(T_SUT::ActualSize.bit_size() == 3);
  STATIC_REQUIRE(T_SUT::MinPossibleSize.bit_size() == 3);
  STATIC_REQUIRE(T_SUT::MaxPossibleSize.bit_size() == 3);

  REQUIRE(T_SUT::MinValue == MsgType::Ping);
  REQUIRE(T_SUT::MaxValue == MsgType::Nack);
}

// ------------------------------------------------------------
// Round-trip encode/decode (no padding)
// ------------------------------------------------------------
TEST_CASE("Enumeration: round-trip (padding=0)", "[Enumeration][roundtrip]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;

  auto [test_name, value, expected_bits, expected_bytes] =
      GENERATE(table<std::string, MsgType, size_t, std::vector<uint8_t>>({
          {"Ping", MsgType::Ping, 3, {0b00000000}},
          {"Pong", MsgType::Pong, 3, {0b00100000}},
          {"Data", MsgType::Data, 3, {0b01000000}},
          {"Ack",  MsgType::Ack,  3, {0b01100000}},
          {"Nack", MsgType::Nack, 3, {0b10000000}},
      }));

  DYNAMIC_SECTION(test_name) {
    T_SUT in{};
    T_SUT out{};

    REQUIRE(in.set_value(value) == BitFieldResult::Ok);

    BitFieldResult wr = BitFieldResult::ErrorUnidentifiedError;
    size_t total_bits = 0;
    auto storage = write_with_padding<128>(in, /*padded_bits=*/0, wr, total_bits);

    REQUIRE_OK(wr);
    REQUIRE(total_bits == expected_bits);
    REQUIRE(to_binary_string(storage) == to_binary_string(expected_bytes));

    // read back
    auto rr = read_with_padding<128>(out, /*padded_bits=*/0, storage, /*payload_bits=*/expected_bits);
    REQUIRE_OK(rr);
    REQUIRE(out.value() == value);
  }
}

// ------------------------------------------------------------
// Round-trip encode/decode (with padding)
// ------------------------------------------------------------
TEST_CASE("Enumeration: round-trip (padding=3)", "[Enumeration][roundtrip][padding]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;

  // After 3 padding bits, 3-bit code sits in bits 4..2 of the first byte.
  auto [test_name, value, padded_bits, payload_bits, expected_total_bits, expected_bytes] =
      GENERATE(table<std::string, MsgType, uint8_t, size_t, size_t, std::vector<uint8_t>>({
          {"Ping pad3", MsgType::Ping, 3, 3, 6, {0b00000000}},
          {"Pong pad3", MsgType::Pong, 3, 3, 6, {0b00000100}},
          {"Data pad3", MsgType::Data, 3, 3, 6, {0b00001000}},
          {"Ack  pad3", MsgType::Ack,  3, 3, 6, {0b00001100}},
          {"Nack pad3", MsgType::Nack, 3, 3, 6, {0b00010000}},
      }));

  DYNAMIC_SECTION(test_name) {
    T_SUT in{};
    T_SUT out{};

    REQUIRE(in.set_value(value) == BitFieldResult::Ok);

    BitFieldResult wr = BitFieldResult::ErrorUnidentifiedError;
    size_t total_bits = 0;
    auto storage = write_with_padding<128>(in, padded_bits, wr, total_bits);

    REQUIRE_OK(wr);
    REQUIRE(total_bits == expected_total_bits);
    REQUIRE(to_binary_string(storage) == to_binary_string(expected_bytes));

    auto rr = read_with_padding<128>(out, padded_bits, storage, payload_bits);
    REQUIRE_OK(rr);
    REQUIRE(out.value() == value);
  }
}

// ------------------------------------------------------------
// set_value range checking
// ------------------------------------------------------------
TEST_CASE("Enumeration: set_value rejects out-of-range (below/above)", "[Enumeration][set_value][overflow]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;

  T_SUT f{};
  REQUIRE(f.value() == MsgType::Ping); // default

  REQUIRE(f.set_value(static_cast<MsgType>(9))  == BitFieldResult::ErrorValueOverflow);
  REQUIRE(f.set_value(static_cast<MsgType>(15)) == BitFieldResult::ErrorValueOverflow);

  // Must remain in a valid, predictable state
  REQUIRE(f.value() == MsgType::Ping);
}

// ------------------------------------------------------------
// unchecked state should still be rejected by write_field
// ------------------------------------------------------------
TEST_CASE("Enumeration: set_value_unchecked can make invalid state; write_field must reject", "[Enumeration][unchecked][write]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;

  T_SUT f{};
  f.set_value_unchecked(static_cast<MsgType>(99)); // outside range

  BitFieldResult wr = BitFieldResult::Ok;
  size_t total_bits = 0;
  auto storage = write_with_padding<128>(f, /*padded_bits=*/0, wr, total_bits);

  (void)storage;
  (void)total_bits;

  REQUIRE_RESULT_IS(wr, BitFieldResult::ErrorValueOverflow);
}

// ------------------------------------------------------------
// Decoder must reject code >= RangeSize (craft invalid payload bits)
// ------------------------------------------------------------
TEST_CASE("Enumeration: decoder rejects out-of-range code (code == RangeSize)", "[Enumeration][read][overflow]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;
  T_SUT out{};

  // RangeSize=5 for Ping..Nack. Invalid code=5 => 0b101.
  BoundedBitWriter<128> bw;
  REQUIRE_OK(bw.template put_bits<uint32_t>(5u, 3u));

  auto storage = bw.storage();
  const auto rr = read_with_padding<128>(out, /*padded_bits=*/0, storage, /*payload_bits=*/3);
  REQUIRE_RESULT_IS(rr, BitFieldResult::ErrorValueOverflow);
}

// Same as above but with padding (catches cursor math)
TEST_CASE("Enumeration: decoder rejects out-of-range code with padding", "[Enumeration][read][overflow][padding]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;
  T_SUT out{};

  BoundedBitWriter<128> bw;
  REQUIRE_OK(bw.put_padding(3));
  REQUIRE_OK(bw.template put_bits<uint32_t>(5u, 3u)); // invalid code

  auto storage = bw.storage();
  const auto rr = read_with_padding<128>(out, /*padded_bits=*/3, storage, /*payload_bits=*/3);
  REQUIRE_RESULT_IS(rr, BitFieldResult::ErrorValueOverflow);
}

// ------------------------------------------------------------
// RangeSize==1 => ActualSize==1 and always round-trips
// ------------------------------------------------------------
TEST_CASE("Enumeration: RangeSize==1 uses 1 bit and round-trips", "[Enumeration][range1]") {
  enum class Solo : uint8_t { Only = 7 };
  using T_SUT = Enumeration<Solo, Solo::Only, Solo::Only>;

  STATIC_REQUIRE(T_SUT::ActualSize.bit_size() == 1);

  T_SUT in{};
  T_SUT out{};
  REQUIRE(in.value() == Solo::Only);

  BitFieldResult wr = BitFieldResult::ErrorUnidentifiedError;
  size_t total_bits = 0;
  auto storage = write_with_padding<128>(in, /*padded_bits=*/0, wr, total_bits);

  REQUIRE_OK(wr);
  REQUIRE(total_bits == 1);

  const auto rr = read_with_padding<128>(out, /*padded_bits=*/0, storage, /*payload_bits=*/1);
  REQUIRE_OK(rr);
  REQUIRE(out.value() == Solo::Only);
}

// ------------------------------------------------------------
// Writer buffer too small must be reported (catches write_field bug)
// ------------------------------------------------------------
TEST_CASE("Enumeration: write_field fails when insufficient buffer capacity", "[Enumeration][write][buffer]") {
  using T_SUT = Enumeration<MsgType, MsgType::Ping, MsgType::Nack>;
  T_SUT in{};
  REQUIRE(in.set_value(MsgType::Nack) == BitFieldResult::Ok);

  // With a 1-byte writer, padding 6 bits leaves only 2 bits free in that byte.
  // Enum needs 3 bits -> must fail.
  BitFieldResult wr = BitFieldResult::ErrorUnidentifiedError;
  size_t total_bits = 0;
  auto storage = write_with_padding<1>(in, /*padded_bits=*/6, wr, total_bits);

  (void)storage;
  (void)total_bits;

  REQUIRE_RESULT_IS(wr, BitFieldResult::ErrorInsufficentBufferSize);
}

#endif // SUB8_ENABLE_ENUM_FIELDS
