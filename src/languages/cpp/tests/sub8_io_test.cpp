// tests/sub8_io_test.cpp
#include <catch2/catch_all.hpp>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "./../src/sub8.h"

using namespace sub8;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static void REQUIRE_BYTES_EQ(const uint8_t *got, const uint8_t *expected, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    INFO("byte[" << i << "] got=" << +got[i] << " expected=" << +expected[i]);
    REQUIRE(got[i] == expected[i]);
  }
}

#if SUB8_ENABLE_VIEW_BUF
static sub8::BitReader make_reader_out_of_scope(uint8_t *data, BitSize size) noexcept {
  return sub8::make_reader(data, size);
}
#endif

// ------------------------------------------------------------
// packing::*
// ------------------------------------------------------------

TEST_CASE("packing: zigzag encode/decode round-trips for signed ints", "[sub8_io][packing][zigzag]") {
  using sub8::packing::zigzag_encode;
  using sub8::packing::zigzag_decode;

  const int32_t vals[] = {0,
                          1,
                          -1,
                          2,
                          -2,
                          123456,
                          -123456,
                          (std::numeric_limits<int32_t>::max)(),
                          (std::numeric_limits<int32_t>::min)() + 1};

  for (auto v : vals) {
    const auto enc = zigzag_encode<int32_t>(v);
    const auto dec = zigzag_decode<int32_t>(enc);
    INFO("v=" << v << " enc=" << enc << " dec=" << dec);
    REQUIRE(dec == v);
  }
}

TEST_CASE("packing: pack/unpack handles signed, unsigned, and enums", "[sub8_io][packing][pack]") {
  using sub8::packing::pack;
  using sub8::packing::unpack;

  // Signed -> zigzag
  {
    int16_t v = -123;
    auto code = pack(v);
    auto rt = unpack<int16_t>(code);
    REQUIRE(rt == v);
    static_assert(std::is_unsigned_v<decltype(code)>);
  }

  // Unsigned -> identity-ish
  {
    uint16_t v = 0xBEEF;
    auto code = pack(v);
    auto rt = unpack<uint16_t>(code);
    REQUIRE(rt == v);
  }

  // Enum underlying
  enum class E : int8_t { A = -1, B = 0, C = 1 };
  {
    auto code = pack(E::A);
    auto rt = unpack<E>(code);
    REQUIRE(rt == E::A);
  }
}

// ------------------------------------------------------------
// BitSize
// ------------------------------------------------------------

TEST_CASE("BitSize: normalization and accessors", "[sub8_io][BitSize]") {
  // 1 byte + 9 bits => normalizes to 2 bytes + 1 bit
  constexpr BitSize s{1, 9};
  STATIC_REQUIRE(s.byte_size_round_down() == 2);
  STATIC_REQUIRE(s.bit_remainder() == 1);
  STATIC_REQUIRE(s.byte_size_round_up() == 3);
  STATIC_REQUIRE(s.bit_size() == 17);
  STATIC_REQUIRE(s.lbit_size() == 17ULL);
}

TEST_CASE("BitSize: arithmetic + comparisons", "[sub8_io][BitSize]") {
  constexpr BitSize a = BitSize::from_bits(9);
  constexpr BitSize b = BitSize::from_bits(15);
  constexpr BitSize c = a + b; // 24 bits
  STATIC_REQUIRE(c == BitSize::from_bytes(3));

  constexpr BitSize d = BitSize::from_bits(24) - BitSize::from_bits(1); // 23 bits
  STATIC_REQUIRE(d.byte_size_round_down() == 2);
  STATIC_REQUIRE(d.bit_remainder() == 7);

  constexpr BitSize e = BitSize::from_bits(5) * 3;
  STATIC_REQUIRE(e == BitSize::from_bits(15));

  STATIC_REQUIRE(BitSize::from_bits(7) < BitSize::from_bits(8));
  STATIC_REQUIRE(BitSize::from_bits(8) <= BitSize::from_bits(8));
  STATIC_REQUIRE(BitSize::from_bits(9) > BitSize::from_bits(8));
}

// ------------------------------------------------------------
// BoundedByteBuffer + BasicBitWriter/Reader (bounded)
// ------------------------------------------------------------

#if SUB8_ENABLE_BOUNDED_BUF

TEST_CASE("BoundedByteBuffer: initializer_list overflow sets ok=false", "[sub8_io][BoundedByteBuffer]") {
  sub8::BoundedByteBuffer<2> b{0xAA, 0xBB, 0xCC};
  REQUIRE(b.size() == 2);
  REQUIRE_FALSE(b.ok());
  const uint8_t expected[2] = {0xAA, 0xBB};
  REQUIRE_BYTES_EQ(b.data(), expected, 2);
}

TEST_CASE("BoundedByteBuffer: push_back respects capacity and returns errors", "[sub8_io][BoundedByteBuffer]") {
  sub8::BoundedByteBuffer<1> b;
  REQUIRE(b.ok());
  REQUIRE(b.push_back(0x11) == BitFieldResult::Ok);
  REQUIRE(b.size() == 1);
  REQUIRE(b.push_back(0x22) == BitFieldResult::ErrorInsufficentBufferSize);
  REQUIRE_FALSE(b.ok());
}

