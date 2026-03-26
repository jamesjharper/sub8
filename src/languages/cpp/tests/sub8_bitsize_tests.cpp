#include <catch2/catch_all.hpp>

#include <cstdint>
#include <limits>
#include <type_traits>

#include "./../src/sub8_io.h"

using sub8::BitSize;

TEST_CASE("BitSize: default ctor is zero", "[BitSize]") {
  constexpr BitSize z{};
  STATIC_REQUIRE(z.byte_size_round_down() == 0);
  STATIC_REQUIRE(z.byte_size_round_up() == 0);
  STATIC_REQUIRE(z.bit_remainder() == 0);
  STATIC_REQUIRE(z.bit_size() == 0);
  STATIC_REQUIRE(z.lbit_size() == 0);
}

TEST_CASE("BitSize: from_bytes", "[BitSize]") {
  constexpr auto a = BitSize::from_bytes(0);
  STATIC_REQUIRE(a.byte_size_round_down() == 0);
  STATIC_REQUIRE(a.byte_size_round_up() == 0);
  STATIC_REQUIRE(a.bit_size() == 0);

  constexpr auto b = BitSize::from_bytes(5);
  STATIC_REQUIRE(b.byte_size_round_down() == 5);
  STATIC_REQUIRE(b.byte_size_round_up() == 5);
  STATIC_REQUIRE(b.bit_remainder() == 0);
  STATIC_REQUIRE(b.bit_size() == 40);
  STATIC_REQUIRE(b.lbit_size() == 40);
}

TEST_CASE("BitSize: from_bits basic", "[BitSize]") {
  constexpr auto a = BitSize::from_bits(0);
  STATIC_REQUIRE(a.byte_size_round_down() == 0);
  STATIC_REQUIRE(a.bit_remainder() == 0);
  STATIC_REQUIRE(a.byte_size_round_up() == 0);

  constexpr auto b = BitSize::from_bits(1);
  STATIC_REQUIRE(b.byte_size_round_down() == 0);
  STATIC_REQUIRE(b.bit_remainder() == 1);
  STATIC_REQUIRE(b.byte_size_round_up() == 1);
  STATIC_REQUIRE(b.bit_size() == 1);

  constexpr auto c = BitSize::from_bits(7);
  STATIC_REQUIRE(c.byte_size_round_down() == 0);
  STATIC_REQUIRE(c.bit_remainder() == 7);
  STATIC_REQUIRE(c.byte_size_round_up() == 1);
  STATIC_REQUIRE(c.bit_size() == 7);

  constexpr auto d = BitSize::from_bits(8);
  STATIC_REQUIRE(d.byte_size_round_down() == 1);
  STATIC_REQUIRE(d.bit_remainder() == 0);
  STATIC_REQUIRE(d.byte_size_round_up() == 1);
  STATIC_REQUIRE(d.bit_size() == 8);

  constexpr auto e = BitSize::from_bits(9);
  STATIC_REQUIRE(e.byte_size_round_down() == 1);
  STATIC_REQUIRE(e.bit_remainder() == 1);
  STATIC_REQUIRE(e.byte_size_round_up() == 2);
  STATIC_REQUIRE(e.bit_size() == 9);
}

TEST_CASE("BitSize: ctor normalizes bit remainder", "[BitSize]") {
  // Construct with remainder >= 8 and verify normalize carries into bytes.
  constexpr BitSize a{0, 8};
  STATIC_REQUIRE(a.byte_size_round_down() == 1);
  STATIC_REQUIRE(a.bit_remainder() == 0);
  STATIC_REQUIRE(a.bit_size() == 8);

  constexpr BitSize b{3, 17}; // +2 bytes +1 bit
  STATIC_REQUIRE(b.byte_size_round_down() == 5);
  STATIC_REQUIRE(b.bit_remainder() == 1);
  STATIC_REQUIRE(b.bit_size() == 41);
}

TEST_CASE("BitSize: comparisons", "[BitSize]") {
  constexpr auto a = BitSize::from_bits(0);
  constexpr auto b = BitSize::from_bits(1);
  constexpr auto c = BitSize::from_bits(8);
  constexpr auto d = BitSize::from_bits(9);

  STATIC_REQUIRE(a < b);
  STATIC_REQUIRE(b < c);
  STATIC_REQUIRE(c < d);

  STATIC_REQUIRE(a <= a);
  STATIC_REQUIRE(d >= c);
  STATIC_REQUIRE(!(c > d));
  STATIC_REQUIRE(a != b);
  STATIC_REQUIRE(BitSize::from_bits(9) == d);
}

