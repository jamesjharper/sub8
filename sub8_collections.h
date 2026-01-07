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

// Enable: Nested Array Fields
// A repeating sequence of serializable types.
#ifndef SUB8_ENABLE_NESTED_ARRAYS
#define SUB8_ENABLE_NESTED_ARRAYS 1
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
template<
    // T:
    // C++ value type used to represent each element.
    // Must be able to represent at least TBitsPerElement bits.
    typename T,

    // TBitsPerElement:
    // Number of bits used to encode each element value.
    uint8_t TBitsPerElement,

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
    ArrayEncoding TEncoding = ArrayEncoding::ThreePlusPrefixed,

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
    size_t TMinElements = 0
>
class ArrayBitField {
  size_t size_{0};
  T value_[TMaxElements]{};

 public:
  static_assert(TBitsPerElement >= 1);
  static_assert(TMaxElements >= TMinElements);

  using Type = T;
  using ValueType = BitIndexValue<T>;
  using InitType = std::initializer_list<T>;

  using UnderlyingType = typename unpack_t::underlying_or_self<T>::type;
  using StorageType = std::make_unsigned_t<UnderlyingType>;

  static constexpr uint8_t BitsPerElement = TBitsPerElement;
  static constexpr size_t MinElements = TMinElements;
  static constexpr size_t MaxElements = TMaxElements;

  using ElementLimits = sub8::limits::bits_limits<T, BitsPerElement>;
  static constexpr StorageType ElementMaxCode = ElementLimits::MaxCode;
  static constexpr T MinValue = ElementLimits::MinValue;
  static constexpr T MaxValue = ElementLimits::MaxValue;

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

  ArrayBitField() noexcept = default;
  ArrayBitField(const ArrayBitField &) noexcept = default;

  ArrayBitField(std::initializer_list<T> init) noexcept { set_value(init); }

  bool empty() const noexcept { return size_ == 0; }
  size_t size() const noexcept { return size_; }
  const T &operator[](size_t i) const noexcept { return value_[i]; }

  void clear() noexcept { size_ = 0; }

  BitIndexValue<T> value() const noexcept { return BitIndexValue<T>{value_, &size_}; }
  explicit operator BitIndexValue<T>() const noexcept { return value(); }

