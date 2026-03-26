#pragma once

// Enable: Array Fields
#ifndef SUB8_ENABLE_ARRAY_FIELDS
#define SUB8_ENABLE_ARRAY_FIELDS 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_STL_TYPE 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_VECTOR
#define SUB8_ENABLE_STL_TYPE_VECTOR SUB8_ENABLE_STL_TYPE
#endif

#ifndef SUB8_ENABLE_STL_TYPE_ARRAY
#define SUB8_ENABLE_STL_TYPE_ARRAY SUB8_ENABLE_STL_TYPE
#endif

#ifndef SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#define SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST SUB8_ENABLE_STL_TYPE
#endif

#if SUB8_ENABLE_ARRAY_FIELDS

#include <cstdint> // uintx_t
#include <cstddef> // size_t
#include <utility> // std::declval
#include <type_traits>

#if SUB8_ENABLE_STL_TYPE_VECTOR
#include <vector>
#endif

#if SUB8_ENABLE_STL_TYPE_ARRAY
#include <array>
#endif

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#include <initializer_list>
#endif

#include <cstring> // std::memcmp, std::memcpy
#include <utility> // std::move


#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_api.h"
#include "sub8_errors.h"
#include "sub8_io.h"
#endif

namespace sub8 {

// `read` APIs
// --------------------------------

template <typename T, typename Storage, typename Elem> inline BitFieldResult read(BasicBitReader<Storage> &br, Elem *out, size_t len) {
  T f{};
  auto r = read_field(br, f);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  const auto seq = f.value();
  const size_t s = seq.size();
  const size_t m = (len < s) ? len : s;

  // Zero-fill any remaining destination elements if the source is shorter.
  for (size_t i = m; i < len; ++i) {
    out[i] = Elem{};
  }

  if (m == 0) {
    return BitFieldResult::Ok;
  }

  using Iter = decltype(seq.begin());
  using SrcElem = std::remove_cv_t<std::remove_reference_t<decltype(*std::declval<Iter>())>>;

  if constexpr (std::is_trivially_copyable_v<Elem> && std::is_trivially_copyable_v<SrcElem> && (sizeof(Elem) == sizeof(SrcElem))) {
    std::memcpy(static_cast<void *>(out), static_cast<const void *>(seq.begin()), m * sizeof(Elem));
    return BitFieldResult::Ok;
  } else {
    // Fallback: element-wise conversion.
    for (size_t i = 0; i < m; ++i) {
      out[i] = static_cast<Elem>(seq[i]);
    }
    return BitFieldResult::Ok;
  }
}

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult read(BasicBitReader<Storage> &br, Elem (&out)[N]) {
  return read<T>(br, out, N);
}

#if SUB8_ENABLE_STL_TYPE_ARRAY

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult read(BasicBitReader<Storage> &br,
  std::array<Elem, N> &out) {
  return read<T>(br, out.data(), N);
}

#endif

#if SUB8_ENABLE_STL_TYPE_VECTOR

template <typename T, typename Storage, typename Elem> inline BitFieldResult read(BasicBitReader<Storage> &br, std::vector<Elem> &out) {
  T f{};
  auto r = read_field(br, f);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  const auto seq = f.value();
  const size_t m = seq.size();

  out.clear();
  out.reserve(m);
  for (size_t i = 0; i < m; ++i) {
    out.push_back(static_cast<Elem>(seq[i]));
  }
  return BitFieldResult::Ok;
}

#endif

// `write` APIs
// --------------------------------

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult write(BasicBitWriter<Storage> &bw,
  const Elem (&in)[N]) noexcept {
  T f{};
  auto r = f.set_value(in, N);
  if (r != BitFieldResult::Ok) {
    return r;
  }
  return write_field(bw, f);
}

#if !SUB8_ENABLE_INFALLIBLE
template <typename T, typename Storage, typename Elem, size_t N> inline void write_or_throw(BasicBitWriter<Storage> &bw,
  const Elem (&in)[N]) {
  auto r = write<T>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, T,
      "sub8::write_or_throw<T>(BasicBitWriter<Storage>,"
      " const Elem (&in)[N])");
  }
}
#endif

