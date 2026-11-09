#include <catch2/catch_all.hpp>
#include <cmath>
#include <cstdint>
#include "../src/sub8.h"

using namespace sub8;

TEST_CASE("AI Audit: PrimitiveVariant defaults to null and counts its tag", "[ai-audit][variant]") {
  PrimitiveVariant value{};
  REQUIRE(value.is_null());
  REQUIRE(value.actual_size() == VariantValueTypeField::ActualSize);
}

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
TEST_CASE("AI Audit: packed unsigned 32-bit variants round trip", "[ai-audit][variant]") {
  for (const auto tag : {VariantValueType::U32P8, VariantValueType::U32P16}) {
    PrimitiveVariant input{};
    constexpr uint32_t number = 0x12345678u;
    if (tag == VariantValueType::U32P8) {
      REQUIRE(input.set_uint32_p8(number) == BitFieldResult::Ok);
    } else {
      REQUIRE(input.set_uint32_p16(number) == BitFieldResult::Ok);
    }
    BoundedBitWriter<32> writer;
    REQUIRE(write_field(writer, input) == BitFieldResult::Ok);
    REQUIRE(writer.size() == input.actual_size());
    BoundedBitReader<32> reader(writer.storage(), writer.size());
    PrimitiveVariant output{};
    REQUIRE(read_field(reader, output) == BitFieldResult::Ok);
    REQUIRE(output.type == tag);
    REQUIRE(output.get_uint32() != nullptr);
    REQUIRE(*output.get_uint32() == number);
  }
}
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
TEST_CASE("AI Audit: packed signed 32-bit variants round trip", "[ai-audit][variant]") {
  for (const auto tag : {VariantValueType::I32P8, VariantValueType::I32P16}) {
    PrimitiveVariant input{};
    constexpr int32_t number = -1234567;
    if (tag == VariantValueType::I32P8) {
      REQUIRE(input.set_int32_p8(number) == BitFieldResult::Ok);
    } else {
      REQUIRE(input.set_int32_p16(number) == BitFieldResult::Ok);
    }
    BoundedBitWriter<32> writer;
    REQUIRE(write_field(writer, input) == BitFieldResult::Ok);
    REQUIRE(writer.size() == input.actual_size());
    BoundedBitReader<32> reader(writer.storage(), writer.size());
    PrimitiveVariant output{};
    REQUIRE(read_field(reader, output) == BitFieldResult::Ok);
    REQUIRE(output.type == tag);
    REQUIRE(output.get_int32() != nullptr);
    REQUIRE(*output.get_int32() == number);
  }
}
#endif

#if SUB8_ENABLE_FLOAT64
TEST_CASE("AI Audit: float64 setter retains double precision", "[ai-audit][variant]") {
  PrimitiveVariant value{};
  const double precise = 1.0 + 1e-12;
  REQUIRE(value.set_float64(precise) == BitFieldResult::Ok);
  REQUIRE(value.get_float64() != nullptr);
  REQUIRE(*value.get_float64() == precise);
}
#endif