  bool operator==(const ArrayBitField &o) const noexcept {

    return size_ == o.size_ && std::memcmp(value_, o.value_, size_ * sizeof(T)) == 0;
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

  ArrayBitField &operator=(std::initializer_list<T> init) noexcept {
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
      auto r = push_back(static_cast<T>(seq[i]));
      if (r != BitFieldResult::Ok) return r;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(const T *seq, size_t len) {
    clear();
    if (seq == nullptr) {
      return (len == 0) ? BitFieldResult::Ok : BitFieldResult::ErrorInvalidBitFieldValue;
    }
    if (len > TMaxElements) return BitFieldResult::ErrorTooManyElements;

    for (size_t i = 0; i < len; ++i) {
      auto r = push_back(static_cast<T>(seq[i]));
      if (r != BitFieldResult::Ok) return r;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(std::initializer_list<T> init) {
    struct View {
      const T *ptr;
      size_t n;
      size_t size() const noexcept { return n; }
      const T &operator[](size_t i) const noexcept { return ptr[i]; }
    };
    return set_value(View{init.begin(), init.size()});
  }

  BitFieldResult push_back(T v) {
    if (v < MinValue || v > MaxValue) return BitFieldResult::ErrorValueTooLarge;
    if (size_ >= TMaxElements) return BitFieldResult::ErrorTooManyElements;
    value_[size_++] = v;
    return BitFieldResult::Ok;
  }
};

// Three Plus Prefixed read / write

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::ThreePlusPrefixed, TMinElements> &p) {
  
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

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::ThreePlusPrefixed, TMinElements> &out) {
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

// Delimited read / write

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Delimited, TMinElements> &p) {
  return write_delimited_field(bw, p);
}

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, ArrayEncoding TArrayEncoding, size_t TMinElements>
inline BitFieldResult write_delimited_field(BasicBitWriter<Storage> &bw, const ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements> &p) {
  using F = ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements>;
  using StorageType = typename F::StorageType;

  constexpr bool kGroupFitsInStorage = (uint32_t(TBitsPerElement) + 1u) <= uint32_t(sizeof(StorageType) * 8u);

  const size_t n = p.size();
  const size_t effective_n = (n < TMinElements) ? TMinElements : n; // zero-pad up to min
  if (effective_n > TMaxElements) {
    return BitFieldResult::ErrorTooManyElements;
  }

  auto write_prefixed_value = [&](T val) -> BitFieldResult {

    if constexpr (sub8::detect::has_write_field_v<Storage, T>) {
      return write_field(bw, val);
    } else {
      const StorageType code = sub8::packing::pack<T>(val);
      if (code > F::ElementMaxCode) 
        return BitFieldResult::ErrorValueTooLarge;

      else if constexpr (kGroupFitsInStorage) {
        // one op: [1 | code]
        const StorageType group = (StorageType{1} << TBitsPerElement) | code;
        return bw.template put_bits<StorageType>(group, uint32_t(TBitsPerElement) + 1u);
      } else {
        // two ops: 1 then code
        auto r = bw.template put_bits<uint8_t>(1u, 1);
        if (r != BitFieldResult::Ok) return r;
        return bw.template put_bits<StorageType>(code, TBitsPerElement);
      }
    }
  };

  auto write_non_prefixed_value = [&](T val) -> BitFieldResult {
    if constexpr (sub8::detect::has_write_field_v<Storage, T>) {
      return write_field(bw, val);
    } else {
      const StorageType code = sub8::packing::pack<T>(val);
      if (code > F::ElementMaxCode) 
        return BitFieldResult::ErrorValueTooLarge;

        
      return bw.template put_bits<StorageType>(code, TBitsPerElement);
    }
  };

  // Empty handling
  size_t i = 0;

  if constexpr (TMinElements == 0) {
    if (n == 0) {
      // empty is encoded as just "0"
      return bw.template put_bits<uint8_t>(0, 1);
    }
  } else {

    for (; i < TMinElements; ++i) {
      const T v = (i < n) ? p[i] : T{};
      auto r = write_non_prefixed_value(v);
      if (r != BitFieldResult::Ok) {
        return r;
      }
    }
  }

  for (; i < effective_n; ++i) {
    auto r = write_prefixed_value(p[i]);
    if (r != BitFieldResult::Ok) 
      return r;
  }
  // Trailing 0 terminator is omitted this array is less then max elements in length
  if (n != TMaxElements) {
    return bw.template put_bits<uint8_t>(0, 1);
  }
  return BitFieldResult::Ok;
}

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Delimited, TMinElements> &out) {
  return read_delimited_field(br, out);
}

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, ArrayEncoding TArrayEncoding, size_t TMinElements>
inline BitFieldResult read_delimited_field(BasicBitReader<Storage> &br,ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements> &out) {
  using F = ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements>;
  using StorageType = typename F::StorageType;

  out.clear();
  constexpr StorageType data_mask = static_cast<StorageType>(F::ElementMaxCode);
  constexpr bool kGroupFitsInStorage = (uint32_t(TBitsPerElement) + 1u) <= uint32_t(sizeof(StorageType) * 8u);

  if constexpr (TMinElements != 0) {
      while (out.size() < TMinElements) {

        if constexpr (sub8::detect::has_read_field_v<Storage, T>) {
          T inner {};
          auto r = read_field(br, inner);
          if (r != BitFieldResult::Ok) {
            return r;
          }
          r = out.push_back(inner);
          if (r != BitFieldResult::Ok) {
            return r;
          }
        } else {
          StorageType raw = 0;
          if (!br.template get_bits<StorageType>(raw, uint32_t(TBitsPerElement))) {
            return BitFieldResult::ErrorExpectedMoreBits;
          }

          StorageType code = StorageType(raw & data_mask);
          auto r = out.push_back(sub8::packing::unpack<T>(code));
          if (r != BitFieldResult::Ok) {
            return r;
          }
        }
    }
  }

  uint8_t cont = 0;
  if (!br.template get_bits<uint8_t>(cont, 1)) {
      return BitFieldResult::ErrorExpectedMoreBits;
  }
  if(cont == 0) {
    return BitFieldResult::Ok;
  }

  while (out.size() < TMaxElements - 1) {

    StorageType raw = 0;
    uint8_t cont = 0;

    if constexpr (sub8::detect::has_read_field_v<Storage, T>) {
        T inner {};
        auto r = read_field(br, inner);
        if (r != BitFieldResult::Ok) {
          return r;
        }
        r = out.push_back(inner);
        if (r != BitFieldResult::Ok) {
          return r;
        }

        if (!br.template get_bits<uint8_t>(cont, 1)) {
          return BitFieldResult::ErrorExpectedMoreBits;
        }
    } else if constexpr (!kGroupFitsInStorage) {
      if (!br.template get_bits<StorageType>(raw, uint32_t(TBitsPerElement))) {
        return BitFieldResult::ErrorExpectedMoreBits;
      }

      if (!br.template get_bits<uint8_t>(cont, 1)) {
        return BitFieldResult::ErrorExpectedMoreBits;
      }
    } else {

      if (!br.template get_bits<StorageType>(raw, uint32_t(TBitsPerElement) + 1u)) {
        return BitFieldResult::ErrorExpectedMoreBits;
      }

      cont = uint8_t(raw & StorageType{1});
      raw  = StorageType(raw >> 1);  
    }

    StorageType code = StorageType(raw & data_mask);
    auto r = out.push_back(sub8::packing::unpack<T>(code));
    if (r != BitFieldResult::Ok) {
      return r;
    }

    // stream is terminated by continuation bit of '0' 
    if ((cont & 1u) == 0u) {
      return BitFieldResult::Ok;
    }
  }

  StorageType last_value = 0;
  if (!br.template get_bits<StorageType>(last_value, uint32_t(TBitsPerElement))) {
    return BitFieldResult::ErrorExpectedMoreBits;
  }

  return out.push_back(sub8::packing::unpack<T>(last_value));
}

// Prefixed read / write

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Prefixed, TMinElements> &p) {
  return write_prefixed_field(bw, p);
}

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, ArrayEncoding TArrayEncoding, size_t TMinElements>
inline BitFieldResult write_prefixed_field(BasicBitWriter<Storage> &bw, const ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements> &f) {
  using F = const ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements>;
  using StorageType = typename F::StorageType;

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
    const T v = (i < n) ? values[i] : T{};

    if constexpr (sub8::detect::has_write_field_v<Storage, T>) {
      auto r = write_field(bw, v);
       if (r != BitFieldResult::Ok)
        return r;
    } else {
      const StorageType code = sub8::packing::pack<T>(v);
      if (code > F::ElementMaxCode)
        return BitFieldResult::ErrorValueTooLarge;

      auto r = bw.template put_bits<StorageType>(code, F::BitsPerElement);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }
  return BitFieldResult::Ok;
}

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements>
inline BitFieldResult read_field(BasicBitReader<Storage> &br,ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Prefixed, TMinElements> &out) {
  return read_prefixed_field(br, out);
}

template<typename Storage, typename T, uint8_t TBitsPerElement, size_t TMaxElements, ArrayEncoding TArrayEncoding, size_t TMinElements>
inline BitFieldResult read_prefixed_field(BasicBitReader<Storage> &br, ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements> &out) {
  using F = ArrayBitField<T, TBitsPerElement, TMaxElements, TArrayEncoding, TMinElements>;
  using StorageType = typename F::StorageType;

  out.clear();

  // Read length prefix
  size_t n = 0;

  if constexpr (TMaxElements == TMinElements) {
    n = TMaxElements;
  } else {
    if (!br.template get_bits<size_t>(n, F::LengthBits))
      return BitFieldResult::ErrorExpectedMoreBits;

    n = n + TMinElements;

    if (n > TMaxElements)
      return BitFieldResult::ErrorTooManyElements;
  }

  // Read elements
  for (size_t i = 0; i < n; ++i) {

    if constexpr (sub8::detect::has_read_field_v<Storage, T>) {
      T inner {};
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
        if (!br.template get_bits<StorageType>(code, F::BitsPerElement))
          return BitFieldResult::ErrorExpectedMoreBits;

        if (code > F::ElementMaxCode)
          return BitFieldResult::ErrorValueTooLarge;

        auto r = out.push_back(sub8::packing::unpack<T>(code));
        if (r != BitFieldResult::Ok)
          return r;
    }
  }

  return BitFieldResult::Ok;
}

template<typename T, uint8_t TBitsPerElement, size_t TMaxElements>
using PathBitField = ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Delimited, 0>;

template<typename T, uint8_t TBitsPerElement, size_t TMinElements, size_t TMaxElements>
using NonEmptyPathBitField = ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Delimited, TMinElements>;

template<typename T, uint8_t TBitsPerElement, size_t TMaxElements, size_t TMinElements = 0>
using PrefixedBitField = ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Prefixed, TMinElements>;

template<typename T, uint8_t TBitsPerElement, size_t TMinElements, size_t TMaxElements>
using NonEmptyPrefixedBitField = ArrayBitField<T, TBitsPerElement, TMaxElements, ArrayEncoding::Prefixed, TMinElements>;

template<typename T, uint8_t TBitsPerElement, size_t TMaxElements>
using FixedArrayBitField = ArrayBitField<T, TBitsPerElement, TMaxElements,ArrayEncoding::Prefixed, TMaxElements>;


#endif  // SUB8_ENABLE_ARRAY_FIELDS

// Nested array Field
// ----------------------
#if SUB8_ENABLE_NESTED_ARRAYS

template<
    // Elem: element storage type, must be equal or greater than the wire bit size
    typename Elem,
    // TMaxElements: Max length of path.
    // the last element will not contain a continuation bit. This can save a bit if
    // likely a max length is known at at compile time
    size_t TMaxElements,