#if SUB8_ENABLE_STL_TYPE_ARRAY

template <typename T, typename Storage, typename Elem, size_t N> inline BitFieldResult write(BasicBitWriter<Storage> &bw, const std::array<Elem, N> &in) noexcept {
  T f{};
  auto r = f.set_value(in.data(), N);
  if (r != BitFieldResult::Ok) {
    return r;
  }
  return write_field(bw, f);
}

#if !SUB8_ENABLE_INFALLIBLE
template <typename T, typename Storage, typename Elem, size_t N> inline void write_or_throw(BasicBitWriter<Storage> &bw, const std::array<Elem, N> &in) {
  auto r = write<T>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, T,
      "sub8::write_or_throw<T>(BasicBitWriter<Storage>,"
      " const std::array<Elem, N>)");
  }
}

#endif

#endif

#if SUB8_ENABLE_STL_TYPE_VECTOR

template <typename T, typename Storage, typename Elem> inline BitFieldResult write(BasicBitWriter<Storage> &bw,
  const std::vector<Elem> &in) noexcept {
  T f{};
  auto r = f.set_value(in.data(), in.size());
  if (r != BitFieldResult::Ok) {
    return r;
  }
  return write_field(bw, f);
}

#if !SUB8_ENABLE_INFALLIBLE
template <typename T, typename Storage, typename Elem> inline void write_or_throw(BasicBitWriter<Storage> &bw, const std::vector<Elem> &in) {
  auto r = write<T>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, T,
      "sub8::write_or_throw<T>(BasicBitWriter<Storage>,"
      " const std::vector<Elem>)");
  }
}
#endif

#endif

// Array view
template <typename T> struct BitIndexValue {
  const T *data_{nullptr};
  const size_t *size_ptr_{nullptr};

  const T *begin() const noexcept { return data_; }
  const T *end() const noexcept { return data_ + size(); }

  size_t size() const noexcept { return size_ptr_ ? *size_ptr_ : 0; }

  const T &operator[](size_t i) const noexcept { return data_[i]; }

  template <typename Seq> bool equals(const Seq &seq) const noexcept {
    const auto n = size();
    if (n != seq.size())
      return false;
    for (size_t i = 0; i < n; ++i) {
      if (data_[i] != seq[i])
        return false;
    }
    return true;
  }

  bool operator==(const BitIndexValue &o) const noexcept { return equals(o); }
  bool operator!=(const BitIndexValue &o) const noexcept { return !(*this == o); }

  template <typename Seq> bool operator==(const Seq &o) const noexcept { return equals(o); }
  template <typename Seq> bool operator!=(const Seq &o) const noexcept { return !(*this == o); }
};

// Array Field
// ----------------------

enum class ArrayEncoding : uint8_t { ThreePlusPrefixed = 0, Delimited, Prefixed };

template <uint8_t BitLength, bool Signed = false> struct PrimitiveElement {
  using Type = typename limits::numeric_for_bits<BitLength, Signed>::type;
  static constexpr bool IsObject = false;
  static constexpr uint8_t BitsPerElement = BitLength;

  static_assert(BitsPerElement >= 1, "Bits must be >= 1");

  using ElementLimits = sub8::limits::bits_limits<Type, BitsPerElement>;
  using UnderlyingType = typename ElementLimits::Integral;
  using StorageType = typename ElementLimits::UnsignedIntegral;

  static constexpr StorageType MaxCode = ElementLimits::MaxCode;
  static constexpr Type MinValue = ElementLimits::MinValue;
  static constexpr Type MaxValue = ElementLimits::MaxValue;
  static constexpr StorageType DataMask = static_cast<StorageType>(MaxCode);

  // True if we can pack (delimiter bit + data bits) into StorageType in one go.
  static constexpr bool FitsInStorage = (uint32_t(BitsPerElement) + 1u) <= uint32_t(sizeof(StorageType) * 8u);
};

