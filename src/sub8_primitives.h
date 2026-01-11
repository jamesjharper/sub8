#pragma once

#include "sub8_basic.h"
#include "sub8_errors.h"
#include "sub8_io.h"

// Enable: Fixed Length Fields.
// Fields with a fixed predetermined length.
// All values are the same size on the wire, signed values are zig zag encoded
#ifndef SUB8_ENABLE_FIXED_LENGTH_FIELDS
#define SUB8_ENABLE_FIXED_LENGTH_FIELDS 1
#endif

// Enable: Variable Length Fields
// Fields encoded into groups for n bit length. Uses continuation bits to indicate more
// Smaller values more efficiently packed on the write. Larger values are less efficiently packed due to continuation
// bit overhead. Uses zig zag encoding to ensure negative numbers do not always consume all groups
#ifndef SUB8_ENABLE_VARIABLE_LENGTH_FIELDS
#define SUB8_ENABLE_VARIABLE_LENGTH_FIELDS 1
#endif

// Enable predefined set of common data types
#ifndef SUB8_ENABLE_BOOL
#define SUB8_ENABLE_BOOL 1
#endif

#ifndef SUB8_ENABLE_UINT4
#define SUB8_ENABLE_UINT4 1
#endif

#ifndef SUB8_ENABLE_UINT8
#define SUB8_ENABLE_UINT8 1
#endif

#ifndef SUB8_ENABLE_UINT16
#define SUB8_ENABLE_UINT16 1
#endif

#ifndef SUB8_ENABLE_UINT64
#define SUB8_ENABLE_UINT64 1
#endif

#ifndef SUB8_ENABLE_VARIABLE_LENGTH_UINT64
#define SUB8_ENABLE_VARIABLE_LENGTH_UINT64 SUB8_ENABLE_UINT64
#endif

#ifndef SUB8_ENABLE_INT8
#define SUB8_ENABLE_INT8 1
#endif

#ifndef SUB8_ENABLE_INT16
#define SUB8_ENABLE_INT16 1
#endif

#ifndef SUB8_ENABLE_UINT32
#define SUB8_ENABLE_UINT32 1
#endif

#ifndef SUB8_ENABLE_VARIABLE_LENGTH_UINT32
#define SUB8_ENABLE_VARIABLE_LENGTH_UINT32 SUB8_ENABLE_UINT32
#endif

#ifndef SUB8_ENABLE_INT32
#define SUB8_ENABLE_INT32 1
#endif

#ifndef SUB8_ENABLE_VARIABLE_LENGTH_INT32
#define SUB8_ENABLE_VARIABLE_LENGTH_INT32 SUB8_ENABLE_INT32
#endif

#ifndef SUB8_ENABLE_INT64
#define SUB8_ENABLE_INT64 1
#endif

#ifndef SUB8_ENABLE_VARIABLE_LENGTH_INT64
#define SUB8_ENABLE_VARIABLE_LENGTH_INT64 SUB8_ENABLE_INT64
#endif

// Primitive Field Types
// =======================

namespace sub8 {

// Foundation Field Types
// =======================

// null Field
// ------------
struct NullValueField {
  using Type = void;
  using ValueType = void;
  using InitType = void;

  using UnderlyingType = void;
  using StorageType = void;
};

template<typename Storage> inline BitFieldResult write_field(BasicBitWriter<Storage> &, NullValueField) noexcept {
  return BitFieldResult::ErrorCanNotWriteNullValue;
}

template<typename Storage> inline BitFieldResult read_field(BasicBitReader<Storage> &, NullValueField &) noexcept {
  return BitFieldResult::ErrorCanNotReadNullValue;
}

// Fixed Length Field
// ------------------
#if SUB8_ENABLE_FIXED_LENGTH_FIELDS

template<
    // T: storage type, must be equal or greater than the wire bit size
    typename T,
    // BitLength: the number of bits the type can utilize
    uint32_t BitLength>
struct FixedLengthBitField {
 public:
  using Type = T;
  using ValueType = T;
  using InitType = T;

  using UnderlyingType = typename unpack_t::underlying_or_self<T>::type;
  using StorageType = std::make_unsigned_t<UnderlyingType>;