    // TAllowEmpty: Allow for empty paths
    // by preventing empty paths allows the first continuation to be dropped.
    // if the path is known to never be empty, then this can save a bit
    bool TAllowEmpty = true>
class NestedBitField {
  size_t size_{0};
  Elem value_[TMaxElements]{};

 public:
  static_assert(TMaxElements >= 1, "NestedBitField: TMaxElements must be >= 1");
  static_assert(std::is_default_constructible_v<Elem>, "Elem must be default-constructible");
  static_assert(std::is_copy_assignable_v<Elem>, "Elem must be copy-assignable");

  using Type = Elem;
  using ValueType = Elem;
  using InitType = std::initializer_list<Elem>;

  NestedBitField() noexcept = default;
  NestedBitField(const NestedBitField &) noexcept = default;
  NestedBitField(std::initializer_list<Elem> init) {
    auto r = set_value(init);
    assert(r == BitFieldResult::Ok);
    (void) r;
  }

  bool empty() const noexcept { return size_ == 0; }
  size_t size() const noexcept { return size_; }
  void clear() noexcept { size_ = 0; }

  const Elem *data() const noexcept { return value_; }
  Elem *data() noexcept { return value_; }

  const Elem &operator[](size_t i) const noexcept { return value_[i]; }
  Elem &operator[](size_t i) noexcept { return value_[i]; }