template <typename T> struct ObjectElement {
  using Type = T;
  static constexpr bool IsObject = true;
  using UnderlyingType = T;
  using StorageType = T;
};

template <
  // TypeInfo:
  // Per-element type information (PrimitiveElement<...> or
  // ObjectElement<...>).
  typename TypeInfo,

  // TMinElements:
  // Minimum number of elements in the array.
  //
  // Delimited encoding:
  //  - The first TMinElements are written without continuation bits.
  //  - If fewer elements are provided, remaining elements are implicitly
  //    zero-padded and deserialize as default values.
  //
  // Prefixed encoding:
  //  - Reduces the number of bits required to encode the length prefix.
  //  - Missing elements are implicitly zero-padded and deserialize as default
  //  values.
  size_t TMinElements,

  // TMaxElements:
  // Maximum number of elements in the array.
  //
  // Prefixed encoding:
  //  - The array is prefixed with its length.
  //  - Length prefix uses the minimum number of bits required to represent
  //    the range [TMinElements, TMaxElements].
  //  - If TMinElements == TMaxElements, the length prefix is omitted.
  //
  // Delimited encoding:
  //  - Elements are written sequentially, each followed by a continuation
  //  bit.
  //  - If the array reaches TMaxElements, the termination bit is omitted,
  //    saving one bit when the maximum length is known at compile time.
  //
  // Example:
  //  - TMaxElements = 15 requires 4 bits to encode the length in prefixed
  //  mode.
  size_t TMaxElements,

  // TEncoding:
  // Array encoding strategy.
  //
  // Delimited:
  //  - Elements are terminated using continuation bits.
  //  - Most bit-efficient for very small or sparsely populated arrays
  //    (typically fewer than ~3 elements).
  //
  // Prefixed:
  //  - Array is prefixed with an explicit length value.
  //  - More efficient for larger arrays (3+ elements).
  //
  // ThreePlusPrefixed (default):
  //  - Uses Prefixed encoding when (TMaxElements - TMinElements) > 3,
  //    otherwise falls back to Delimited encoding.
  //  - Note: this decision is compile-time (template-based), not based on
  //  runtime length.
  ArrayEncoding TEncoding = ArrayEncoding::ThreePlusPrefixed>
class Array {
public:
  static_assert(TMaxElements >= TMinElements);

  using ElementType = TypeInfo;
  using Type = typename TypeInfo::Type;
  using ValueType = BitIndexValue<Type>;

  #if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
  using InitType = std::initializer_list<Type>;
  #endif

  using UnderlyingType = typename TypeInfo::UnderlyingType;
  using StorageType = typename TypeInfo::StorageType;

  static constexpr bool IsPrimitive = !TypeInfo::IsObject;
  static constexpr bool IsObject = TypeInfo::IsObject;

  static constexpr size_t MinElements = TMinElements;
  static constexpr size_t MaxElements = TMaxElements;
  static constexpr size_t ValueRange = TMaxElements - TMinElements;

  static constexpr ArrayEncoding Encoding = []() constexpr noexcept -> ArrayEncoding {
    if constexpr (TMaxElements == TMinElements) {
      return ArrayEncoding::Prefixed;
    }
    if constexpr (TEncoding != ArrayEncoding::ThreePlusPrefixed) {
      return TEncoding;
    }

    if constexpr (ValueRange <= 3) {
      return ArrayEncoding::Delimited;
    }
    return ArrayEncoding::Prefixed;
  }();

  static constexpr uint8_t LengthPrefixBitWidth = ValueRange <= 1u ? 1u : limits::bitwidth_to_express_max_value(ValueRange);

