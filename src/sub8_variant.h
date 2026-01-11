#pragma once

// Enable: Variant Fields
// Support for predefined set of variant data types on the wire
#ifndef SUB8_ENABLE_VARIANT
#define SUB8_ENABLE_VARIANT 1
#endif

#if SUB8_ENABLE_VARIANT

#include "sub8_errors.h"
#include "sub8_io.h"
#include "sub8_primitives.h"
#include "sub8_floats.h"

namespace sub8 {

// VariantValueField (FixedLength, 5 bits)
enum class VariantValueType : uint32_t {
  NullValueField = 0,
  BoolValueField = 1,
  Uint4ValueField = 2,
  Uint8ValueField = 3,
  Int8ValueField = 4,
  Uint16ValueField = 5,
  Int16ValueField = 6,
  Uint32ValueField = 7,
  Int32ValueField = 8,
  Uint64ValueField = 9,
  Int64ValueField = 10,

  Uint32Pack8ValueField = 11,
  Uint32Pack16ValueField = 12,

  Uint64Pack32ValueField = 13,

  Int32Pack8ValueField = 14,
  Int32Pack16ValueField = 15,

  Int64Pack32ValueField = 16,

  Float16ValueField = 17,
  Float32ValueField = 18,
  Float64ValueField = 19,
};

SUB8_DECLARE_BITFIELD_ALIAS(VariantValueTypeField, FixedLengthBitField<VariantValueType, /*DataBitsPerGroup=*/5>);

struct VariantValueField {
  VariantValueType type;

  union VariantValue {
    NullValueField null_v;
#if SUB8_ENABLE_BOOL
    BoolValueField bool_v;
#endif
#if SUB8_ENABLE_UINT4
    Uint4ValueField u4_v;
#endif
#if SUB8_ENABLE_UINT8
    Uint8ValueField u8_v;
#endif
#if SUB8_ENABLE_UINT16
    Uint16ValueField u16_v;
#endif
#if SUB8_ENABLE_UINT32
    Uint32ValueField u32_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    Uint32Pack8ValueField u32_p8_v;
    Uint32Pack16ValueField u32_p16_v;
#endif
#if SUB8_ENABLE_UINT64
    Uint64ValueField u64_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    Uint64Pack32ValueField u64_p32_v;
#endif
#if SUB8_ENABLE_INT8
    Int8ValueField i8_v;
#endif
#if SUB8_ENABLE_INT16
    Int16ValueField i16_v;
#endif
#if SUB8_ENABLE_INT32
    Int32ValueField i32_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    Int32Pack8ValueField i32_p8_v;
    Int32Pack16ValueField i32_p16_v;
#endif
#if SUB8_ENABLE_INT64
    Int64ValueField i64_v;
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    Int64Pack32ValueField i64_p32_v;
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
    VariantValue() : null_v(NullValueField()) {}
  } variant;

  using Type = VariantValueField;
  using InitType = VariantValueField;
  using ValueType = VariantValueField;

  const VariantValueField &value() const noexcept { return *this; }

