#pragma once

// Enable: Variant Fields
// Support for predefined set of variant data types on the wire
#ifndef SUB8_ENABLE_VARIANT
#define SUB8_ENABLE_VARIANT 1
#endif

#if SUB8_ENABLE_VARIANT

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_enums.h"
#include "sub8_errors.h"
#include "sub8_floats.h"
#include "sub8_io.h"
#include "sub8_macros.h"
#include "sub8_primitives.h"
#endif 

namespace sub8 {

// PrimitiveVariant (FixedLength, 5 bits)
enum class VariantValueType : uint8_t {

  NullValue = 0,
  Boolean = 1,
  U4 = 2,
  U8 = 3,
  I8 = 4,
  U16 = 5,
  I16 = 6,
  U32 = 7,
  I32 = 8,
  U64 = 9,
  I64 = 10,

  U32P8 = 11,
  U32P16 = 12,

  U64P32 = 13,

  I32P8 = 14,
  I32P16 = 15,

  I64P32 = 16,

  Float16ValueField = 17,
  Float32ValueField = 18,
  Float64ValueField = 19,
};

using VariantValueTypeField = Enumeration<VariantValueType, VariantValueType::NullValue, VariantValueType::Float64ValueField>;

struct PrimitiveVariant {
  VariantValueType type;

  union VariantValue {
    NullValue null_v;
#if SUB8_ENABLE_BOOL
    Boolean bool_v;
#endif
#if SUB8_ENABLE_UINT4
    U4 u4_v;
#endif
#if SUB8_ENABLE_UINT8
    U8 u8_v;
#endif
#if SUB8_ENABLE_UINT16
    U16 u16_v;
#endif
#if SUB8_ENABLE_UINT32
    U32 u32_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    U32P8 u32_p8_v;
    U32P16 u32_p16_v;
#endif
#if SUB8_ENABLE_UINT64
    U64 u64_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    U64P32 u64_p32_v;
#endif
#if SUB8_ENABLE_INT8
    I8 i8_v;
#endif
#if SUB8_ENABLE_INT16
    I16 i16_v;
#endif
#if SUB8_ENABLE_INT32
    I32 i32_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    I32P8 i32_p8_v;
    I32P16 i32_p16_v;
#endif
#if SUB8_ENABLE_INT64
    I64 i64_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    I64P32 i64_p32_v;
#endif
#if SUB8_ENABLE_FLOAT16
    Float16ValueField f16_v;
#endif
#if SUB8_ENABLE_FLOAT32
    Float32ValueField f32_v;
#endif
#if SUB8_ENABLE_FLOAT64
    Float64ValueField f64_v;
#endif
    VariantValue() : null_v(NullValue()) {}
  } variant;

  using Type = PrimitiveVariant;
  using InitType = PrimitiveVariant;
  using ValueType = PrimitiveVariant;

  static constexpr BitSize MaxPossibleSize = VariantValueTypeField::MaxPossibleSize + BitSize::from_bits(65);
  static constexpr BitSize MinPossibleSize = VariantValueTypeField::MinPossibleSize; // Null variant type

  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }

  BitSize actual_size() const noexcept {
    switch (type) {
    case VariantValueType::NullValue: {
      return NullValue::ActualSize;
    }

#if SUB8_ENABLE_BOOL
    case VariantValueType::Boolean: {
      return Boolean::ActualSize;
    }
#endif

#if SUB8_ENABLE_UINT4
    case VariantValueType::U4: {
      return U4::ActualSize;
    }
#endif

#if SUB8_ENABLE_UINT8
    case VariantValueType::U8: {
      return U8::ActualSize;
    }
#endif

#if SUB8_ENABLE_INT8
    case VariantValueType::I8: {
      return I8::ActualSize;
    }
#endif

#if SUB8_ENABLE_UINT16
    case VariantValueType::U16: {
      return U16::ActualSize;
    }
#endif

#if SUB8_ENABLE_INT16
    case VariantValueType::I16: {
      return I16::ActualSize;
    }
#endif

#if SUB8_ENABLE_UINT32
    case VariantValueType::U32: {
      return U32::ActualSize;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    case VariantValueType::U32P8: {
      return variant.u32_p8_v.actual_size();
    }
    case VariantValueType::U32P16: {
      return variant.u32_p16_v.actual_size();
    }
#endif

#if SUB8_ENABLE_INT32
    case VariantValueType::I32: {
      return I32::ActualSize;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    case VariantValueType::I32P8: {
      return variant.i32_p8_v.actual_size();
    }
    case VariantValueType::I32P16: {
      return variant.i32_p16_v.actual_size();
    }
#endif

#if SUB8_ENABLE_UINT64
    case VariantValueType::U64: {
      return U64::ActualSize;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    case VariantValueType::U64P32: {
      return variant.u64_p32_v.actual_size();
    }
#endif

#if SUB8_ENABLE_INT64
    case VariantValueType::I64: {
      return I64::ActualSize;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    case VariantValueType::I64P32: {
      return variant.i64_p32_v.actual_size();
    }
#endif

#if SUB8_ENABLE_FLOAT16
    case VariantValueType::Float16ValueField: {
      return Float16ValueField::ActualSize;
    }
#endif

#if SUB8_ENABLE_FLOAT32
    case VariantValueType::Float32ValueField: {
      return Float32ValueField::ActualSize;
    }
#endif

#if SUB8_ENABLE_FLOAT64
    case VariantValueType::Float64ValueField: {
      return Float64ValueField::ActualSize;
    }
#endif

    default:
      return NullValue::ActualSize;
    }
  }

  const PrimitiveVariant &value() const noexcept { return *this; }

  bool operator==(const PrimitiveVariant &o) const noexcept {
    if (type != o.type) {
      return false;
    }

    switch (type) {
    case VariantValueType::NullValue: {
      return true;
    }

#if SUB8_ENABLE_BOOL
    case VariantValueType::Boolean: {
      return variant.bool_v == o.variant.bool_v;
    }
#endif

#if SUB8_ENABLE_UINT4
    case VariantValueType::U4: {
      return variant.u4_v == o.variant.u4_v;
    }
#endif

#if SUB8_ENABLE_UINT8
    case VariantValueType::U8: {
      return variant.u8_v == o.variant.u8_v;
    }
#endif

#if SUB8_ENABLE_INT8
    case VariantValueType::I8: {
      return variant.i8_v == o.variant.i8_v;
    }
#endif

#if SUB8_ENABLE_UINT16
    case VariantValueType::U16: {
      return variant.u16_v == o.variant.u16_v;
    }
#endif

#if SUB8_ENABLE_INT16
    case VariantValueType::I16: {
      return variant.i16_v == o.variant.i16_v;
    }
#endif

#if SUB8_ENABLE_UINT32
    case VariantValueType::U32: {
      return variant.u32_v == o.variant.u32_v;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    case VariantValueType::U32P8: {
      return variant.u32_p8_v == o.variant.u32_p8_v;
    }
    case VariantValueType::U32P16: {
      return variant.u32_p16_v == o.variant.u32_p16_v;
    }
#endif

#if SUB8_ENABLE_INT32
    case VariantValueType::I32: {
      return variant.i32_v == o.variant.i32_v;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    case VariantValueType::I32P8: {
      return variant.i32_p8_v == o.variant.i32_p8_v;
    }
    case VariantValueType::I32P16: {
      return variant.i32_p16_v == o.variant.i32_p16_v;
    }
#endif

#if SUB8_ENABLE_UINT64
    case VariantValueType::U64: {
      return variant.u64_v == o.variant.u64_v;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    case VariantValueType::U64P32: {
      return variant.u64_p32_v == o.variant.u64_p32_v;
    }
#endif

#if SUB8_ENABLE_INT64
    case VariantValueType::I64: {
      return variant.i64_v == o.variant.i64_v;
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    case VariantValueType::I64P32: {
      return variant.i64_p32_v == o.variant.i64_p32_v;
    }
#endif

#if SUB8_ENABLE_FLOAT16
    case VariantValueType::Float16ValueField: {
      return variant.f16_v == o.variant.f16_v;
    }
#endif

#if SUB8_ENABLE_FLOAT32
    case VariantValueType::Float32ValueField: {
      return variant.f32_v == o.variant.f32_v;
    }
#endif

#if SUB8_ENABLE_FLOAT64
    case VariantValueType::Float64ValueField: {
      return variant.f64_v == o.variant.f64_v;
    }
#endif

    default:
      return false;
    }
  }

  bool operator!=(const PrimitiveVariant &o) const noexcept { return !(*this == o); }

  BitFieldResult set_value(PrimitiveVariant v) noexcept {
    type = v.type;
    variant = v.variant;
    return BitFieldResult::Ok;
  }

  bool is_null() { return type == VariantValueType::NullValue; }

#if SUB8_ENABLE_BOOL
  const bool *get_bool() {
    if (type == VariantValueType::Boolean) {
      return &variant.bool_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_bool(bool b) {
    type = VariantValueType::Boolean;
    variant.bool_v = b;
    return BitFieldResult::Ok;
  }

  bool is_bool() { return type == VariantValueType::Boolean; }
#endif

#if SUB8_ENABLE_UINT4
  const uint8_t *get_uint4() {
    if (type == VariantValueType::U4) {
      return &variant.u4_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_uint4(size_t v) {
    auto r = variant.u4_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U4;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_uint4() { return type == VariantValueType::U4; }
#endif

#if SUB8_ENABLE_UINT8
  const uint8_t *get_uint8() {
    if (type == VariantValueType::U8) {
      return &variant.u8_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_uint8(uint8_t v) {
    auto r = variant.u8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U8;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_uint8() { return type == VariantValueType::U8; }
#endif

#if SUB8_ENABLE_UINT16
  inline const uint16_t *get_uint16() {
    if (type == VariantValueType::U16) {
      return &variant.u16_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_uint16(uint16_t v) {
    auto r = variant.u16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U16;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_uint16() { return type == VariantValueType::U16; }
#endif

#if SUB8_ENABLE_UINT32 || SUB8_ENABLE_VARIABLE_LENGTH_UINT32

  inline const uint32_t *get_uint32() {
#if SUB8_ENABLE_UINT32
    if (type == VariantValueType::U32) {
      return &variant.u32_v.value();
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    if (type == VariantValueType::U32P8) {
      return &variant.u32_p8_v.value();
    }
    if (type == VariantValueType::U32P16) {
      return &variant.u32_p16_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_UINT32
  BitFieldResult set_uint32(uint32_t v) {
    auto r = variant.u32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U32;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
  BitFieldResult set_uint32_p8(uint32_t v) {
    auto r = variant.u32_p8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U32P8;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  BitFieldResult set_uint32_p16(uint32_t v) {
    auto r = variant.u32_p16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U32P16;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }
#endif

  bool is_uint32() {
    switch (type) {
    case VariantValueType::U32:
    case VariantValueType::U32P8:
    case VariantValueType::U32P16:
      return true;
    default:
      return false;
    }
  }
#endif

#if SUB8_ENABLE_UINT64 || SUB8_ENABLE_VARIABLE_LENGTH_UINT64
  inline const uint64_t *get_uint64() {
#if SUB8_ENABLE_UINT64
    if (type == VariantValueType::U64) {
      return &variant.u64_v.value();
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    if (type == VariantValueType::U64P32) {
      return &variant.u64_p32_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_UINT64
  BitFieldResult set_uint64(uint64_t v) {
    auto r = variant.u64_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U64;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
  BitFieldResult set_uint64_p32(uint64_t v) {
    auto r = variant.u64_p32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::U64P32;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }
#endif

  bool is_uint64() {
    switch (type) {
    case VariantValueType::U64:
    case VariantValueType::U64P32:
      return true;
    default:
      return false;
    }
  }
#endif

#if SUB8_ENABLE_INT8
  const int8_t *get_int8() {
    if (type == VariantValueType::I8) {
      return &variant.i8_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_int8(int8_t v) {
    auto r = variant.i8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I8;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_int8() { return type == VariantValueType::I8; }
#endif

#if SUB8_ENABLE_INT16
  const int16_t *get_int16() {
    if (type == VariantValueType::I16) {
      return &variant.i16_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_int16(int16_t v) {
    auto r = variant.i16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I16;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_int16() { return type == VariantValueType::I16; }

#endif

#if SUB8_ENABLE_INT32 || SUB8_ENABLE_VARIABLE_LENGTH_INT32
  const int32_t *get_int32() {
#if SUB8_ENABLE_INT32
    if (type == VariantValueType::I32) {
      return &variant.i32_v.value();
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    if (type == VariantValueType::I32P8) {
      return &variant.i32_p8_v.value();
    }
    if (type == VariantValueType::I32P16) {
      return &variant.i32_p16_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_INT32

  BitFieldResult set_int32(int32_t v) {
    auto r = variant.i32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I32;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32

  BitFieldResult set_int32_p8(int32_t v) {
    auto r = variant.i32_p8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I32P8;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  BitFieldResult set_int32_p16(int32_t v) {
    auto r = variant.i32_p16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I32P16;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

#endif

  bool is_int32() {
    switch (type) {
    case VariantValueType::I32:
    case VariantValueType::I32P8:
    case VariantValueType::I32P16:
      return true;
    default:
      return false;
    }
  }
#endif

#if SUB8_ENABLE_INT64 || SUB8_ENABLE_VARIABLE_LENGTH_INT64

  const int64_t *get_int64() const {
#if SUB8_ENABLE_INT64
    if (type == VariantValueType::I64) {
      return &variant.i64_v.value();
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    if (type == VariantValueType::I64P32) {
      return &variant.i64_p32_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_INT64
  BitFieldResult set_int64(int64_t v) {
    auto r = variant.i64_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I64;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
  BitFieldResult set_int64_p32(int64_t v) {
    auto r = variant.i64_p32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::I64P32;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }
#endif

  bool is_int64() const {
    switch (type) {
    case VariantValueType::I64:
    case VariantValueType::I64P32:
      return true;
    default:
      return false;
    }
  }

#endif

#if SUB8_ENABLE_FLOAT16
  const float *get_float16() const {
    if (type == VariantValueType::Float16ValueField) {
      return &variant.f16_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_float16(float v) {
    auto r = variant.f16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Float16ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_float16() const { return type == VariantValueType::Float16ValueField; }

#endif

#if SUB8_ENABLE_FLOAT32
  const float *get_float32() const {
    if (type == VariantValueType::Float32ValueField) {
      return &variant.f32_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_float32(float v) {
    auto r = variant.f32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Float32ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_float32() const { return type == VariantValueType::Float32ValueField; }

#endif

#if SUB8_ENABLE_FLOAT64
  const double *get_float64() const {
    if (type == VariantValueType::Float64ValueField) {
      return &variant.f64_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_float64(float v) {
    auto r = variant.f64_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Float64ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValue;
    return r;
  }

  bool is_float64() const { return type == VariantValueType::Float64ValueField; }
#endif

  template <typename T> inline const T *value() {
    // Null marker
    if constexpr (std::is_same_v<T, std::nullptr_t>) {
      return nullptr;
    }
// Bool
#if SUB8_ENABLE_BOOL
    else if constexpr (std::is_same_v<T, bool>) {
      return get_bool();
    }
#endif

// Unsigned 8-bit: try u8, then u4 (both represented as uint8_t)
#if SUB8_ENABLE_UINT8 || SUB8_ENABLE_UINT4
    else if constexpr (std::is_same_v<T, uint8_t>) {
#if SUB8_ENABLE_UINT8
      if (auto v = get_uint8()) {
        return v;
      }
#endif
#if SUB8_ENABLE_UINT4
      if (auto v = get_uint4()) {
        return v;
      }
#endif
      return nullptr;
    }
#endif

// Signed 8-bit
#if SUB8_ENABLE_INT8
    else if constexpr (std::is_same_v<T, int8_t>) {
      return get_int8();
    }
#endif

// Unsigned 16-bit
#if SUB8_ENABLE_UINT16
    else if constexpr (std::is_same_v<T, uint16_t>) {
      return get_uint16();
    }
#endif

// Signed 16-bit
#if SUB8_ENABLE_INT16
    else if constexpr (std::is_same_v<T, int16_t>) {
      return get_int16();
    }
#endif

// Unsigned 32-bit (covers fixed + packed encodings)
#if SUB8_ENABLE_UINT32 || SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    else if constexpr (std::is_same_v<T, uint32_t>) {
      return get_uint32();
    }
#endif

// Signed 32-bit (covers fixed + packed encodings)
#if SUB8_ENABLE_INT32 || SUB8_ENABLE_VARIABLE_LENGTH_INT32
    else if constexpr (std::is_same_v<T, int32_t>) {
      return get_int32();
    }
#endif

// Unsigned 64-bit (covers fixed + packed encodings)
#if SUB8_ENABLE_UINT64 || SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    else if constexpr (std::is_same_v<T, uint64_t>) {
      return get_uint64();
    }
#endif

// Signed 64-bit (covers fixed + packed encodings)
#if SUB8_ENABLE_INT64 || SUB8_ENABLE_VARIABLE_LENGTH_INT64
    else if constexpr (std::is_same_v<T, int64_t>) {
      return get_int64();
    }
#endif

// Float32 / Float16
#if SUB8_ENABLE_FLOAT32 || SUB8_ENABLE_FLOAT16
    else if constexpr (std::is_same_v<T, float>) {
#if SUB8_ENABLE_FLOAT32
      if (auto v = get_float32()) {
        return v;
      }
#endif
#if SUB8_ENABLE_FLOAT16
      if (auto v = get_float16()) {
        return v;
      }
#endif
      return nullptr;
    }
#endif

// Float64
#if SUB8_ENABLE_FLOAT64
    else if constexpr (std::is_same_v<T, double>) {
      return get_float64();
    }
#endif
    return nullptr;
  }
};

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const PrimitiveVariant &field) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  VariantValueTypeField type_field;
  BitFieldResult r = type_field.set_value(field.type);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  r = write_field(bw, type_field);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  switch (field.type) {
  case VariantValueType::NullValue: {
    return BitFieldResult::Ok;
  }

#if SUB8_ENABLE_BOOL
  case VariantValueType::Boolean: {
    return write_field(bw, field.variant.bool_v);
  }
#endif

#if SUB8_ENABLE_UINT4
  case VariantValueType::U4: {
    return write_field(bw, field.variant.u4_v);
  }
#endif

#if SUB8_ENABLE_UINT8
  case VariantValueType::U8: {
    return write_field(bw, field.variant.u8_v);
  }
#endif

#if SUB8_ENABLE_INT8
  case VariantValueType::I8: {
    return write_field(bw, field.variant.i8_v);
  }
#endif

#if SUB8_ENABLE_UINT16
  case VariantValueType::U16: {
    return write_field(bw, field.variant.u16_v);
  }
#endif

#if SUB8_ENABLE_INT16
  case VariantValueType::I16: {
    return write_field(bw, field.variant.i16_v);
  }
#endif

#if SUB8_ENABLE_UINT32
  case VariantValueType::U32: {
    return write_field(bw, field.variant.u32_v);
  }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
  case VariantValueType::U32P8: {
    return write_field(bw, field.variant.u32_v);
  }
  case VariantValueType::U32P16: {
    return write_field(bw, field.variant.u32_v);
  }
#endif

#if SUB8_ENABLE_INT32
  case VariantValueType::I32: {
    return write_field(bw, field.variant.i32_v);
  }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
  case VariantValueType::I32P8: {
    return write_field(bw, field.variant.i32_v);
  }
  case VariantValueType::I32P16: {
    return write_field(bw, field.variant.i32_v);
  }
#endif

#if SUB8_ENABLE_UINT64
  case VariantValueType::U64: {
    return write_field(bw, field.variant.u64_v);
  }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
  case VariantValueType::U64P32: {
    return write_field(bw, field.variant.u64_v);
  }
#endif

#if SUB8_ENABLE_INT64
  case VariantValueType::I64: {
    return write_field(bw, field.variant.i64_v);
  }
  case VariantValueType::I64P32: {
    return write_field(bw, field.variant.i64_v);
  }
#endif

#if SUB8_ENABLE_FLOAT16
  case VariantValueType::Float16ValueField: {
    return write_field(bw, field.variant.f16_v);
  }
#endif

#if SUB8_ENABLE_FLOAT32
  case VariantValueType::Float32ValueField: {
    return write_field(bw, field.variant.f32_v);
  }
#endif

#if SUB8_ENABLE_FLOAT64
  case VariantValueType::Float64ValueField: {
    return write_field(bw, field.variant.f64_v);
  }
#endif

  default:
    return BitFieldResult::WarningNotSupportedConfiguration;
  }
}

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br, PrimitiveVariant &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  BitFieldResult r;

  VariantValueTypeField type_field;
  r = read_field(br, type_field);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  out.type = type_field.value();

  switch (out.type) {
  case VariantValueType::NullValue: {
    out.variant.null_v = NullValue{};
    return BitFieldResult::Ok;
  }

#if SUB8_ENABLE_BOOL
  case VariantValueType::Boolean: {
    return read_field(br, out.variant.bool_v);
  }
#endif

#if SUB8_ENABLE_UINT4
  case VariantValueType::U4: {
    return read_field(br, out.variant.u4_v);
  }
#endif

#if SUB8_ENABLE_UINT8
  case VariantValueType::U8: {
    return read_field(br, out.variant.u8_v);
  }
#endif

#if SUB8_ENABLE_INT8
  case VariantValueType::I8: {
    return read_field(br, out.variant.i8_v);
  }
#endif

#if SUB8_ENABLE_UINT16
  case VariantValueType::U16: {
    return read_field(br, out.variant.u16_v);
  }
#endif

#if SUB8_ENABLE_INT16
  case VariantValueType::I16: {
    return read_field(br, out.variant.i16_v);
  }
#endif

#if SUB8_ENABLE_UINT32
  case VariantValueType::U32: {
    return read_field(br, out.variant.u32_v);
  }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
  case VariantValueType::U32P8: {
    return read_field(br, out.variant.u32_v);
  }
  case VariantValueType::U32P16: {
    return read_field(br, out.variant.u32_v);
  }
#endif

#if SUB8_ENABLE_INT32
  case VariantValueType::I32: {
    return read_field(br, out.variant.i32_v);
  }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
  case VariantValueType::I32P8: {
    return read_field(br, out.variant.i32_v);
  }
  case VariantValueType::I32P16: {
    return read_field(br, out.variant.i32_v);
  }
#endif

#if SUB8_ENABLE_UINT64
  case VariantValueType::U64: {
    return read_field(br, out.variant.u64_v);
  }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
  case VariantValueType::U64P32: {
    return read_field(br, out.variant.u64_v);
  }
#endif

#if SUB8_ENABLE_INT64
  case VariantValueType::I64: {
    return read_field(br, out.variant.i64_v);
  }
  case VariantValueType::I64P32: {
    return read_field(br, out.variant.i64_v);
  }
#endif

#if SUB8_ENABLE_FLOAT16
  case VariantValueType::Float16ValueField: {
    return read_field(br, out.variant.f16_v);
  }
#endif

#if SUB8_ENABLE_FLOAT32
  case VariantValueType::Float32ValueField: {
    return read_field(br, out.variant.f32_v);
  }
#endif

#if SUB8_ENABLE_FLOAT64
  case VariantValueType::Float64ValueField: {
    return read_field(br, out.variant.f64_v);
  }
#endif

  default:
    return BitFieldResult::WarningNotSupportedConfiguration;
  }
}

// Null variant
inline PrimitiveVariant make_null_variant() {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::NullValue;
  return pv;
}

#if SUB8_ENABLE_BOOL
inline PrimitiveVariant make_bool_variant(bool v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::Boolean;
  SUB8_THROW_IF_ERROR(pv.variant.bool_v.set_value(v), PrimitiveVariant, "sub8::make_bool_variant(bool v)");
  return pv;
}
#endif

#if SUB8_ENABLE_UINT4
inline PrimitiveVariant make_uint4_variant(uint8_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U4;
  SUB8_THROW_IF_ERROR(pv.variant.u4_v.set_value(v), PrimitiveVariant, "make_uint4_variant(uint8_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_UINT8
inline PrimitiveVariant make_uint8_variant(uint8_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U8;
  SUB8_THROW_IF_ERROR(pv.variant.u8_v.set_value(v), PrimitiveVariant, "make_uint8_variant(uint8_t v)");
  return pv;
}
#endif

#if SUB8_ENABLE_INT8
inline PrimitiveVariant make_int8_variant(int8_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I8;
  SUB8_THROW_IF_ERROR(pv.variant.i8_v.set_value(v), PrimitiveVariant, "make_int8_variant(int8_t v)");
  return pv;
}
#endif

#if SUB8_ENABLE_UINT16
inline PrimitiveVariant make_uint16_variant(uint16_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U16;
  SUB8_THROW_IF_ERROR(pv.variant.u16_v.set_value(v), PrimitiveVariant, "make_uint16_variant(uint16_t v)");
  return pv;
}
#endif

#if SUB8_ENABLE_INT16
inline PrimitiveVariant make_int16_variant(int16_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I16;
  SUB8_THROW_IF_ERROR(pv.variant.i16_v.set_value(v), PrimitiveVariant, "make_int16_variant(int16_t v)");
  return pv;
}
#endif

#if SUB8_ENABLE_UINT32
// Fixed-length uint32
inline PrimitiveVariant make_uint32_variant(uint32_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U32;
  SUB8_THROW_IF_ERROR(pv.variant.u32_v.set_value(v), PrimitiveVariant, "make_uint32_variant(uint32_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32

// Packed uint32 (8-bit Groups)
inline PrimitiveVariant make_uint32_p8_variant(uint32_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U32P8;
  SUB8_THROW_IF_ERROR(pv.variant.u32_p8_v.set_value(v), PrimitiveVariant, "make_uint32_p8_variant(uint32_t v)");
  return pv;
}

// Packed uint32 (16-bit Groups)
inline PrimitiveVariant make_uint32_p16_variant(uint32_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U32P16;
  SUB8_THROW_IF_ERROR(pv.variant.u32_p16_v.set_value(v), PrimitiveVariant, "make_uint32_p16_variant(uint32_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_INT32

// Fixed-length int32
inline PrimitiveVariant make_int32_variant(int32_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I32;
  SUB8_THROW_IF_ERROR(pv.variant.i32_v.set_value(v), PrimitiveVariant, "make_int32_variant(int32_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32

// Packed int32 (8-bit Groups)
inline PrimitiveVariant make_int32_p8_variant(int32_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I32P8;
  SUB8_THROW_IF_ERROR(pv.variant.i32_p8_v.set_value(v), PrimitiveVariant, "make_int32_p8_variant(int32_t v)");
  return pv;
}

// Packed int32 (16-bit Groups)
inline PrimitiveVariant make_int32_p16_variant(int32_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I32P16;

  SUB8_THROW_IF_ERROR(pv.variant.i32_p16_v.set_value(v), PrimitiveVariant, "make_int32_p16_variant(int32_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_UINT64

// Fixed-length uint64
inline PrimitiveVariant make_uint64_variant(uint64_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U64;
  SUB8_THROW_IF_ERROR(pv.variant.u64_v.set_value(v), PrimitiveVariant, "make_uint64_variant(uint64_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64

// Packed uint64 (32-bit Groups)
inline PrimitiveVariant make_uint64_p32_variant(uint64_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::U64P32;
  SUB8_THROW_IF_ERROR(pv.variant.u64_p32_v.set_value(v), PrimitiveVariant, "make_uint64_p32_variant(uint64_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_INT64
// Fixed-length int64
inline PrimitiveVariant make_int64_variant(int64_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I64;
  SUB8_THROW_IF_ERROR(pv.variant.i64_v.set_value(v), PrimitiveVariant, "make_int64_variant(int64_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64

// Packed int64 (32-bit Groups)
inline PrimitiveVariant make_int64_p32_variant(int64_t v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::I64P32;
  SUB8_THROW_IF_ERROR(pv.variant.i64_p32_v.set_value(v), PrimitiveVariant, "make_int64_p32_variant(int64_t v)");
  return pv;
}

#endif

#if SUB8_ENABLE_FLOAT16
inline PrimitiveVariant make_float16_variant(float v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::Float16ValueField;
  SUB8_THROW_IF_ERROR(pv.variant.f16_v.set_value(v), PrimitiveVariant, "make_float16_variant(float v)");
  return pv;
}
#endif

#if SUB8_ENABLE_FLOAT32
inline PrimitiveVariant make_float32_variant(float v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::Float32ValueField;
  SUB8_THROW_IF_ERROR(pv.variant.f32_v.set_value(v), PrimitiveVariant, "make_float32_variant(float v)");
  return pv;
}
#endif

#if SUB8_ENABLE_FLOAT64
inline PrimitiveVariant make_float64_variant(double v) {
  PrimitiveVariant pv{};
  pv.type = VariantValueType::Float64ValueField;
  SUB8_THROW_IF_ERROR(pv.variant.f64_v.set_value(v), PrimitiveVariant, "make_float64_variant(double v)");
  return pv;
}
#endif

} // namespace sub8

#endif