  static constexpr BitSize ElementMaxPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TypeInfo::IsObject) {
      return Type::MaxPossibleSize;
    } else {
      return BitSize::from_bits(TypeInfo::BitsPerElement);
    }
  }();

  static constexpr BitSize ElementMinPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TypeInfo::IsObject) {
      return Type::MinPossibleSize;
    } else {
      return BitSize::from_bits(TypeInfo::BitsPerElement);
    }
  }();

  static constexpr BitSize MaxPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TMaxElements == TMinElements) {
      return ElementMaxPossibleSize * TMaxElements;
    } else if constexpr (Encoding == ArrayEncoding::Delimited) {
      // Delimited mode uses (ValueRange - 1) continuation bits at most.
      return ElementMaxPossibleSize * TMaxElements + BitSize::from_bits(ValueRange);
    } else {
      // Prefixed mode uses a fixed-width length prefix.
      return ElementMaxPossibleSize * TMaxElements + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }();

  static constexpr BitSize MinPossibleSize = []() constexpr noexcept -> BitSize {
    if constexpr (TMaxElements == TMinElements) {
      return ElementMinPossibleSize * TMinElements;
    } else if constexpr (Encoding == ArrayEncoding::Delimited) {
      // Smallest delimiter overhead is always 1 bit (the terminator),
      // including the (TMinElements == 0 && n == 0) case.
      return ElementMinPossibleSize * TMinElements + BitSize::from_bits(1u);
    } else {
      // Prefixed mode uses a fixed-width length prefix.
      return ElementMinPossibleSize * TMinElements + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }();

  BitSize max_possible_size() const noexcept { return MaxPossibleSize; }
  BitSize min_possible_size() const noexcept { return MinPossibleSize; }
  BitSize actual_size() const noexcept {
    const size_t effective_n = (size_ < TMinElements) ? TMinElements : size_;
    BitSize elements_size = BitSize::from_bits(0);

    if constexpr (IsPrimitive) {
      // Primitive arrays: size is always bits-per-element * effective_n
      elements_size = BitSize::from_bits(TypeInfo::BitsPerElement) * effective_n;
    } else if constexpr (sub8::details_t::has_actual_size_v<Type>) {
      // Object with compile-time known element size
      elements_size = Type::ActualSize * effective_n;
    } else {
      // Object with runtime element sizes:
      // sum actual for present elements...
      for (size_t i = 0; i < size_; ++i) {
        elements_size += value_[i].actual_size();
      }
      // + pad up to effective_n with min size
      if (effective_n > size_) {
        // Note, this assumes default ctor will write MinPossibleSize in length
        elements_size += Type::MinPossibleSize * (effective_n - size_);
      }
    }

    if constexpr (TMaxElements == TMinElements) {
      // Fixed-length array: no length/termination overhead.
      return elements_size;

    } else if constexpr (Encoding == ArrayEncoding::Delimited) {
      // Delimited overhead depends on runtime length.
      size_t delimiter_bits = 0;

      if constexpr (TMinElements == 0) {
        if (effective_n == 0) {
          delimiter_bits = 1; // just terminator
        } else if (effective_n == TMaxElements) {
          delimiter_bits = ValueRange; // no terminator
        } else {
          delimiter_bits = effective_n + 1; // cont bits + terminator
        }
      } else {
        if (effective_n == TMaxElements) {
          delimiter_bits = ValueRange; // no terminator
        } else {
          delimiter_bits = (effective_n - TMinElements) + 1; // cont bits + terminator
        }
      }

      return elements_size + BitSize::from_bits(delimiter_bits);

    } else {
      // Prefixed: fixed length prefix overhead.
      return elements_size + BitSize::from_bits(LengthPrefixBitWidth);
    }
  }

private:
  static constexpr bool CanSkipPrimitiveRangeChecks =
    IsPrimitive && std::is_integral_v<Type> && std::is_unsigned_v<Type> && (TypeInfo::BitsPerElement >= (sizeof(Type) * 8u));

  template <typename U> static constexpr bool CanMemcpy = std::is_same_v<std::remove_cv_t<U>, Type> && std::is_trivially_copyable_v<Type>;

public:
  Array() noexcept = default;
  Array(const Array &) noexcept = default;

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#if !SUB8_ENABLE_INFALLIBLE
  Array(std::initializer_list<Type> init) SUB8_OPT_NO_EXCEPT {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, Array, "sub8::Array(std::initializer_list<Type>)");
  }