TEST_CASE("BoundedBitWriter: put_bits packs MSB-first across byte boundaries", "[sub8_io][BitWriter][Bounded]") {
  sub8::BoundedBitWriter<8> bw;

  // 3 bits: 101 => 10100000
  REQUIRE(bw.put_bits(uint8_t{0b101}, 3) == BitFieldResult::Ok);
  // 5 bits: 00111 => 10100111 = 0xA7
  REQUIRE(bw.put_bits(uint8_t{0b00111}, 5) == BitFieldResult::Ok);

  REQUIRE(bw.size() == BitSize::from_bytes(1));
  REQUIRE(bw.storage().size() == 1);

  const uint8_t expected[1] = {0xA7};
  REQUIRE_BYTES_EQ(bw.storage().data(), expected, 1);
}

TEST_CASE("BoundedBitWriter: multi-byte write and size() with sub-byte remainder", "[sub8_io][BitWriter][Bounded]") {
  sub8::BoundedBitWriter<8> bw;

  // 12 bits: 0xDE then 0xA high nibble => [0xDE, 0xA0]
  REQUIRE(bw.put_bits(uint8_t{0xDE}, 8) == BitFieldResult::Ok);
  REQUIRE(bw.put_bits(uint8_t{0x0A}, 4) == BitFieldResult::Ok);

  REQUIRE(bw.size() == BitSize(1, 4));
  REQUIRE(bw.storage().size() == 2);

  const uint8_t expected[2] = {0xDE, 0xA0};
  REQUIRE_BYTES_EQ(bw.storage().data(), expected, 2);
}

TEST_CASE("BoundedBitWriter: put_padding advances and writes zeros", "[sub8_io][BitWriter][padding]") {
  sub8::BoundedBitWriter<8> bw;

  REQUIRE(bw.put_bits(uint8_t{0xF}, 4) == BitFieldResult::Ok); // 1111xxxx => 0xF0
  REQUIRE(bw.put_padding(4) == BitFieldResult::Ok);            // fill low nibble with 0

  REQUIRE(bw.size() == BitSize::from_bytes(1));
  const uint8_t expected[1] = {0xF0};
  REQUIRE_BYTES_EQ(bw.storage().data(), expected, 1);
}

TEST_CASE("BoundedBitReader: round-trip write then read equals original", "[sub8_io][BitReader][Bounded][roundtrip]") {
  sub8::BoundedBitWriter<16> bw;

  REQUIRE(bw.put_bits(uint8_t{0b101}, 3) == BitFieldResult::Ok);
  REQUIRE(bw.put_bits(uint16_t{0x2AA}, 10) == BitFieldResult::Ok);
  REQUIRE(bw.put_bits(uint8_t{0b11}, 2) == BitFieldResult::Ok);
  // total = 15 bits

  auto &buf = bw.storage();
  sub8::BoundedBitReader<16> br(buf, BitSize::from_bits(15));

  uint8_t a = 0;
  uint16_t b = 0;
  uint8_t c = 0;

  REQUIRE(br.get_bits(a, 3) == BitFieldResult::Ok);
  REQUIRE(br.get_bits(b, 10) == BitFieldResult::Ok);
  REQUIRE(br.get_bits(c, 2) == BitFieldResult::Ok);

  REQUIRE(a == 0b101);
  REQUIRE(b == 0x2AA);
  REQUIRE(c == 0b11);

  uint8_t extra = 0;
  REQUIRE(br.get_bits(extra, 1) == BitFieldResult::ErrorExpectedMoreBits);
}

TEST_CASE("BoundedBitReader: has() boundary behavior with sub-byte remainder", "[sub8_io][BitReader][Bounded][has]") {
  // 13 bits total => 1 byte + 5 bits
  sub8::BoundedByteBuffer<2> buf{0xFF, 0xF8};
  sub8::BoundedBitReader<2> br(buf, BitSize(1, 5));

  REQUIRE(br.has(13));
  REQUIRE_FALSE(br.has(14));

  uint16_t v = 0;
  REQUIRE(br.get_bits(v, 13) == BitFieldResult::Ok);
  REQUIRE_FALSE(br.has(1));
}

TEST_CASE("BoundedBitReader: get_bits rejects too-many-bits requests", "[sub8_io][BitReader][errors]") {
  sub8::BoundedByteBuffer<1> buf{0xFF};
  sub8::BoundedBitReader<1> br(buf, BitSize::from_bits(8));

  uint64_t out = 0;
  REQUIRE(br.get_bits(out, 65) == BitFieldResult::ErrorTooManyBits);
}

TEST_CASE("BoundedBitWriter: put_bits rejects too-many-bits requests", "[sub8_io][BitWriter][errors]") {
  sub8::BoundedBitWriter<8> bw;
  REQUIRE(bw.put_bits(uint64_t{0}, 65) == BitFieldResult::ErrorTooManyBits);
}

#endif // SUB8_ENABLE_BOUNDED_BUF

// ------------------------------------------------------------
// ByteBufferView + make_reader (view) + compile-time writer notes
// ------------------------------------------------------------