  static_assert(BitLength >= 1);

  static constexpr uint32_t TotalUsableBits = BitLength;
  static constexpr uint32_t StorageTypeBits = sizeof(StorageType) * 8;

  using BitLimits = sub8::limits::bits_limits<T, TotalUsableBits>;
  static constexpr StorageType MaxCode = BitLimits::MaxCode;
  static constexpr T MinValue = BitLimits::MinValue;
  static constexpr T MaxValue = BitLimits::MaxValue;

  T value_{};

  operator T() const noexcept { return value_; }
  const T &value() const noexcept { return value_; }

  bool operator==(const FixedLengthBitField &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const FixedLengthBitField &o) const noexcept { return !(*this == o); }

  FixedLengthBitField() noexcept = default;
  FixedLengthBitField(const FixedLengthBitField &) noexcept = default;

  FixedLengthBitField &operator=(const FixedLengthBitField &) noexcept = default;
  FixedLengthBitField &operator=(T v) noexcept {
    auto r = set_value(v);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  BitFieldResult set_value(InitType v) noexcept {
    if (v < MinValue || v > MaxValue)
      return BitFieldResult::ErrorValueTooLarge;

    value_ = v;
    return BitFieldResult::Ok;
  }
};

template<typename Storage, typename T, uint32_t BitLength>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const FixedLengthBitField<T, BitLength> &f) noexcept {
  using F = FixedLengthBitField<T, BitLength>;
  using StorageType = typename F::StorageType;

  StorageType v = sub8::packing::pack<T>(f.value());
  if (v > F::MaxCode)
    return BitFieldResult::ErrorValueTooLarge;

  return bw.template put_bits<StorageType>(v, F::TotalUsableBits);
}

template<typename Storage, typename T, uint32_t BitLength>
inline BitFieldResult read_field(BasicBitReader<Storage> &br, FixedLengthBitField<T, BitLength> &out)  noexcept {
  using F = FixedLengthBitField<T, BitLength>;
  using StorageType = typename F::StorageType;

  StorageType tmp = 0;
  auto r = br.template get_bits<StorageType>(tmp, F::TotalUsableBits);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  if (tmp > F::MaxCode) {
    return BitFieldResult::ErrorValueTooLarge;
  }

  return out.set_value(sub8::packing::unpack<T>(tmp));
}

#endif  // SUB8_ENABLE_FIXED_LENGTH_FIELDS

// Variable Length Field
// ----------------------
#if SUB8_ENABLE_VARIABLE_LENGTH_FIELDS

template<
    // T: storage type, must be equal or greater than the wire bit size
    typename T,
    // TDataBitsPerGroup: Size of each bit group. TDataBitsPerGroup * TMaxGroupCount can not be greater than the storage
    // type T
    uint32_t TDataBitsPerGroup,
    // TMaxGroupCount: Max number of groups. Assumed to be max whole groups which can fit within the storage type T
    uint32_t TMaxGroupCount =
        (sizeof(typename unpack_t::underlying_or_self<T>::type) * 8 + TDataBitsPerGroup - 1) / TDataBitsPerGroup>
struct VariableLengthBitField {
 public:
  using Type = T;
  using ValueType = T;
  using InitType = T;

  using UnderlyingType = typename unpack_t::underlying_or_self<T>::type;
  using StorageType = typename std::make_unsigned<UnderlyingType>::type;  // packed representation type

  static_assert(TDataBitsPerGroup >= 1, "TDataBitsPerGroup must be >= 1");
  static constexpr uint32_t MaxGroupCount = TMaxGroupCount;
  static constexpr uint32_t DataBitsPerGroup = (MaxGroupCount == 1) ? (TDataBitsPerGroup + 1) : TDataBitsPerGroup;
  static constexpr uint32_t GroupBitSize = (MaxGroupCount == 1) ? TDataBitsPerGroup : (TDataBitsPerGroup + 1);
  static constexpr uint32_t TotalUsableBits =
      (MaxGroupCount == 1) ? (TDataBitsPerGroup + 1) : (DataBitsPerGroup * MaxGroupCount);