#endif
#endif

  template <typename U> SUB8_NO_DISCARD static BitFieldResult make(const U *seq, size_t len, Array &out) noexcept {
    return out.set_value(seq, len);
  }

#if !SUB8_ENABLE_INFALLIBLE
  template <typename U> static Array make_or_throw(const U *seq, size_t len) SUB8_OPT_NO_EXCEPT {
    Array out{};
    auto r = out.set_value(seq, len);
    SUB8_THROW_IF_ERROR(r, Array, "Array::make_or_throw(ptr,len)");
    return out;
  }

#endif

  template <typename Seq> SUB8_NO_DISCARD static BitFieldResult make(const Seq &seq, Array &out) { return out.set_value(seq); }


#if !SUB8_ENABLE_INFALLIBLE
  template <typename Seq> static Array make_or_throw(const Seq &seq) SUB8_OPT_NO_EXCEPT {
    Array out{};
    auto r = out.set_value(seq);
    SUB8_THROW_IF_ERROR(r, Array, "Array::make_or_throw(seq)");
    return out;
  }
#endif

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST

  SUB8_NO_DISCARD static BitFieldResult make(std::initializer_list<Type> init, Array &out) noexcept { return out.set_value(init); }

#if !SUB8_ENABLE_INFALLIBLE

  static Array make_or_throw(std::initializer_list<Type> init) SUB8_OPT_NO_EXCEPT {
    Array out{};
    auto r = out.set_value(init);
    SUB8_THROW_IF_ERROR(r, Array, "Array::make_or_throw(init_list)");
    return out;
  }
#endif

#endif

  bool empty() const noexcept { return size_ == 0; }
  size_t size() const noexcept { return size_; }
  const Type &operator[](size_t i) const noexcept { return value_[i]; }

  void clear() noexcept { size_ = 0; }

  BitIndexValue<Type> value() const noexcept { return BitIndexValue<Type>{value_, &size_}; }

  explicit operator BitIndexValue<Type>() const noexcept { return value(); }

  bool operator==(const Array &o) const noexcept {
    if (size_ != o.size_)
      return false;

    if constexpr (!IsObject && std::is_trivially_copyable_v<Type> && std::has_unique_object_representations_v<Type>) {
      return std::memcmp(value_, o.value_, size_ * sizeof(Type)) == 0;
    } else {
      for (size_t i = 0; i < size_; ++i) {
        if (!(value_[i] == o.value_[i]))
          return false;
      }
      return true;
    }
  }

  bool operator!=(const Array &o) const noexcept { return !(*this == o); }
  Array &operator=(const Array &) noexcept = default;

  #if !SUB8_ENABLE_INFALLIBLE
  template <typename Seq> Array &operator=(const Seq &seq) {
    auto r = set_value(seq);
    SUB8_THROW_IF_ERROR(r, Array, "Array::operator=(const Seq &)");
    return *this;
  }
  #endif

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#if !SUB8_ENABLE_INFALLIBLE
  Array &operator=(std::initializer_list<Type> init) {
    auto r = set_value(init);
    SUB8_THROW_IF_ERROR(r, Array, "Array::operator=(std::initializer_list<Type>)");
    return *this;
  }
