#pragma once

// Enable: Path Fields
// Strings of values encoded by a continuation bit.
// Will be more size efficient for item lengths shorter then 5
#ifndef SUB8_ENABLE_PATH_FIELDS
#define SUB8_ENABLE_PATH_FIELDS 1
#endif

// Enable: Array Fields
// Strings of values encoded by a prefixed length indicator
// NOTE: Use this for lengths longer then 5, otherwise Paths will ways be more size efficient
#ifndef SUB8_ENABLE_ARRAY_FIELDS
#define SUB8_ENABLE_ARRAY_FIELDS 1
#endif


#include <initializer_list>
#include <cstring>  // for memcmp

#include "sub8_basic.h"
#include "sub8_errors.h"
#include "sub8_io.h"

namespace sub8 {

// `make` collection Field APIs
// --------------------------------

template<typename TCollectionFeildType, typename Elem, size_t N>
inline BitFieldResult make(const Elem (&t_val)[N], TCollectionFeildType &out) {
  TCollectionFeildType tmp{};
  auto r = tmp.set_value(t_val, N);
  if (r == BitFieldResult::Ok) {
    out = tmp;
  }
  return r;
}

template<typename TCollectionFeildType, typename Elem, size_t N>
inline BitFieldResult make(const std::array<Elem, N> &in, TCollectionFeildType &out) {
  TCollectionFeildType tmp{};
  auto r = tmp.set_value(in.data(), static_cast<size_t>(in.size()));
  if (r == BitFieldResult::Ok) {
    out = tmp;
  }
  return r;
}

template<typename TCollectionFeildType, typename Elem>
inline BitFieldResult make(const std::vector<Elem> &in, TCollectionFeildType &out) {
  TCollectionFeildType tmp{};
  auto r = tmp.set_value(in.data(), static_cast<size_t>(in.size()));
  if (r == BitFieldResult::Ok) {
    out = tmp;
  }
  return r;
}

// `read` collection Field APIs
// --------------------------------

template<typename T, typename Storage, typename Elem>
inline BitFieldResult read(BasicBitReader<Storage> &br, Elem *out, size_t len) {
  T f{};
  auto r = read_field(br, f);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  const auto seq = f.value();
  const size_t m = std::min(len, seq.size());

  for (size_t i = m; i < len; ++i) {
    out[i] = Elem{};
  }

  if (m == 0) {
    return BitFieldResult::Ok;
  }

  // Try memcpy when it's safe/meaningful.
  // We infer the source element type from what begin() returns.
  using SrcElem = std::remove_cv_t<std::remove_pointer_t<decltype(seq.begin())>>;

  if constexpr (std::is_trivially_copyable_v<Elem> && std::is_trivially_copyable_v<SrcElem> &&
                (sizeof(Elem) == sizeof(SrcElem))) {
    std::memcpy(static_cast<void *>(out), static_cast<const void *>(seq.begin()), m * sizeof(Elem));
    return BitFieldResult::Ok;
  } else {
    // Fallback: element-wise conversion
    for (size_t i = 0; i < m; ++i) {
      out[i] = static_cast<Elem>(seq[i]);
    }
    return BitFieldResult::Ok;
  }
}

template<typename T, typename Storage, typename Elem, size_t N>
inline BitFieldResult read(BasicBitReader<Storage> &br, Elem (&out)[N]) {
  return sub8::read<T>(br, out, N);
}

template<typename T, typename Storage, typename Elem, size_t N>
inline BitFieldResult read(BasicBitReader<Storage> &br, std::array<Elem, N> &out) {
  return sub8::read<T>(br, out.data(), N);
}

template<typename T, typename Storage, typename Elem>
inline BitFieldResult read(BasicBitReader<Storage> &br, std::vector<Elem> &out) {
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

// `write` collection Field APIs
// --------------------------------
template<typename T, typename Storage, typename Elem, size_t N>
inline BitFieldResult write(BasicBitWriter<Storage> &bw, const Elem (&in)[N]) {
  T f{};
  auto r = make<T>(in, f);
  if (r != BitFieldResult::Ok)
    return r;
  return write_field(bw, f);
}

template<typename TFeildType, typename Storage, typename Elem, size_t N>
inline BitFieldResult write(BasicBitWriter<Storage> &bw, const std::array<Elem, N> &in) {
  TFeildType f{};
  auto r = f.set_value(in.data(), N);
  if (r != BitFieldResult::Ok)
    return r;
  return write_field(bw, f);
}

template<typename TFeildType, typename Storage, typename Elem>
inline BitFieldResult write(BasicBitWriter<Storage> &bw, const std::vector<Elem> &in) {
  TFeildType f{};
  auto r = f.set_value(in.data(), in.size());
  if (r != BitFieldResult::Ok)
    return r;
  return write_field(bw, f);
}

template<typename TFeildType, typename Storage, typename Elem, size_t N>
inline void write_or_throw(BasicBitWriter<Storage> &br, const std::array<Elem, N> &in) {
  auto r = write<TFeildType>(br, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
                              "sub8::write_or_throw<TFeildType>(BasicBitWriter<Storage>, const std::array<Elem, N>)");
  }
}

template<typename TFeildType, typename Storage, typename Elem, size_t N>
inline void write_or_throw(BasicBitWriter<Storage> &bw, const Elem (&in)[N]) {
  auto r = write<TFeildType>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
                              "sub8::write_or_throw<TFeildType>(BasicBitWriter<Storage>, const Elem (&in)[N])");
  }
}