  bool operator==(const VariantValueField &o) const noexcept {
    if (type != o.type) {
      return false;
    }

    switch (type) {
      case VariantValueType::NullValueField: {
        return true;
      }

#if SUB8_ENABLE_BOOL
      case VariantValueType::BoolValueField: {
        return variant.bool_v == o.variant.bool_v;
      }
#endif

#if SUB8_ENABLE_UINT4
      case VariantValueType::Uint4ValueField: {
        return variant.u4_v == o.variant.u4_v;
      }
#endif

#if SUB8_ENABLE_UINT8
      case VariantValueType::Uint8ValueField: {
        return variant.u8_v == o.variant.u8_v;
      }
#endif

#if SUB8_ENABLE_INT8
      case VariantValueType::Int8ValueField: {
        return variant.i8_v == o.variant.i8_v;
      }
#endif

#if SUB8_ENABLE_UINT16
      case VariantValueType::Uint16ValueField: {
        return variant.u16_v == o.variant.u16_v;
      }
#endif

#if SUB8_ENABLE_INT16
      case VariantValueType::Int16ValueField: {
        return variant.i16_v == o.variant.i16_v;
      }
#endif

#if SUB8_ENABLE_UINT32
      case VariantValueType::Uint32ValueField: {
        return variant.u32_v == o.variant.u32_v;
      }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
      case VariantValueType::Uint32Pack8ValueField: {
        return variant.u32_p8_v == o.variant.u32_p8_v;
      }
      case VariantValueType::Uint32Pack16ValueField: {
        return variant.u32_p16_v == o.variant.u32_p16_v;
      }
#endif

#if SUB8_ENABLE_INT32
      case VariantValueType::Int32ValueField: {
        return variant.i32_v == o.variant.i32_v;
      }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
      case VariantValueType::Int32Pack8ValueField: {
        return variant.i32_p8_v == o.variant.i32_p8_v;
      }
      case VariantValueType::Int32Pack16ValueField: {
        return variant.i32_p16_v == o.variant.i32_p16_v;
      }
#endif

#if SUB8_ENABLE_UINT64
      case VariantValueType::Uint64ValueField: {
        return variant.u64_v == o.variant.u64_v;
      }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
      case VariantValueType::Uint64Pack32ValueField: {
        return variant.u64_p32_v == o.variant.u64_p32_v;
      }
#endif

#if SUB8_ENABLE_INT64
      case VariantValueType::Int64ValueField: {
        return variant.i64_v == o.variant.i64_v;
      }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
      case VariantValueType::Int64Pack32ValueField: {
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

  bool operator!=(const VariantValueField &o) const noexcept { return !(*this == o); }

  BitFieldResult set_value(VariantValueField v) noexcept {
    type = v.type;
    variant = v.variant;
    return BitFieldResult::Ok;
  }

  bool is_null() { return type == VariantValueType::NullValueField; }

#if SUB8_ENABLE_BOOL
  const bool *get_bool() {
    if (type == VariantValueType::BoolValueField) {
      return &variant.bool_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_bool(bool b) {
    type = VariantValueType::BoolValueField;
    variant.bool_v = b;
    return BitFieldResult::Ok;
  }

  bool is_bool() { return type == VariantValueType::BoolValueField; }
#endif

#if SUB8_ENABLE_UINT4
  const uint8_t *get_uint4() {
    if (type == VariantValueType::Uint4ValueField) {
      return &variant.u4_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_uint4(size_t v) {
    auto r = variant.u4_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint4ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  bool is_uint4() { return type == VariantValueType::Uint4ValueField; }
#endif

#if SUB8_ENABLE_UINT8
  const uint8_t *get_uint8() {
    if (type == VariantValueType::Uint8ValueField) {
      return &variant.u8_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_uint8(uint8_t v) {
    auto r = variant.u8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint8ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  bool is_uint8() { return type == VariantValueType::Uint8ValueField; }
#endif

#if SUB8_ENABLE_UINT16
  inline const uint16_t *get_uint16() {
    if (type == VariantValueType::Uint16ValueField) {
      return &variant.u16_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_uint16(uint16_t v) {
    auto r = variant.u16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint16ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  bool is_uint16() { return type == VariantValueType::Uint16ValueField; }
#endif

#if SUB8_ENABLE_UINT32 || SUB8_ENABLE_VARIABLE_LENGTH_UINT32

  inline const uint32_t *get_uint32() {
#if SUB8_ENABLE_UINT32
    if (type == VariantValueType::Uint32ValueField) {
      return &variant.u32_v.value();
    }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    if (type == VariantValueType::Uint32Pack8ValueField) {
      return &variant.u32_p8_v.value();
    }
    if (type == VariantValueType::Uint32Pack16ValueField) {
      return &variant.u32_p16_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_UINT32
  BitFieldResult set_uint32(uint32_t v) {
    auto r = variant.u32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint32ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
  BitFieldResult set_uint32_p8(uint32_t v) {
    auto r = variant.u32_p8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint32Pack8ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  BitFieldResult set_uint32_p16(uint32_t v) {
    auto r = variant.u32_p16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint32Pack16ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }
#endif

  bool is_uint32() {
    switch (type) {
      case VariantValueType::Uint32ValueField:
      case VariantValueType::Uint32Pack8ValueField:
      case VariantValueType::Uint32Pack16ValueField:
        return true;
      default:
        return false;
    }
  }
#endif

#if SUB8_ENABLE_UINT64 || SUB8_ENABLE_VARIABLE_LENGTH_UINT64
  inline const uint64_t *get_uint64() {
#if SUB8_ENABLE_UINT64
    if (type == VariantValueType::Uint64ValueField) {
      return &variant.u64_v.value();
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
    if (type == VariantValueType::Uint64Pack32ValueField) {
      return &variant.u64_p32_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_UINT64
  BitFieldResult set_uint64(uint64_t v) {
    auto r = variant.u64_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint64ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
  BitFieldResult set_uint64_p32(uint64_t v) {
    auto r = variant.u64_p32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Uint64Pack32ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }
#endif

  bool is_uint64() {
    switch (type) {
      case VariantValueType::Uint64ValueField:
      case VariantValueType::Uint64Pack32ValueField:
        return true;
      default:
        return false;
    }
  }
#endif

#if SUB8_ENABLE_INT8
  const int8_t *get_int8() {
    if (type == VariantValueType::Int8ValueField) {
      return &variant.i8_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_int8(int8_t v) {
    auto r = variant.i8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int8ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  bool is_int8() { return type == VariantValueType::Int8ValueField; }
#endif

#if SUB8_ENABLE_INT16
  const int16_t *get_int16() {
    if (type == VariantValueType::Int16ValueField) {
      return &variant.i16_v.value();
    }
    return nullptr;
  }

  BitFieldResult set_int16(int16_t v) {
    auto r = variant.i16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int16ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  bool is_int16() { return type == VariantValueType::Int16ValueField; }

#endif

#if SUB8_ENABLE_INT32 || SUB8_ENABLE_VARIABLE_LENGTH_INT32
  const int32_t *get_int32() {
#if SUB8_ENABLE_INT32
    if (type == VariantValueType::Int32ValueField) {
      return &variant.i32_v.value();
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    if (type == VariantValueType::Int32Pack8ValueField) {
      return &variant.i32_p8_v.value();
    }
    if (type == VariantValueType::Int32Pack16ValueField) {
      return &variant.i32_p16_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_INT32

  BitFieldResult set_int32(int32_t v) {
    auto r = variant.i32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int32ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32

  BitFieldResult set_int32_p8(int32_t v) {
    auto r = variant.i32_p8_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int32Pack8ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

  BitFieldResult set_int32_p16(int32_t v) {
    auto r = variant.i32_p16_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int32Pack16ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

#endif

  bool is_int32() {
    switch (type) {
      case VariantValueType::Int32ValueField:
      case VariantValueType::Int32Pack8ValueField:
      case VariantValueType::Int32Pack16ValueField:
        return true;
      default:
        return false;
    }
  }
#endif

#if SUB8_ENABLE_INT64 || SUB8_ENABLE_VARIABLE_LENGTH_INT64

  const int64_t *get_int64() const {
#if SUB8_ENABLE_INT64
    if (type == VariantValueType::Int64ValueField) {
      return &variant.i64_v.value();
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    if (type == VariantValueType::Int64Pack32ValueField) {
      return &variant.i64_p32_v.value();
    }
#endif
    return nullptr;
  }

#if SUB8_ENABLE_INT64
  BitFieldResult set_int64(int64_t v) {
    auto r = variant.i64_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int64ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
  BitFieldResult set_int64_p32(int64_t v) {
    auto r = variant.i64_p32_v.set_value(v);
    if (r == BitFieldResult::Ok) {
      type = VariantValueType::Int64Pack32ValueField;
      return BitFieldResult::Ok;
    }
    type = VariantValueType::NullValueField;
    return r;
  }
#endif

  bool is_int64() const {
    switch (type) {
      case VariantValueType::Int64ValueField:
      case VariantValueType::Int64Pack32ValueField:
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
    type = VariantValueType::NullValueField;
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
    type = VariantValueType::NullValueField;
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
    type = VariantValueType::NullValueField;
    return r;
  }

  bool is_float64() const { return type == VariantValueType::Float64ValueField; }
#endif

  template<typename T> inline const T *value() {
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

template<typename Storage>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const VariantValueField &field) {
  VariantValueTypeField type_field;
  type_field.set_value(field.type);

  BitFieldResult r = write_field(bw, type_field);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  switch (field.type) {
    case VariantValueType::NullValueField: {
      return BitFieldResult::Ok;
    }

#if SUB8_ENABLE_BOOL
    case VariantValueType::BoolValueField: {
      return write_field(bw, field.variant.bool_v);
    }
#endif

#if SUB8_ENABLE_UINT4
    case VariantValueType::Uint4ValueField: {
      return write_field(bw, field.variant.u4_v);
    }
#endif

#if SUB8_ENABLE_UINT8
    case VariantValueType::Uint8ValueField: {
      return write_field(bw, field.variant.u8_v);
    }
#endif

#if SUB8_ENABLE_INT8
    case VariantValueType::Int8ValueField: {
      return write_field(bw, field.variant.i8_v);
    }
#endif

#if SUB8_ENABLE_UINT16
    case VariantValueType::Uint16ValueField: {
      return write_field(bw, field.variant.u16_v);
    }
#endif

#if SUB8_ENABLE_INT16
    case VariantValueType::Int16ValueField: {
      return write_field(bw, field.variant.i16_v);
    }
#endif

#if SUB8_ENABLE_UINT32
    case VariantValueType::Uint32ValueField: {
      return write_field(bw, field.variant.u32_v);
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    case VariantValueType::Uint32Pack8ValueField: {
      return write_field(bw, field.variant.u32_v);
    }
    case VariantValueType::Uint32Pack16ValueField: {
      return write_field(bw, field.variant.u32_v);
    }
#endif

#if SUB8_ENABLE_INT32
    case VariantValueType::Int32ValueField: {
      return write_field(bw, field.variant.i32_v);
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    case VariantValueType::Int32Pack8ValueField: {
      return write_field(bw, field.variant.i32_v);
    }
    case VariantValueType::Int32Pack16ValueField: {
      return write_field(bw, field.variant.i32_v);
    }
#endif

#if SUB8_ENABLE_UINT64
    case VariantValueType::Uint64ValueField: {
      return write_field(bw, field.variant.u64_v);
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    case VariantValueType::Uint64Pack32ValueField: {
      return write_field(bw, field.variant.u64_v);
    }
#endif

#if SUB8_ENABLE_INT64
    case VariantValueType::Int64ValueField: {
      return write_field(bw, field.variant.i64_v);
    }
    case VariantValueType::Int64Pack32ValueField: {
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

template<typename Storage> inline BitFieldResult read_field(BasicBitReader<Storage> &br, VariantValueField &out) {
  BitFieldResult r;

  VariantValueTypeField type_field;
  r = read_field(br, type_field);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  out.type = type_field.value();

  switch (out.type) {
    case VariantValueType::NullValueField: {
      out.variant.null_v = NullValueField{};
      return BitFieldResult::Ok;
    }

#if SUB8_ENABLE_BOOL
    case VariantValueType::BoolValueField: {
      return read_field(br, out.variant.bool_v);
    }
#endif

#if SUB8_ENABLE_UINT4
    case VariantValueType::Uint4ValueField: {
      return read_field(br, out.variant.u4_v);
    }
#endif

#if SUB8_ENABLE_UINT8
    case VariantValueType::Uint8ValueField: {
      return read_field(br, out.variant.u8_v);
    }
#endif

#if SUB8_ENABLE_INT8
    case VariantValueType::Int8ValueField: {
      return read_field(br, out.variant.i8_v);
    }
#endif

#if SUB8_ENABLE_UINT16
    case VariantValueType::Uint16ValueField: {
      return read_field(br, out.variant.u16_v);
    }
#endif

#if SUB8_ENABLE_INT16
    case VariantValueType::Int16ValueField: {
      return read_field(br, out.variant.i16_v);
    }
#endif

#if SUB8_ENABLE_UINT32
    case VariantValueType::Uint32ValueField: {
      return read_field(br, out.variant.u32_v);
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
    case VariantValueType::Uint32Pack8ValueField: {
      return read_field(br, out.variant.u32_v);
    }
    case VariantValueType::Uint32Pack16ValueField: {
      return read_field(br, out.variant.u32_v);
    }
#endif

#if SUB8_ENABLE_INT32
    case VariantValueType::Int32ValueField: {
      return read_field(br, out.variant.i32_v);
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
    case VariantValueType::Int32Pack8ValueField: {
      return read_field(br, out.variant.i32_v);
    }
    case VariantValueType::Int32Pack16ValueField: {
      return read_field(br, out.variant.i32_v);
    }
#endif

#if SUB8_ENABLE_UINT64
    case VariantValueType::Uint64ValueField: {
      return read_field(br, out.variant.u64_v);
    }
#endif
#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
    case VariantValueType::Uint64Pack32ValueField: {
      return read_field(br, out.variant.u64_v);
    }
#endif

#if SUB8_ENABLE_INT64
    case VariantValueType::Int64ValueField: {
      return read_field(br, out.variant.i64_v);
    }
    case VariantValueType::Int64Pack32ValueField: {
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
inline VariantValueField make_null_variant() {
  VariantValueField pv{};
  pv.type = VariantValueType::NullValueField;
  return pv;
}

#if SUB8_ENABLE_BOOL
inline VariantValueField make_bool_variant(bool v) {
  VariantValueField pv{};
  pv.type = VariantValueType::BoolValueField;
  pv.variant.bool_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_UINT4
inline VariantValueField make_uint4_variant(uint8_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint4ValueField;
  pv.variant.u4_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_UINT8
inline VariantValueField make_uint8_variant(uint8_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint8ValueField;
  pv.variant.u8_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_INT8
inline VariantValueField make_int8_variant(int8_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int8ValueField;
  pv.variant.i8_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_UINT16
inline VariantValueField make_uint16_variant(uint16_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint16ValueField;
  pv.variant.u16_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_INT16
inline VariantValueField make_int16_variant(int16_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int16ValueField;
  pv.variant.i16_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_UINT32
// Fixed-length uint32
inline VariantValueField make_uint32_variant(uint32_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint32ValueField;
  pv.variant.u32_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32

// Packed uint32 (8-bit Groups)
inline VariantValueField make_uint32_p8_variant(uint32_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint32Pack8ValueField;
  pv.variant.u32_p8_v.set_value(v);
  return pv;
}

// Packed uint32 (16-bit Groups)
inline VariantValueField make_uint32_p16_variant(uint32_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint32Pack16ValueField;
  pv.variant.u32_p16_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_INT32

// Fixed-length int32
inline VariantValueField make_int32_variant(int32_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int32ValueField;
  pv.variant.i32_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32

// Packed int32 (8-bit Groups)
inline VariantValueField make_int32_p8_variant(int32_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int32Pack8ValueField;
  pv.variant.i32_p8_v.set_value(v);
  return pv;
}

// Packed int32 (16-bit Groups)
inline VariantValueField make_int32_p16_variant(int32_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int32Pack16ValueField;
  pv.variant.i32_p16_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_UINT64

// Fixed-length uint64
inline VariantValueField make_uint64_variant(uint64_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint64ValueField;
  pv.variant.u64_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64

// Packed uint64 (32-bit Groups)
inline VariantValueField make_uint64_p32_variant(uint64_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Uint64Pack32ValueField;
  pv.variant.u64_p32_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_INT64
// Fixed-length int64
inline VariantValueField make_int64_variant(int64_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int64ValueField;
  pv.variant.i64_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64

// Packed int64 (32-bit Groups)
inline VariantValueField make_int64_p32_variant(int64_t v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Int64Pack32ValueField;
  pv.variant.i64_p32_v.set_value(v);
  return pv;
}

#endif

#if SUB8_ENABLE_FLOAT16
inline VariantValueField make_float16_variant(float v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Float16ValueField;
  pv.variant.f16_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_FLOAT32
inline VariantValueField make_float32_variant(float v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Float32ValueField;
  pv.variant.f32_v.set_value(v);
  return pv;
}
#endif

#if SUB8_ENABLE_FLOAT64
inline VariantValueField make_float64_variant(double v) {
  VariantValueField pv{};
  pv.type = VariantValueType::Float64ValueField;
  pv.variant.f64_v.set_value(v);
  return pv;
}
#endif

}  // namespace sub8

#endif