#endif
#endif

  template <typename Seq> SUB8_NO_DISCARD BitFieldResult set_value(const Seq &seq) {
    // Note Seq doesn't guarantee noexcept, so this method can not be noexcept
    clear();
    const size_t n = seq.size();
    if (n > TMaxElements)
      return BitFieldResult::ErrorTooManyElements;

    for (size_t i = 0; i < n; ++i) {
      auto r = push_back(static_cast<Type>(seq[i]));
      if (r != BitFieldResult::Ok)
        return r;
    }
    return BitFieldResult::Ok;
  }

  template <typename U> SUB8_NO_DISCARD BitFieldResult set_value(const U *seq, size_t len) noexcept {
    clear();
    if (seq == nullptr) {
      return (len == 0) ? BitFieldResult::Ok : BitFieldResult::ErrorInvalidBitFieldValue;
    }
    if (len > TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    // Fast path: exact type match and trivially copyable.
    // Only allowed if we don't need per-element validation.
    if constexpr (CanMemcpy<U>) {
      if constexpr (IsObject) {
        std::memcpy(value_, seq, len * sizeof(Type));
        size_ = len;
        return BitFieldResult::Ok;
      } else {
        // Primitive arrays:
        // If all values fit in Type, we can skip per-element checks.
        // Otherwise we must validate each element (even if memcpy would be
        // possible).
        if constexpr (CanSkipPrimitiveRangeChecks) {
          std::memcpy(value_, seq, len * sizeof(Type));
          size_ = len;
          return BitFieldResult::Ok;
        }
      }
    }

    for (size_t i = 0; i < len; ++i) {
      auto r = push_back(static_cast<Type>(seq[i]));
      if (r != BitFieldResult::Ok) {
        return r;
      }
    }
    return BitFieldResult::Ok;
  }

#if SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST

  SUB8_NO_DISCARD BitFieldResult set_value(std::initializer_list<Type> init) noexcept { return set_value(init.begin(), init.size()); }

#endif

// TODO: needs changes to be   #if !SUB8_ENABLE_INFALLIBLE
  SUB8_NO_DISCARD BitFieldResult push_back(const Type &v) noexcept(std::is_nothrow_copy_assignable_v<Type>) {
    if constexpr (IsPrimitive) {
      if (v < ElementType::MinValue || v > ElementType::MaxValue) {
        return BitFieldResult::ErrorValueOverflow;
      }
    }
    if (size_ >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    value_[size_++] = v;
    return BitFieldResult::Ok;
  }

  SUB8_NO_DISCARD BitFieldResult push_back(Type &&v) noexcept(std::is_nothrow_move_assignable_v<Type>) {
    if constexpr (IsPrimitive) {
      if (v < ElementType::MinValue || v > ElementType::MaxValue) {
        return BitFieldResult::ErrorValueOverflow;
      }
    }
    if (size_ >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }
    value_[size_] = std::move(v); // Type could throw exception here
    size_++;                      // increment after move, just incase exception is thrown
    return BitFieldResult::Ok;
  }

private:
  size_t size_{0};
  Type value_[TMaxElements]{};

};

// Delimited read / write implementation

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
write_delimited_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, Encoding> &p) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  using F = Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  const size_t n = p.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : n;
  if (effective_n > TMaxElements) {
    return BitFieldResult::ErrorTooManyElements;
  }

  auto write_prefixed_value = [&](Type v) -> BitFieldResult {
    // Note: the delimiter bit is the MSB of the encoded group.
    if constexpr (F::IsObject) {
      auto r = bw.template put_bits<uint8_t>(1u, 1);
      if (r != BitFieldResult::Ok)
        return r;
      return write_field(bw, v);
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);

      if constexpr (TypeInfo::FitsInStorage) {
        const StorageType masked = code & TypeInfo::DataMask;
        const StorageType group = (StorageType{1} << TypeInfo::BitsPerElement) | masked;
        return bw.template put_bits<StorageType>(group, uint32_t(TypeInfo::BitsPerElement) + 1u);
      } else {
        auto r = bw.template put_bits<uint8_t>(1u, 1);
        if (r != BitFieldResult::Ok)
          return r;
        return bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
      }
    }
  };

  auto write_non_prefixed_value = [&](Type v) -> BitFieldResult {
    if constexpr (F::IsObject) {
      return write_field(bw, v);
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);
      if (code > TypeInfo::MaxCode) {
        return BitFieldResult::ErrorValueOverflow;
      }
      return bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
    }
  };

  size_t i = 0;

  if constexpr (TMinElements == 0) {
    if (n == 0) {
      return bw.template put_bits<uint8_t>(0u, 1);
    }
  } else {
    // Write the first MinElements without delimiter bits.
    // MinElements are always written; if elements are missing they are written
    // as Type{} (i.e. zero-padded / default-constructed).
    for (; i < TMinElements; ++i) {
      const Type v = (i < n) ? p[i] : Type{};
      auto r = write_non_prefixed_value(v);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  // Write remaining elements with a delimiter (continuation) bit.
  for (; i < effective_n; ++i) {
    const Type v = (i < n) ? p[i] : Type{};
    auto r = write_prefixed_value(v);
    if (r != BitFieldResult::Ok)
      return r;
  }

  // Write a terminator unless the array is exactly at max length (termination
  // is implied).
  if (n != TMaxElements) {
    return bw.template put_bits<uint8_t>(0u, 1);
  }
  return BitFieldResult::Ok;
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
read_delimited_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, Encoding> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  out.clear();

  auto read_non_prefixed_value = [&](Type &v) -> BitFieldResult {
    if constexpr (F::IsObject) {
      return read_field(br, v);
    } else {
      StorageType raw = 0;
      auto r = br.template get_bits<StorageType>(raw, uint32_t(TypeInfo::BitsPerElement));
      if (r != BitFieldResult::Ok)
        return r;

      const StorageType code = StorageType(raw & TypeInfo::DataMask);
      v = sub8::packing::unpack<Type>(code);
      return BitFieldResult::Ok;
    }
  };

  auto read_prefixed_value = [&](Type &v) -> BitFieldResult {
    // Continuation bit was already consumed; the next bits are the value.
    if constexpr (F::IsObject) {
      return read_field(br, v);
    } else {
      StorageType raw = 0;
      auto r = br.template get_bits<StorageType>(raw, uint32_t(TypeInfo::BitsPerElement));
      if (r != BitFieldResult::Ok)
        return r;

      const StorageType code = StorageType(raw & TypeInfo::DataMask);
      v = sub8::packing::unpack<Type>(code);
      return BitFieldResult::Ok;
    }
  };

  // Read fixed (non-delimited) minimum portion.
  if constexpr (TMinElements != 0) {
    while (out.size() < TMinElements) {
      Type v{};
      auto r = read_non_prefixed_value(v);
      if (r != BitFieldResult::Ok)
        return r;

      r = out.push_back(std::move(v));
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  // Read delimited portion: [cont][value] ... terminates when cont == 0.
  uint8_t cont = 0;
  auto r = br.template get_bits<uint8_t>(cont, 1);
  if (r != BitFieldResult::Ok)
    return r;

  if (cont == 0) {
    return BitFieldResult::Ok;
  }

  while (true) {
    if (out.size() >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    Type v{};
    auto r = read_prefixed_value(v);
    if (r != BitFieldResult::Ok)
      return r;

    r = out.push_back(std::move(v));
    if (r != BitFieldResult::Ok)
      return r;

    // If we're at max, the trailing terminator bit is implied.
    if (out.size() == TMaxElements) {
      return BitFieldResult::Ok;
    }

    r = br.template get_bits<uint8_t>(cont, 1);
    if (r != BitFieldResult::Ok)
      return r;

    if (cont == 0) {
      return BitFieldResult::Ok;
    }
  }
}

// Prefixed read / write implementation

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
write_prefixed_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, Encoding> &f) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));

  using F = const Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  if (f.size() > TMaxElements) {
    return BitFieldResult::ErrorTooManyElements;
  }

  const size_t n = f.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : f.size(); // Zero-pad up to MinElements.

  // Omit writing the length prefix if TMaxElements == TMinElements.
  if constexpr (TMaxElements != TMinElements) {
    auto r = bw.template put_bits<size_t>(effective_n - TMinElements, F::LengthPrefixBitWidth);
    if (r != BitFieldResult::Ok)
      return r;
  }

  auto values = f.value();

  for (size_t i = 0; i < effective_n; ++i) {
    const Type v = (i < n) ? values[i] : Type{};

    if constexpr (!F::IsPrimitive) {
      auto r = write_field(bw, v);
      if (r != BitFieldResult::Ok)
        return r;
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);
      if (code > TypeInfo::MaxCode)
        return BitFieldResult::ErrorValueOverflow;

      auto r = bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  return BitFieldResult::Ok;
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding> inline BitFieldResult
read_prefixed_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, Encoding> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  using F = Array<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  out.clear();

  // Read length prefix.
  size_t n = 0;

  if constexpr (TMaxElements == TMinElements) {
    n = TMaxElements;
  } else {
    auto r = br.template get_bits<size_t>(n, F::LengthPrefixBitWidth);
    if (r != BitFieldResult::Ok)
      return r;

    n = n + TMinElements;

    if (n > TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }
  }

  // Read elements.
  for (size_t i = 0; i < n; ++i) {
    if constexpr (!F::IsPrimitive) {
      Type inner{};
      auto r = read_field(br, inner);
      if (r != BitFieldResult::Ok)
        return r;

      r = out.push_back(inner);
      if (r != BitFieldResult::Ok)
        return r;
    } else {
      StorageType code = 0;

      auto r = br.template get_bits<StorageType>(code, TypeInfo::BitsPerElement);
      if (r != BitFieldResult::Ok)
        return r;

      if (code > TypeInfo::MaxCode) {
        return BitFieldResult::ErrorValueOverflow;
      }

      r = out.push_back(sub8::packing::unpack<Type>(code));
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  return BitFieldResult::Ok;
}

// Delimited read / write
template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Delimited> &p) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  return write_delimited_field(bw, p);
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Delimited> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  return read_delimited_field(br, out);
}

// Prefixed read / write
template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
read_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Prefixed> &out) noexcept {
  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));
  return read_prefixed_field(br, out);
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Prefixed> &p) noexcept {
  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  return write_prefixed_field(bw, p);
}