template<typename TFeildType, typename Storage, typename Elem>
inline void write_or_throw(BasicBitWriter<Storage> &bw, const std::vector<Elem> &in) {
  auto r = write<TFeildType>(bw, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
                              "sub8::write_or_throw<TFeildType>(BasicBitWriter<Storage>, const std::vector<Elem>)");
  }
}

// Array view
template<typename T> struct BitIndexValue {
  const T *data_{nullptr};
  const size_t *size_ptr_{nullptr};

  const T *begin() const noexcept { return data_; }
  const T *end() const noexcept { return data_ + size(); }

  size_t size() const noexcept { return size_ptr_ ? *size_ptr_ : 0; }

  const T &operator[](size_t i) const noexcept { return data_[i]; }

  template<typename Seq> bool equals(const Seq &seq) const noexcept {
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

  template<typename Seq> bool operator==(const Seq &o) const noexcept { return equals(o); }
  template<typename Seq> bool operator!=(const Seq &o) const noexcept { return !(*this == o); }
};

// Array Field
// ----------------------
#if SUB8_ENABLE_ARRAY_FIELDS

enum class ArrayEncoding : uint8_t {
  ThreePlusPrefixed = 0,
  Delimited,
  Prefixed
};

template<typename T, uint8_t TBitsPerElement>
struct PrimitiveElement {
  using Type = T;
  static constexpr bool IsObject = false;
  static constexpr uint8_t BitsPerElement = TBitsPerElement;

  static_assert(BitsPerElement >= 1, "Bits must be >= 1");

  using ElementLimits = sub8::limits::bits_limits<T, BitsPerElement>;
  using UnderlyingType = typename ElementLimits::UnderlyingType;
  using StorageType    = typename ElementLimits::StorageType;

  static constexpr StorageType MaxCode = ElementLimits::MaxCode;
  static constexpr Type MinValue = ElementLimits::MinValue;
  static constexpr Type MaxValue = ElementLimits::MaxValue;
  static constexpr StorageType DataMask = static_cast<StorageType>(MaxCode);
  static constexpr bool FitsInStorage = (uint32_t(BitsPerElement) + 1u) <= uint32_t(sizeof(StorageType) * 8u);
};

template<typename T>
struct ObjectElement {
  using Type = T;
  static constexpr bool IsObject = true;
  using UnderlyingType = T;
  using StorageType    = T;
};


template<
    // T:
    // C++ value type used to represent each element.
    // Must be able to represent at least TBitsPerElement bits.
    typename TypeInfo,

    // TMinElements:
    // Minimum number of elements in the array.
    //
    // Delimited encoding:
    //  - The first TMinElements are written without continuation bits.
    //  - If fewer elements are provided, remaining elements are implicitly
    //    zero-padded and deserialize as zero.
    //
    // Prefixed encoding:
    //  - Reduces the number of bits required to encode the length prefix.
    //  - Missing elements are implicitly zero-padded and deserialize as zero.
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
    //  - Elements are written sequentially, each followed by a continuation bit.
    //  - If the array reaches TMaxElements, the final continuation bit is omitted,
    //    saving one bit when the maximum length is known at compile time.
    //
    // Example:
    //  - TMaxElements = 15 requires 4 bits to encode the length in prefixed mode.
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
    //  - Note: this is determined at compile time based on templates not the actual array length
    ArrayEncoding TEncoding = ArrayEncoding::ThreePlusPrefixed
>
class ArrayBitField {
 public:
  static_assert(TMaxElements >= TMinElements);

  using ElementType = TypeInfo;
  using Type = typename TypeInfo::Type;
  using ValueType = BitIndexValue<Type>;
  using InitType = std::initializer_list<Type>;
  using UnderlyingType = typename TypeInfo::UnderlyingType;
  using StorageType = typename TypeInfo::StorageType;

  static constexpr bool IsPrimitive = !TypeInfo::IsObject;
  static constexpr bool IsObject = TypeInfo::IsObject;

  static constexpr ArrayEncoding Encoding = TEncoding;
  static constexpr size_t MinElements = TMinElements;
  static constexpr size_t MaxElements = TMaxElements;
  static constexpr size_t ValueRange = TMaxElements - TMinElements;

  static constexpr uint8_t calc_required_len() noexcept {
    size_t v = ValueRange;
    uint8_t bits = 0;
    while (v > 0) {
      v >>= 1;
      ++bits;
    }
    return bits ? bits : uint8_t{1};
  }

  static constexpr uint8_t LengthBits = calc_required_len();

  private:
  size_t size_{0};
  Type value_[TMaxElements]{};

  public:

  ArrayBitField() noexcept = default;
  ArrayBitField(const ArrayBitField &) noexcept = default;

  ArrayBitField(std::initializer_list<Type> init) noexcept { set_value(init); }

  bool empty() const noexcept { return size_ == 0; }
  size_t size() const noexcept { return size_; }
  const Type &operator[](size_t i) const noexcept { return value_[i]; }

  void clear() noexcept { size_ = 0; }

  BitIndexValue<Type> value() const noexcept { return BitIndexValue<Type>{value_, &size_}; }
  explicit operator BitIndexValue<Type>() const noexcept { return value(); }

  bool operator==(const ArrayBitField& o) const noexcept {
    if (size_ != o.size_) 
      return false;

    if constexpr (!IsObject && std::is_trivially_copyable_v<Type> && std::has_unique_object_representations_v<Type>) {
        return std::memcmp(value_, o.value_, size_ * sizeof(Type)) == 0;
    } else {
        for (size_t i = 0; i < size_; ++i) {
          if (!(value_[i] == o.value_[i])) return false;
        }
        return true;
    }
  }


  bool operator!=(const ArrayBitField &o) const noexcept { return !(*this == o); }
  ArrayBitField &operator=(const ArrayBitField &) noexcept = default;

  template<typename Seq>
  ArrayBitField &operator=(const Seq &seq) noexcept {
    auto r = set_value(seq);
    assert(r == BitFieldResult::Ok);
    (void)r;
    return *this;
  }

  ArrayBitField &operator=(std::initializer_list<Type> init) noexcept {
    auto r = set_value(init);
    assert(r == BitFieldResult::Ok);
    (void)r;
    return *this;
  }

  template<typename Seq>
  BitFieldResult set_value(const Seq &seq) {
    clear();
    const size_t n = seq.size();
    if (n > TMaxElements) return BitFieldResult::ErrorTooManyElements;

    for (size_t i = 0; i < n; ++i) {
      auto r = push_back(static_cast<Type>(seq[i]));
      if (r != BitFieldResult::Ok) return r;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(const Type *seq, size_t len) {
    clear();
    if (seq == nullptr) {
      return (len == 0) ? BitFieldResult::Ok : BitFieldResult::ErrorInvalidBitFieldValue;
    }
    if (len > TMaxElements) return BitFieldResult::ErrorTooManyElements;

    for (size_t i = 0; i < len; ++i) {
      auto r = push_back(static_cast<Type>(seq[i]));
      if (r != BitFieldResult::Ok) return r;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(std::initializer_list<Type> init) {
    struct View {
      const Type *ptr;
      size_t n;
      size_t size() const noexcept { return n; }
      const Type &operator[](size_t i) const noexcept { return ptr[i]; }
    };
    return set_value(View{init.begin(), init.size()});
  }

  BitFieldResult push_back(const Type& v) {
    if constexpr (IsPrimitive) {
      if (v < ElementType::MinValue || v > ElementType::MaxValue) {
        return BitFieldResult::ErrorValueTooLarge;
      }
    }
    if (size_ >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    value_[size_++] = v;
    return BitFieldResult::Ok;
  }

  BitFieldResult push_back(Type&& v) {
    if constexpr (IsPrimitive) {
      if (v < ElementType::MinValue || v > ElementType::MaxValue) {
        return BitFieldResult::ErrorValueTooLarge;
      }
    }
    if (size_ >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    value_[size_++] = std::move(v);
    return BitFieldResult::Ok;
  }

};


// Delimited read / write Impl

template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding>
inline BitFieldResult write_delimited_field(
    BasicBitWriter<Storage> &bw,
    const ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding> &p
) {
  using F = ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  const size_t n = p.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : n;
  if (effective_n > TMaxElements) {
    return BitFieldResult::ErrorTooManyElements;
  }

  auto write_prefixed_value = [&](Type v) -> BitFieldResult {

    // Note the delimiter bit is the MSB.

    if constexpr (F::IsObject) {
      auto r = bw.template put_bits<uint8_t>(1u, 1);
      if (r != BitFieldResult::Ok) return r;
      return write_field(bw, v);
    } else {
      const StorageType code = sub8::packing::pack<Type>(v);

      if constexpr (TypeInfo::FitsInStorage) {
        const StorageType masked = code & TypeInfo::DataMask;
        const StorageType group  = (StorageType{1} << TypeInfo::BitsPerElement) | masked;
        return bw.template put_bits<StorageType>(group, uint32_t(TypeInfo::BitsPerElement) + 1u);
      } else {
        auto r = bw.template put_bits<uint8_t>(1u, 1);
        if (r != BitFieldResult::Ok) return r;
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
        return BitFieldResult::ErrorValueTooLarge;
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
    // Write out elements less then MinElements without a delimiter 
    // Note that MinElements is always written, if elements are missing they
    // are written out as Type{}, ie null padded or default constructed
    for (; i < TMinElements; ++i) {
      const Type v = (i < n) ? p[i] : Type{};
      auto r = write_non_prefixed_value(v);
      if (r != BitFieldResult::Ok) return r;
    }
  }

  // Write out remaining elements with a delimiter 
  for (; i < effective_n; ++i) {
    const Type v = (i < n) ? p[i] : Type{};
    auto r = write_prefixed_value(v);
    if (r != BitFieldResult::Ok) return r;
  }

  // Write terminator unless the array is exactly at max length then termination is implied 
  if (n != TMaxElements) {
    return bw.template put_bits<uint8_t>(0u, 1);
  }
  return BitFieldResult::Ok;
}

template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding>
inline BitFieldResult read_delimited_field(
    BasicBitReader<Storage> &br,
    ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding> &out
) {
  using F = ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  out.clear();

  auto read_non_prefixed_value = [&](Type &v) -> BitFieldResult {
    if constexpr (F::IsObject) {
      return read_field(br, v);
    } else {
      StorageType raw = 0;
      auto r = br.template get_bits<StorageType>(raw, uint32_t(TypeInfo::BitsPerElement));
      if (r != BitFieldResult::Ok) {
        return r;
      }
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
      if (r != BitFieldResult::Ok) {
        return r;
      }
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
      if (r != BitFieldResult::Ok) return r;

      r = out.push_back(std::move(v));
      if (r != BitFieldResult::Ok) return r;
    }
  }

  // Read delimited portion: [cont][value] ... terminates when cont == 0.
  uint8_t cont = 0;
  auto r = br.template get_bits<uint8_t>(cont, 1);
  if (r != BitFieldResult::Ok) {
    return r;
  }

  if (cont == 0) {
    return BitFieldResult::Ok;
  }

  while (true) {
    if (out.size() >= TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    Type v{};
    auto r = read_prefixed_value(v);
    if (r != BitFieldResult::Ok) return r;

    r = out.push_back(std::move(v));
    if (r != BitFieldResult::Ok) return r;

    // If we're at max, the trailing terminator bit is implied 
    if (out.size() == TMaxElements) {
      return BitFieldResult::Ok;
    }

    r = br.template get_bits<uint8_t>(cont, 1);
    if (r != BitFieldResult::Ok) {
      return r;
    }
    if (cont == 0) {
      return BitFieldResult::Ok;
    }
  }
}

// Prefixed read / write Impl
template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding>
inline BitFieldResult write_prefixed_field(BasicBitWriter<Storage> &bw, const ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding> &f) {
  using F = const ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;
  if (f.size() > TMaxElements)
    return BitFieldResult::ErrorTooManyElements;

  const size_t n = f.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : f.size(); // Expect zero-pad up to min

  // Omit writing length prefix if TMaxElements == TMinElements
  if constexpr (TMaxElements != TMinElements) {
    auto r = bw.template put_bits<size_t>(effective_n - TMinElements, F::LengthBits);
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
        return BitFieldResult::ErrorValueTooLarge;

      auto r = bw.template put_bits<StorageType>(code, TypeInfo::BitsPerElement);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }
  return BitFieldResult::Ok;
}


template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding>
inline BitFieldResult read_prefixed_field(BasicBitReader<Storage> &br, ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding> &out) {
  using F = ArrayBitField<TypeInfo, TMinElements, TMaxElements, Encoding>;
  using StorageType = typename F::StorageType;
  using Type = typename F::Type;

  out.clear();

  // Read length prefix
  size_t n = 0;

  if constexpr (TMaxElements == TMinElements) {
    n = TMaxElements;
  } else {

    auto r = br.template get_bits<size_t>(n, F::LengthBits);
    if (r != BitFieldResult::Ok) {
      return r;
    }

    n = n + TMinElements;

    if (n > TMaxElements)
      return BitFieldResult::ErrorTooManyElements;
  }

  // Read elements
  for (size_t i = 0; i < n; ++i) {

    if constexpr (!F::IsPrimitive) {
      Type inner {};
      auto r = read_field(br, inner);
      if (r != BitFieldResult::Ok) {
        return r;
      }
      r = out.push_back(inner);
      if (r != BitFieldResult::Ok) {
        return r;
      }
    } else {
        StorageType code = 0;

        auto r = br.template get_bits<StorageType>(code, TypeInfo::BitsPerElement);
        if (r != BitFieldResult::Ok) {
          return r;
        }

        if (code > TypeInfo::MaxCode)
          return BitFieldResult::ErrorValueTooLarge;

        r = out.push_back(sub8::packing::unpack<Type>(code));
        if (r != BitFieldResult::Ok)
          return r;
    }
  }

  return BitFieldResult::Ok;
}

// Delimited read / write
template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const ArrayBitField<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Delimited>  &p) {
  return write_delimited_field(bw, p);
}

template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,ArrayBitField<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Delimited> &out) {
  return read_delimited_field(br, out);
}

// Prefixed read / write
template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,ArrayBitField<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Prefixed> &out) {
  return read_prefixed_field(br, out);
}

template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const ArrayBitField<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::Prefixed> &p) {
  return write_prefixed_field(bw, p);
}

// Three Plus Prefixed read / write
template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const ArrayBitField<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::ThreePlusPrefixed> &p) {
  
  if constexpr (TMaxElements == TMinElements) {
    // use the prefixed implementation for fixed feild sizes
    // While both methods can support it, the behavior will be more stable
    // if only one implementation is used
    return write_prefixed_field(bw, p);
  } else if constexpr ((TMaxElements - TMinElements) <= 3) {
    return write_delimited_field(bw, p);
  } else {
    return write_prefixed_field(bw, p);
  }
}

template<typename Storage, typename TypeInfo, size_t TMinElements, size_t TMaxElements>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,ArrayBitField<TypeInfo, TMinElements, TMaxElements, ArrayEncoding::ThreePlusPrefixed> &out) {
  if constexpr (TMaxElements == TMinElements) {
    // use the prefixed implementation for fixed feild sizes
    // While both methods can support it, the behavior will be more stable
    // if only one implementation is used
    return read_prefixed_field(br, out);
  } else if constexpr ((TMaxElements - TMinElements) <= 3) {
    return read_delimited_field(br, out);
  } else {
    return read_prefixed_field(br, out);
  } 
}

// Object Array

template<typename T, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed>
using ObjectArrayBitField = ArrayBitField<ObjectElement<T>, TMinElements, TMaxElements, Encoding>;

template<typename T, uint8_t BitWidth, size_t TMinElements, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed>
using NumericArrayBitField = ArrayBitField<PrimitiveElement<T, BitWidth>, TMinElements, TMaxElements, Encoding>;

template<typename T, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed>
using ObjectFixedArrayBitField = ArrayBitField<ObjectElement<T>, TMaxElements, TMaxElements, Encoding>;

template<typename T, uint8_t BitWidth, size_t TMaxElements, ArrayEncoding Encoding = ArrayEncoding::ThreePlusPrefixed>
using NumericFixedArrayBitField = ArrayBitField<PrimitiveElement<T, BitWidth>, TMaxElements, TMaxElements, Encoding>;


#endif  // SUB8_ENABLE_ARRAY_FIELDS

}  // namespace sub8