  static_assert(MaxGroupCount > 0, "MaxGroupCount must be at least 1");
  static_assert(MaxGroupCount <= (sizeof(StorageType) * 8) / DataBitsPerGroup,
                "MaxGroupCount exceeds capacity of underlying type");

  static constexpr StorageType make_mask(uint32_t bits) noexcept {
    if (bits == 0)
      return StorageType{0};

    constexpr uint32_t W = sizeof(StorageType) * 8;
    if (bits >= W)
      return std::numeric_limits<StorageType>::max();
    return (StorageType{1} << bits) - StorageType{1};
  }

  static constexpr StorageType GroupDataMask = make_mask(DataBitsPerGroup);

  using BitLimits = sub8::limits::bits_limits<T, TotalUsableBits>;
  static constexpr StorageType MaxCode = BitLimits::MaxCode;
  static constexpr T MinValue = BitLimits::MinValue;
  static constexpr T MaxValue = BitLimits::MaxValue;

 public:
  T value_{};

  VariableLengthBitField() noexcept = default;
  VariableLengthBitField(const VariableLengthBitField &) noexcept = default;

  operator T() const noexcept { return value_; }
  const T &value() const noexcept { return value_; }

  bool operator==(const VariableLengthBitField &o) const noexcept { return value_ == o.value_; }
  bool operator!=(const VariableLengthBitField &o) const noexcept { return !(*this == o); }

  VariableLengthBitField &operator=(const VariableLengthBitField &) noexcept = default;

  VariableLengthBitField &operator=(T v) noexcept {
    auto r = set_value(v);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  BitFieldResult set_value(InitType v) noexcept {
    const UnderlyingType uv = static_cast<UnderlyingType>(v);
    if (uv < MinValue || uv > MaxValue)
      return BitFieldResult::ErrorValueTooLarge;

    value_ = v;
    return BitFieldResult::Ok;
  }
};

template<typename T, uint32_t TDataBitsPerGroup, uint32_t TMaxGroupCount>
inline uint32_t calculate_field_group_count(
    const VariableLengthBitField<T, TDataBitsPerGroup, TMaxGroupCount> &field) noexcept {
  using F = VariableLengthBitField<T, TDataBitsPerGroup, TMaxGroupCount>;
  using StorageType = typename F::StorageType;

  if constexpr (F::MaxGroupCount == 1) {
    return 1;
  }

  StorageType pack_bits = packing::pack(field.value());

  if (pack_bits == 0) {
    return 1;
  }

  uint32_t groups = 1;
  pack_bits >>= F::DataBitsPerGroup;

  while (pack_bits != 0 && groups < F::MaxGroupCount) {
    ++groups;
    pack_bits >>= F::DataBitsPerGroup;
  }

  return groups;
}

template<typename Storage, typename T, uint32_t TDataBitsPerGroup, uint32_t TMaxGroupCount>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw,
                                  const VariableLengthBitField<T, TDataBitsPerGroup, TMaxGroupCount> &field) noexcept {
  using F = VariableLengthBitField<T, TDataBitsPerGroup, TMaxGroupCount>;
  using StorageType = typename F::StorageType;

  StorageType v = packing::pack(field.value());

  if (v > F::MaxCode)
    return BitFieldResult::ErrorValueTooLarge;

  if constexpr (F::MaxGroupCount == 1) {
    return bw.template put_bits<StorageType>(v & F::GroupDataMask, F::DataBitsPerGroup);
  }

  if (v == 0) {
    return bw.template put_bits<StorageType>(0, F::GroupBitSize);
  }

  const uint32_t numGroups = calculate_field_group_count(field);
  if (numGroups > F::MaxGroupCount)
    return BitFieldResult::ErrorTooManyFragments;

  uint32_t shift = (numGroups - 1u) * F::DataBitsPerGroup;

  // Emit continuation groups: [1 | chunk]
  for (uint32_t g = 1; g < numGroups; ++g) {
    StorageType chunk = (v >> shift) & F::GroupDataMask;
    StorageType group_bits = (StorageType{1} << F::DataBitsPerGroup) | chunk;

    auto r = bw.template put_bits<StorageType>(group_bits, F::GroupBitSize);
    if (r != BitFieldResult::Ok)
      return r;

    shift -= F::DataBitsPerGroup;
  }

  // Last chunk
  StorageType last_chunk = v & F::GroupDataMask;

  if (numGroups == F::MaxGroupCount) {
    // Max groups: emit ONLY data bits (no cont-bit)
    return bw.template put_bits<StorageType>(last_chunk, F::DataBitsPerGroup);
  } else {
    // Fewer than max: emit [0 | data]
    return bw.template put_bits<StorageType>(last_chunk, F::GroupBitSize);
  }
}

template<typename Storage, typename T, uint32_t TDataBitsPerGroup, uint32_t TMaxGroupCount>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,
                                 VariableLengthBitField<T, TDataBitsPerGroup, TMaxGroupCount> &out) noexcept {
  using F = VariableLengthBitField<T, TDataBitsPerGroup, TMaxGroupCount>;
  using U = typename F::StorageType;

  U accum = 0;

  if constexpr (F::MaxGroupCount == 1) {
    U chunk_bits = 0;
    auto r = br.template get_bits<U>(chunk_bits, F::DataBitsPerGroup);
    if (r != BitFieldResult::Ok) {
      return r;
    }

    if (chunk_bits > F::MaxCode) {
      return BitFieldResult::ErrorValueTooLarge;
    }

    return out.set_value(packing::unpack<T>(chunk_bits));
  }

  constexpr U ACCUM_LIMIT = (std::numeric_limits<U>::max() >> F::DataBitsPerGroup);
  uint32_t groups_read = 0;

  while (true) {
    if (groups_read + 1 < F::MaxGroupCount) {
      U group_bits = 0;

      
      auto r = br.template get_bits<U>(group_bits, F::GroupBitSize);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      const U data = static_cast<U>(group_bits & F::GroupDataMask);
      const bool cont = (group_bits >> F::DataBitsPerGroup) != 0;

      if (accum > ACCUM_LIMIT) {
        return BitFieldResult::ErrorValueTooLarge;
      }

      accum = static_cast<U>((accum << F::DataBitsPerGroup) | data);
      ++groups_read;

      if (!cont)
        break;
    } else {
      // Last possible group
      U data = 0;

      auto r = br.template get_bits<U>(data, F::DataBitsPerGroup);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      if (accum > ACCUM_LIMIT) {
        return BitFieldResult::ErrorValueTooLarge;
      }

      accum = static_cast<U>((accum << F::DataBitsPerGroup) | (data & F::GroupDataMask));
      ++groups_read;
      break;
    }
  }

  if (accum > F::MaxCode)
    return BitFieldResult::ErrorValueTooLarge;

  return out.set_value(packing::unpack<T>(accum));
}