// ThreePlusPrefixed read / write
template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> SUB8_NO_DISCARD inline BitFieldResult
write_field(BasicBitWriter<Storage> &bw, const Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::ThreePlusPrefixed> &p) noexcept {

  static_assert(noexcept(std::declval<BasicBitWriter<Storage>&>().put_bits(uint32_t{}, uint8_t{})));
  if constexpr (TMaxElements == TMinElements) {
    // Use the prefixed implementation for fixed field sizes.
    // While both methods can support it, behavior is more stable if only one
    // implementation is used.
    return write_prefixed_field(bw, p);
  } else if constexpr ((TMaxElements - TMinElements) <= 3) {
    return write_delimited_field(bw, p);
  } else {
    return write_prefixed_field(bw, p);
  }
}

template <typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements> 
SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br, Array<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::ThreePlusPrefixed> &out) noexcept {

  static_assert(noexcept(std::declval<BasicBitReader<Storage>&>().get_bits(std::declval<uint32_t&>(), uint8_t{})));

  if constexpr (TMaxElements == TMinElements) {
    // Use the prefixed implementation for fixed field sizes.
    // While both methods can support it, behavior is more stable if only one
    // implementation is used.
    return read_prefixed_field(br, out);
  } else if constexpr ((TMaxElements - TMinElements) <= 3) {
    return read_delimited_field(br, out);
  } else {
    return read_prefixed_field(br, out);
  }
}

// Object Array
template <typename T, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using ObjectArray =
  Array<ObjectElement<T>, TMinElements, TMaxElements, Encoding>;

template <uint8_t BitWidth, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using BufferArray =
  Array<PrimitiveElement<BitWidth>, TMinElements, TMaxElements, Encoding>;

template <uint8_t BitWidth, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed>
using SignedBufferArray = Array<PrimitiveElement<BitWidth, /*Signed*/ true>, TMinElements, TMaxElements, Encoding>;

template <typename T, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using FixedSizeObjectArray =
  Array<ObjectElement<T>, TMaxElements, TMaxElements, Encoding>;

template <uint8_t BitWidth, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed> using FixedSizeBufferArray =
  Array<PrimitiveElement<BitWidth>, TMaxElements, TMaxElements, Encoding>;

} // namespace sub8

#endif // SUB8_ENABLE_ARRAY_FIELDS