  bool operator==(const NestedBitField &o) const noexcept {
    if (size_ != o.size_)
      return false;
      
    for (size_t i = 0; i < size_; ++i) {
      if (!(value_[i] == o.value_[i]))
        return false;
    }
    return true;
  }
  bool operator!=(const NestedBitField &o) const noexcept { return !(*this == o); }

  NestedBitField &operator=(const NestedBitField &) noexcept = default;

  template<typename Seq> NestedBitField &operator=(const Seq &seq) noexcept {
    auto r = set_value(seq);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  NestedBitField &operator=(std::initializer_list<Elem> init) noexcept {
    auto r = set_value(init);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  template<typename Seq> BitFieldResult set_value(const Seq &seq) {
    clear();
    const size_t n = seq.size();

    if constexpr (!TAllowEmpty) {
      if (n == 0) {
        assert(false && "NestedBitField: empty path not allowed");
        return BitFieldResult::ErrorInvalidBitFieldValue;
      }
    }

    if (n > TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    for (size_t i = 0; i < n; ++i) {
      auto r = push(seq[i]);
      if (r != BitFieldResult::Ok)
        return r;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(const Elem *seq, size_t len) {
    clear();

    if (seq == nullptr) {
      return (len == 0) ? BitFieldResult::Ok : BitFieldResult::ErrorInvalidBitFieldValue;
    }
    if (len > TMaxElements) {
      return BitFieldResult::ErrorTooManyElements;
    }

    for (size_t i = 0; i < len; ++i) {
      auto r = push(seq[i]);
      if (r != BitFieldResult::Ok)
        return r;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult set_value(std::initializer_list<Elem> init) {
    struct View {
      const Elem *ptr;
      size_t n;
      size_t size() const noexcept { return n; }
      const Elem &operator[](size_t i) const noexcept { return ptr[i]; }
    };
    return set_value(View{init.begin(), init.size()});
  }

  BitFieldResult push(const Elem &v) {
    if (size_ >= TMaxElements)
      return BitFieldResult::ErrorTooManyElements;
    value_[size_++] = v;
    return BitFieldResult::Ok;
  }
};

template<typename Storage, typename Elem, size_t TMaxElements, bool TAllowEmpty>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw,
                                  const NestedBitField<Elem, TMaxElements, TAllowEmpty> &p) {
  const size_t n = p.size();

  if constexpr (TAllowEmpty) {
    if (n == 0) {
      return bw.template put_bits<uint8_t>(0, 1);
    }
    auto r = bw.template put_bits<uint8_t>(1, 1);
    if (r != BitFieldResult::Ok)
      return r;
  } else {
    if (n == 0) {
      assert(false && "NestedBitField: empty path not allowed");
      return BitFieldResult::ErrorInvalidBitFieldValue;
    }
  }

  for (size_t i = 0; i < n; ++i) {

    auto r = write_field(bw, p[i]);
    if (r != BitFieldResult::Ok)
      return r;

    const bool is_last = (i + 1 == n);
    const bool omit_cont = (is_last && (n == TMaxElements));

    if (!omit_cont) {
      const uint8_t cont = is_last ? 0u : 1u;
      auto r = bw.template put_bits<uint8_t>(cont, 1);
      if (r != BitFieldResult::Ok)
        return r;
    }
  }

  return BitFieldResult::Ok;
}

template<typename Storage, typename Elem, size_t TMaxElements, bool TAllowEmpty>
inline BitFieldResult read_field(BasicBitReader<Storage> &br, NestedBitField<Elem, TMaxElements, TAllowEmpty> &out) {
  out.clear();

  if constexpr (TAllowEmpty) {
    uint8_t start_bit = 0;
    if (!br.template get_bits<uint8_t>(start_bit, 1)) {
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    if (start_bit == 0) {
      return BitFieldResult::Ok;  // empty path
    }
  }

  size_t groups_read = 0;
  while (groups_read < TMaxElements) {


    Elem tmp{};
    {
      auto r = read_field(br, tmp);  // MUST be self-delimiting
      if (r != BitFieldResult::Ok)
        return r;
    }

    {
      auto r = out.push(tmp);
      if (r != BitFieldResult::Ok)
        return r;
    }

    ++groups_read;

    // Only the last possible slot omits cont bit (the optimization case).
    uint8_t cont = 0;
    const bool have_cont_bit = ! (groups_read + 1 == TMaxElements);

    if (have_cont_bit) {
      if (!br.template get_bits<uint8_t>(cont, 1)) {
        return BitFieldResult::ErrorExpectedMoreBits;
      }
    }

    if (!have_cont_bit) {
      // last-slot optimization: no cont bit, must end here
      return BitFieldResult::Ok;
    }

    if (cont == 0) {
      // normal termination
      return BitFieldResult::Ok;
    }
    // else cont==1 => loop for next element
  }

  return BitFieldResult::Ok;
}

#endif  // SUB8_ENABLE_NEST_ARRAYS

}  // namespace sub8