#endif  // SUB8_ENABLE_VARIABLE_LENGTH_FIELDS

#if SUB8_ENABLE_BOOL
SUB8_DECLARE_BITFIELD_ALIAS(BoolValueField, BasicValueField<bool, 1>);

// Custom read write implementation because BasicValueField dont have one defined
template<typename Storage> inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const BoolValueField &field) {
  return bw.put_bits(field.value() ? 1u : 0u, 1);
}

template<typename Storage> inline BitFieldResult read_field(BasicBitReader<Storage> &br, BoolValueField &out) {
  size_t tmp = 0;
  auto r = br.get_bits(tmp, 1);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  return out.set_value((tmp & 1u) != 0u);
}

#endif

// Unsigned integer fields can use the bitfield templates directly.
#if SUB8_ENABLE_UINT4
SUB8_DECLARE_BITFIELD_ALIAS(Uint4ValueField, FixedLengthBitField<uint8_t, /*DataBitsPerGroup=*/4>);
#endif

#if SUB8_ENABLE_UINT8
SUB8_DECLARE_BITFIELD_ALIAS(Uint8ValueField, FixedLengthBitField<uint8_t, /*DataBitsPerGroup=*/8>);
#endif

#if SUB8_ENABLE_UINT16
SUB8_DECLARE_BITFIELD_ALIAS(Uint16ValueField, FixedLengthBitField<uint16_t, /*DataBitsPerGroup=*/16>);
#endif