TEST_CASE("BitSize: addition", "[BitSize]") {
  constexpr auto a = BitSize::from_bits(3);
  constexpr auto b = BitSize::from_bits(6);

  constexpr auto c = a + b; // 9 bits => 1 byte, 1 bit
  STATIC_REQUIRE(c.byte_size_round_down() == 1);
  STATIC_REQUIRE(c.bit_remainder() == 1);
  STATIC_REQUIRE(c.bit_size() == 9);

  constexpr auto d = BitSize::from_bits(7) + BitSize::from_bits(1);
  STATIC_REQUIRE(d.byte_size_round_down() == 1);
  STATIC_REQUIRE(d.bit_remainder() == 0);
  STATIC_REQUIRE(d.bit_size() == 8);
}

TEST_CASE("BitSize: subtraction without borrow", "[BitSize]") {
  constexpr auto a = BitSize::from_bits(25); // 3 bytes, 1 bit
  constexpr auto b = BitSize::from_bits(8);  // 1 byte
  constexpr auto c = a - b;                  // 17 bits => 2 bytes, 1 bit

  STATIC_REQUIRE(c.byte_size_round_down() == 2);
  STATIC_REQUIRE(c.bit_remainder() == 1);
  STATIC_REQUIRE(c.bit_size() == 17);
}

TEST_CASE("BitSize: subtraction with borrow", "[BitSize]") {
  constexpr auto a = BitSize{2, 0}; // 16 bits
  constexpr auto b = BitSize{1, 7}; // 15 bits
  constexpr auto c = a - b;         // 1 bit

  STATIC_REQUIRE(c.byte_size_round_down() == 0);
  STATIC_REQUIRE(c.bit_remainder() == 1);
  STATIC_REQUIRE(c.bit_size() == 1);

  // Another: 9 bits - 1 bit = 8 bits
  constexpr auto d = BitSize::from_bits(9) - BitSize::from_bits(1);
  STATIC_REQUIRE(d.byte_size_round_down() == 1);
  STATIC_REQUIRE(d.bit_remainder() == 0);
  STATIC_REQUIRE(d.bit_size() == 8);
}

TEST_CASE("BitSize: add_bits / sub_bits", "[BitSize]") {
  BitSize s{};
  s.add_bits(9);
  REQUIRE(s.bit_size() == 9);
  REQUIRE(s.byte_size_round_down() == 1);
  REQUIRE(s.byte_size_round_up() == 2);
  REQUIRE(s.bit_remainder() == 1);

  s.sub_bits(1);
  REQUIRE(s.bit_size() == 8);
  REQUIRE(s.byte_size_round_down() == 1);
  REQUIRE(s.byte_size_round_up() == 1);
  REQUIRE(s.bit_remainder() == 0);
}

TEST_CASE("BitSize: multiplication", "[BitSize]") {
  // 9 bits * 3 = 27 bits => 3 bytes, 3 bits
  constexpr auto a = BitSize::from_bits(9);
  constexpr auto b = a * 3;
  STATIC_REQUIRE(b.byte_size_round_down() == 3);
  STATIC_REQUIRE(b.bit_remainder() == 3);
  STATIC_REQUIRE(b.bit_size() == 27);

  // 1 byte * 5 = 5 bytes
  constexpr auto c = BitSize::from_bytes(1) * 5;
  STATIC_REQUIRE(c.byte_size_round_down() == 5);
  STATIC_REQUIRE(c.bit_remainder() == 0);
  STATIC_REQUIRE(c.bit_size() == 40);

  // (3 bytes, 7 bits) * 2 => (7 bytes, 6 bits)
  constexpr BitSize d{3, 7}; // 31 bits
  constexpr auto e = d * 2;  // 62 bits => 7 bytes, 6 bits
  STATIC_REQUIRE(e.byte_size_round_down() == 7);
  STATIC_REQUIRE(e.bit_remainder() == 6);
  STATIC_REQUIRE(e.bit_size() == 62);
}

TEST_CASE("BitSize: lbit_size matches bit_size for safe ranges", "[BitSize]") {
  // This test is mostly a sanity check for consistency.
  for (uint64_t bits : {0ULL, 1ULL, 7ULL, 8ULL, 9ULL, 63ULL, 64ULL, 127ULL, 128ULL, 1024ULL}) {
    auto s = BitSize::from_bits(bits);
    REQUIRE(s.lbit_size() == bits);

    // bit_size() is size_t and may overflow for huge inputs; these are safe.
    REQUIRE(static_cast<uint64_t>(s.bit_size()) == bits);
  }
}