#if SUB8_ENABLE_VIEW_BUF

TEST_CASE("ByteBufferView: push_back respects capacity and ok flag", "[sub8_io][ByteBufferView]") {
  uint8_t backing[2] = {0, 0};
  sub8::ByteBufferView v(backing, 0, 2);

  REQUIRE(v.ok());
  REQUIRE(v.push_back(0x11) == BitFieldResult::Ok);
  REQUIRE(v.push_back(0x22) == BitFieldResult::Ok);
  REQUIRE(v.size() == 2);
  REQUIRE(v.push_back(0x33) == BitFieldResult::ErrorInsufficentBufferSize);
  REQUIRE_FALSE(v.ok());
}

TEST_CASE("make_reader: returned reader owns its ByteBufferView (safe after factory scope)", "[sub8_io][make_reader][View][owning]") {
  uint8_t backing[2] = {0xAB, 0xC0};

  auto br = make_reader_out_of_scope(backing, BitSize(1, 4));

  uint8_t a = 0;
  uint8_t b = 0;

  REQUIRE(br.get_bits(a, 8) == BitFieldResult::Ok);
  REQUIRE(br.get_bits(b, 4) == BitFieldResult::Ok);
  REQUIRE(a == 0xAB);
  REQUIRE(b == 0x0C);
}

TEST_CASE("BitReader<ByteBufferView>: move-construct preserves readable state", "[sub8_io][BitReader][View][move]") {
  uint8_t backing[2] = {0xDE, 0xA0};
  auto br1 = make_reader(backing, BitSize(1, 4));

  uint8_t x = 0;
  REQUIRE(br1.get_bits(x, 4) == BitFieldResult::Ok);
  REQUIRE(x == 0x0D);

  sub8::BitReader br2(std::move(br1));

  uint8_t y = 0;
  REQUIRE(br2.get_bits(y, 4) == BitFieldResult::Ok);
  REQUIRE(y == 0x0E);

  uint8_t z = 0;
  REQUIRE(br2.get_bits(z, 4) == BitFieldResult::Ok);
  REQUIRE(z == 0x0A);
}

TEST_CASE("BitWriter alias: not constructible from ByteBufferView (documents current limitation)", "[sub8_io][BitWriter][View][constructibility]") {
  // Current API: BasicBitWriter<Storage> has no ctor taking Storage, so BitWriter can't be built from a view.
  STATIC_REQUIRE_FALSE(std::is_constructible_v<sub8::BitWriter, sub8::ByteBufferView>);
  STATIC_REQUIRE(std::is_default_constructible_v<sub8::BitWriter>);
}

#endif // SUB8_ENABLE_VIEW_BUF

// ------------------------------------------------------------
// UnboundedByteBuffer + init_unbounded_writer_for
// ------------------------------------------------------------

#if SUB8_ENABLE_UNBOUNDED_BUF

namespace {
struct FakeMsgForReserve {
  static constexpr sub8::BitSize MinPossibleSize = sub8::BitSize::from_bits(8);   // 1 byte
  static constexpr sub8::BitSize MaxPossibleSize = sub8::BitSize::from_bits(80);  // 10 bytes
};
} // namespace

TEST_CASE("UnboundedByteBuffer: reserve/push_back return Ok in normal conditions", "[sub8_io][UnboundedByteBuffer]") {
  sub8::UnboundedByteBuffer b;
  REQUIRE(b.reserve(64) == BitFieldResult::Ok);
  REQUIRE(b.push_back(0x11) == BitFieldResult::Ok);
  REQUIRE(b.size() == 1);
  REQUIRE(b.data()[0] == 0x11);
}

TEST_CASE("recommended_unbounded_buffer_byte_size: clamps percentage and interpolates", "[sub8_io][UnboundedByteBuffer][reserve]") {
  REQUIRE(sub8::recommended_unbounded_buffer_byte_size<FakeMsgForReserve>(0) == 1);
  REQUIRE(sub8::recommended_unbounded_buffer_byte_size<FakeMsgForReserve>(100) == 10);
  REQUIRE(sub8::recommended_unbounded_buffer_byte_size<FakeMsgForReserve>(50) == 1 + (9 * 50) / 100);
  REQUIRE(sub8::recommended_unbounded_buffer_byte_size<FakeMsgForReserve>(200) == 10);
}

TEST_CASE("init_unbounded_writer_for clears and reserves recommended size", "[sub8_io][init_unbounded_writer_for]") {
  sub8::UnboundedBitWriter bw;
  REQUIRE(bw.put_bits(uint8_t{0xAA}, 8) == BitFieldResult::Ok);
  REQUIRE(bw.storage().size() == 1);

  auto r = sub8::init_unbounded_writer_for<FakeMsgForReserve>(bw, 50);
  REQUIRE(r == BitFieldResult::Ok);

  REQUIRE(bw.storage().size() == 0);

  const size_t want = sub8::recommended_unbounded_buffer_byte_size<FakeMsgForReserve>(50);
  REQUIRE(bw.storage().capacity() >= want);
}

#endif // SUB8_ENABLE_UNBOUNDED_BUF