#if SUB8_ENABLE_UINT32
SUB8_DECLARE_BITFIELD_ALIAS(Uint32ValueField, FixedLengthBitField<uint32_t, /*DataBitsPerGroup=*/32>);
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT32
SUB8_DECLARE_BITFIELD_ALIAS(Uint32Pack8ValueField,
                            VariableLengthBitField<uint32_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 4>);
SUB8_DECLARE_BITFIELD_ALIAS(Uint32Pack16ValueField,
                            VariableLengthBitField<uint32_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 2>);
#endif

#if SUB8_ENABLE_UINT64
SUB8_DECLARE_BITFIELD_ALIAS(Uint64ValueField, FixedLengthBitField<uint64_t, /*DataBitsPerGroup=*/64>);
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_UINT64
SUB8_DECLARE_BITFIELD_ALIAS(Uint64Pack8ValueField,
                            VariableLengthBitField<uint64_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 8>);
SUB8_DECLARE_BITFIELD_ALIAS(Uint64Pack16ValueField,
                            VariableLengthBitField<uint64_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 4>);
SUB8_DECLARE_BITFIELD_ALIAS(Uint64Pack32ValueField,
                            VariableLengthBitField<uint64_t, /*DataBitsPerGroup=*/32, /* Max Groups */ 2>);

#endif

#if SUB8_ENABLE_INT8
SUB8_DECLARE_BITFIELD_ALIAS(Int8ValueField, FixedLengthBitField<int8_t, /*DataBitsPerGroup=*/8>);
#endif

#if SUB8_ENABLE_INT16
SUB8_DECLARE_BITFIELD_ALIAS(Int16ValueField, FixedLengthBitField<int16_t, /*DataBitsPerGroup=*/16>);
#endif

#if SUB8_ENABLE_INT32
SUB8_DECLARE_BITFIELD_ALIAS(Int32ValueField, FixedLengthBitField<int32_t, /*DataBitsPerGroup=*/32>);
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT32
SUB8_DECLARE_BITFIELD_ALIAS(Int32Pack8ValueField,
                            VariableLengthBitField<int32_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 4>);
SUB8_DECLARE_BITFIELD_ALIAS(Int32Pack16ValueField,
                            VariableLengthBitField<int32_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 2>);
#endif

#if SUB8_ENABLE_INT64
SUB8_DECLARE_BITFIELD_ALIAS(Int64ValueField, FixedLengthBitField<int64_t, /*DataBitsPerGroup=*/64>);
#endif

#if SUB8_ENABLE_VARIABLE_LENGTH_INT64
SUB8_DECLARE_BITFIELD_ALIAS(Int64Pack8ValueField,
                            VariableLengthBitField<int64_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 8>);
SUB8_DECLARE_BITFIELD_ALIAS(Int64Pack16ValueField,
                            VariableLengthBitField<int64_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 4>);
SUB8_DECLARE_BITFIELD_ALIAS(Int64Pack32ValueField,
                            VariableLengthBitField<int64_t, /*DataBitsPerGroup=*/32, /* Max Groups */ 2>);
#endif

#if SUB8_ENABLE_FLOAT16
SUB8_DECLARE_BITFIELD_ALIAS(Float16ValueField, FixedLengthFloatField<float, 5, 10>);
SUB8_DECLARE_BITFIELD_ALIAS(BFloat16ValueField, FixedLengthFloatField<float, 8, 7>);
#endif

#if SUB8_ENABLE_FLOAT24
SUB8_DECLARE_BITFIELD_ALIAS(Float24ValueField, FixedLengthFloatField<float, 7, 16>);
#endif

#if SUB8_ENABLE_FLOAT32
SUB8_DECLARE_BITFIELD_ALIAS(Float32ValueField, FixedLengthFloatField<float, 8, 23>);
#endif

#if SUB8_ENABLE_FLOAT48
SUB8_DECLARE_BITFIELD_ALIAS(Float48ValueField, FixedLengthFloatField<double, 11, 48>);
#endif

#if SUB8_ENABLE_FLOAT64
SUB8_DECLARE_BITFIELD_ALIAS(Float64ValueField, FixedLengthFloatField<double, 11, 52>);
#endif

}  // namespace sub8
