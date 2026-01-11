#pragma once
#include <cassert>
#include "sub8_errors.h"

// Feature flags":
// ===============

// Supported read/write buffer types
// ---------------------------------

// Enable: Support for stack based buffers
#ifndef SUB8_ENABLE_BOUNDED_BUF
#define SUB8_ENABLE_BOUNDED_BUF 1
#endif

// Enable: For heap based buffers (uses vectors internally)
#ifndef SUB8_ENABLE_UNBOUNDED_BUF
#define SUB8_ENABLE_UNBOUNDED_BUF 1
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
#include <vector>
#endif

namespace sub8 {

namespace packing {

template<typename T> using underlying_or_self_t = typename unpack_t::underlying_or_self<T>::type;

template<typename T> using packed_t = std::make_unsigned_t<underlying_or_self_t<T>>;

template<typename S> using make_unsigned_t = typename std::make_unsigned<S>::type;

template<typename S> constexpr make_unsigned_t<S> zigzag_encode(S x) noexcept {
  static_assert(std::is_signed<S>::value, "zigzag::encode expects signed type");
  using U = make_unsigned_t<S>;
  return (static_cast<U>(x) << 1) ^ static_cast<U>(x >> (std::numeric_limits<S>::digits));
}

template<typename S> constexpr S zigzag_decode(make_unsigned_t<S> u) noexcept {
  static_assert(std::is_signed<S>::value, "zigzag::decode expects signed type");
  using U = make_unsigned_t<S>;
  return static_cast<S>((u >> 1) ^ static_cast<U>(-static_cast<typename std::make_signed<U>::type>(u & 1)));
}

template<typename T> constexpr packed_t<T> pack(T v) noexcept {
  using U = underlying_or_self_t<T>;
  using P = packed_t<T>;

  if constexpr (std::is_signed_v<U>) {
    return static_cast<P>(zigzag_encode<U>(static_cast<U>(v)));
  } else {
    return static_cast<P>(static_cast<U>(v));
  }
}

template<typename T> constexpr T unpack(packed_t<T> code) noexcept {
  using U = underlying_or_self_t<T>;

  if constexpr (std::is_signed_v<U>) {
    const U v = zigzag_decode<U>(code);
    return static_cast<T>(v);
  } else {
    return static_cast<T>(static_cast<U>(code));
  }
}

}  // namespace packing

// Bit buffers
// =======================

// Object for addressing bit buffer lengths and position
class BitSize {
  size_t byte_size_ = 0;       // whole bytes
  uint8_t bit_remainder_ = 0;  // 0..7 extra bits beyond whole bytes

  // Keeps invariant: bit_remainder_ in [0,7] by carrying into bytes.
  constexpr void normalize() noexcept {
    if (bit_remainder_ >= 8) {
      byte_size_ += static_cast<size_t>(bit_remainder_ / 8);
      bit_remainder_ = static_cast<uint8_t>(bit_remainder_ % 8);
    }
  }

 public:
  constexpr BitSize() noexcept = default;

  constexpr BitSize(size_t byte_size, uint8_t bit_remainder) noexcept
      : byte_size_{byte_size}, bit_remainder_{bit_remainder} {
    normalize();
  }

  static constexpr BitSize from_bits(uint64_t bits) noexcept {
    return BitSize{static_cast<size_t>(bits / 8), static_cast<uint8_t>(bits % 8)};
  }

  // Accessors
  constexpr uint8_t bit_remainder() const noexcept { return bit_remainder_; }
  constexpr size_t byte_size_round_down() const noexcept { return byte_size_; }

  // Bytes required to store this many bits (round up to next byte if partial)
  constexpr size_t byte_size_round_up() const noexcept { return byte_size_ + (bit_remainder_ ? 1u : 0u); }

  // Generally avoid using bit_size() and lbit_size().
  // Will overflow with very large values.
  constexpr size_t bit_size() const noexcept { return (byte_size_ * size_t{8}) + size_t{bit_remainder_}; }

  constexpr uint64_t lbit_size() const noexcept {
    return (static_cast<uint64_t>(byte_size_) * 8ULL) + static_cast<uint64_t>(bit_remainder_);
  }

  constexpr BitSize &add_bits(uint64_t bits) noexcept {
    *this += from_bits(bits);
    return *this;
  }

  constexpr BitSize &sub_bits(uint64_t bits) noexcept {
    *this -= from_bits(bits);
    return *this;
  }

  // Comparisons
  friend constexpr bool operator==(const BitSize &a, const BitSize &b) noexcept {
    return a.byte_size_ == b.byte_size_ && a.bit_remainder_ == b.bit_remainder_;
  }
  friend constexpr bool operator!=(const BitSize &a, const BitSize &b) noexcept { return !(a == b); }

  friend constexpr bool operator<(const BitSize &a, const BitSize &b) noexcept {
    return (a.byte_size_ < b.byte_size_) || (a.byte_size_ == b.byte_size_ && a.bit_remainder_ < b.bit_remainder_);
  }
  friend constexpr bool operator>(const BitSize &a, const BitSize &b) noexcept { return b < a; }
  friend constexpr bool operator<=(const BitSize &a, const BitSize &b) noexcept { return !(b < a); }
  friend constexpr bool operator>=(const BitSize &a, const BitSize &b) noexcept { return !(a < b); }

  // Arithmetic (BitSize +/- BitSize)
  friend constexpr BitSize operator+(BitSize a, const BitSize &b) noexcept {
    a += b;
    return a;
  }

  friend constexpr BitSize operator-(BitSize a, const BitSize &b) noexcept {
    a -= b;
    return a;
  }

  constexpr BitSize &operator+=(const BitSize &other) noexcept {
    byte_size_ += other.byte_size_;
    bit_remainder_ = static_cast<uint8_t>(bit_remainder_ + other.bit_remainder_);
    normalize();
    return *this;
  }

  constexpr BitSize &operator-=(const BitSize &other) noexcept {
    // Require no underflow
    assert(*this >= other && "BitSize underflow");
    if (bit_remainder_ < other.bit_remainder_) {
      // borrow 1 byte => +8 bits
      assert(byte_size_ > other.byte_size_ && "BitSize underflow (borrow)");
      byte_size_ -= 1;
      bit_remainder_ = static_cast<uint8_t>(bit_remainder_ + 8);
    }
    bit_remainder_ = static_cast<uint8_t>(bit_remainder_ - other.bit_remainder_);
    byte_size_ -= other.byte_size_;
    return *this;
  }
};

/*
template<typename Storage> class BasicBitWriter {
  Storage buf_;
  uint8_t sub_byte_pos_ = 0;

 public:
  Storage &storage() noexcept { return buf_; }
  const Storage &storage() const noexcept { return buf_; }

  BitSize size() const noexcept {
    auto buf_size = buf_.size();
    if (sub_byte_pos_ == 0 || buf_size == 0) {
      return BitSize(buf_size, sub_byte_pos_);
    }
    return BitSize(buf_size - 1, sub_byte_pos_);
  }

  BitFieldResult put_padding(uint8_t nbits) {
    if (!nbits)
      return BitFieldResult::Ok;

    uint8_t remaining = nbits;

    while (remaining >= 8) {
      auto r = put_bits(0u, 8);
      if (r != BitFieldResult::Ok)
        return r;

      remaining -= 8;
    }

    if (remaining) {
      return put_bits(0u, remaining);
    }
    return BitFieldResult::Ok;
  }

  template<typename T> BitFieldResult put_bits(T value, uint8_t nbits) {
    using U = typename unpack_t::underlying_or_self<T>::type;

    static_assert(std::is_unsigned<U>::value, "BitWriter::put_bits expects unsigned values");
    assert(nbits > 0);
    assert(nbits <= sizeof(U) * 8);

    const U v = static_cast<U>(value);

    // Small fields go straight through the 8-bit path (no loop)
    if (nbits <= 8) {
      return put_bits(static_cast<uint8_t>(v), static_cast<uint8_t>(nbits));
    } else {
      size_t remaining = nbits;

      // Emit MSB-first 8-bit slices
      while (remaining) {
        const uint8_t take = static_cast<uint8_t>(remaining >= 8 ? 8 : remaining);
        const uint8_t shift = static_cast<uint8_t>(remaining - take);
        const uint8_t chunk = static_cast<uint8_t>((v >> shift) & ((U(1) << take) - 1u));

        auto r = put_bits(chunk, take);
        if (r != BitFieldResult::Ok)
          return r;
        remaining -= take;
      }
    }
    return BitFieldResult::Ok;
  }

  template<> BitFieldResult put_bits(uint8_t v, uint8_t nbits) {
    if (!nbits)
      return BitFieldResult::Ok;

    assert(nbits <= 8);

    // keep only low nbits
    if (nbits < 8) {
      v &= static_cast<uint8_t>((static_cast<uint8_t>(1u) << nbits) - 1u);
    }

    uint8_t pos = sub_byte_pos_;

    while (nbits) {
      if (pos == 0) {
        if (!buf_.push_back(0)) {
          return BitFieldResult::ErrorInsufficentBufferSize;
        }
      }

      const uint8_t remaining_space = static_cast<uint8_t>(8u - pos);
      const uint8_t take = (nbits < remaining_space ? nbits : remaining_space);

      const uint8_t shift = static_cast<uint8_t>(nbits - take);
      const uint8_t chunk = static_cast<uint8_t>((v >> shift) & ((static_cast<uint8_t>(1u) << take) - 1u));

      uint8_t &dst = buf_.back();
      dst |= static_cast<uint8_t>(chunk << (remaining_space - take));

      pos = static_cast<uint8_t>(pos + take);
      nbits = static_cast<uint8_t>(nbits - take);

      if (pos == 8) {
        pos = 0;
      }
    }

    sub_byte_pos_ = pos;
    return BitFieldResult::Ok;
  }
};
*/

template<typename Storage>
class BasicBitWriter {
  Storage buf_;
  uint8_t sub_byte_pos_ = 0;

 public:
  Storage &storage() noexcept { return buf_; }
  const Storage &storage() const noexcept { return buf_; }

  BitSize size() const noexcept {
    auto buf_size = buf_.size();
    if (sub_byte_pos_ == 0 || buf_size == 0) {
      return BitSize(buf_size, sub_byte_pos_);
    }
    return BitSize(buf_size - 1, sub_byte_pos_);
  }

  BitFieldResult put_padding(uint8_t nbits) noexcept {
    if (!nbits) {
      return BitFieldResult::Ok;
    }

    uint8_t remaining = nbits;
    while (remaining >= 8) {
      auto r = put_bits(uint8_t{0}, uint8_t{8});
      if (r != BitFieldResult::Ok) {
        return r;
      }
      remaining -= 8;
    }
    if (remaining) {
      return put_bits(uint8_t{0}, remaining);
    }
    return BitFieldResult::Ok;
  }

  template<typename T>
  BitFieldResult put_bits(T value, uint8_t nbits) noexcept {
    using U = typename unpack_t::underlying_or_self<T>::type;
    static_assert(std::is_unsigned<U>::value, "BitWriter::put_bits expects unsigned values");

    if(nbits == 0) {
      return BitFieldResult::Ok;
    }

    const U v = static_cast<U>(value);
    if (nbits <= 8) {
      return put_bits(static_cast<uint8_t>(v), static_cast<uint8_t>(nbits));
    }

    if(nbits > sizeof(U) * 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    size_t remaining = nbits;
    while (remaining) {
      const uint8_t take  = static_cast<uint8_t>(remaining >= 8 ? 8 : remaining);
      const uint8_t shift = static_cast<uint8_t>(remaining - take);
      const uint8_t chunk = static_cast<uint8_t>((v >> shift) & ((U(1) << take) - 1u));

      auto r = put_bits(chunk, take);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      remaining -= take;
    }
    return BitFieldResult::Ok;
  }

  BitFieldResult put_bits(uint8_t v, uint8_t nbits) noexcept {
    if(nbits == 0) {
      return BitFieldResult::Ok;
    }

    if(nbits > 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    if (nbits < 8) {
      v &= static_cast<uint8_t>((static_cast<uint8_t>(1u) << nbits) - 1u);
    }

    uint8_t pos = sub_byte_pos_;

    while (nbits) {
      if (pos == 0) {
        auto r = buf_.push_back(0);
        if (r != BitFieldResult::Ok) {
          return r;
        }
      }

      const uint8_t remaining_space = static_cast<uint8_t>(8u - pos);
      const uint8_t take = (nbits < remaining_space ? nbits : remaining_space);

      const uint8_t shift = static_cast<uint8_t>(nbits - take);
      const uint8_t chunk = static_cast<uint8_t>((v >> shift) & ((static_cast<uint8_t>(1u) << take) - 1u));

      uint8_t *dst = nullptr;
      auto br = buf_.back_mut_ptr(dst);
      if (br != BitFieldResult::Ok || dst == nullptr) return br;

      *dst |= static_cast<uint8_t>(chunk << (remaining_space - take));

      pos   = static_cast<uint8_t>(pos + take);
      nbits = static_cast<uint8_t>(nbits - take);

      if (pos == 8) pos = 0;
    }

    sub_byte_pos_ = pos;
    return BitFieldResult::Ok;
  }
};

/*
template<typename Storage> class BasicBitReader {
  const Storage &buf_;

  BitSize total_size_;

  size_t idx_ = 0;
  uint8_t sub_byte_pos_ = 0;

 public:
  BasicBitReader(const Storage &data, BitSize bit_size) : buf_(data), total_size_(bit_size) {
    assert(total_size_.byte_size_round_up() <= buf_.size());
  }

  inline const Storage &storage() const noexcept { return buf_; }

  inline const BitSize &size() const noexcept { return total_size_; }

  inline BitSize cursor_position() const noexcept { return BitSize(idx_, sub_byte_pos_); }

  inline void set_cursor_position(BitSize pos) noexcept {
    idx_ = pos.byte_size_round_down();
    sub_byte_pos_ = pos.bit_remainder();
  }

  inline bool has(size_t bits) const noexcept {
    size_t has_bytes = (bits >> 3) + idx_;
    uint8_t has_bits = static_cast<uint8_t>(bits & 7) + sub_byte_pos_;

    if (has_bits >= 8) {
      has_bits -= 8;
      has_bytes++;
    }

    return has_bytes == total_size_.byte_size_round_down() ? has_bits <= total_size_.bit_remainder()
                                                           : has_bytes <= total_size_.byte_size_round_down();
  }

  template<typename T> inline bool get_bits(T &out, size_t nbits) noexcept {
    using U = typename unpack_t::underlying_or_self<T>::type;

    if (nbits == 0) {
      out = T(0);
      return true;
    }
    if (nbits > sizeof(U) * 8)
      return false;
    if (!has(nbits))
      return false;

    U acc = 0;
    size_t rem = nbits;

    while (rem) {
      uint8_t take = uint8_t(rem > 8 ? 8 : rem);
      uint8_t chunk;
      if (!get_bits<uint8_t>(chunk, take))
        return false;

      acc = U((acc << take) | chunk);
      rem -= take;
    }

    out = static_cast<T>(acc);
    return true;
  }

  template<> inline bool get_bits(uint8_t &out, size_t nbits) noexcept {
    if (nbits == 0) {
      out = 0;
      return true;
    }
    if (nbits > 8)
      return false;

    if (!has(nbits))
      return false;

    uint8_t v = 0;
    uint8_t need = nbits;
    const size_t max_bytes = total_size_.byte_size_round_up();

    size_t idx = idx_;
    uint8_t pos = sub_byte_pos_;

    while (need) {
      if (idx >= max_bytes)
        return false;

      const uint8_t cur = buf_[idx];
      const uint8_t avail = uint8_t(8 - pos);
      const uint8_t take = (need < avail ? need : avail);

      const uint8_t shift = uint8_t(avail - take);
      const uint8_t mask = uint8_t(((1u << take) - 1u) << shift);
      v = uint8_t((v << take) | ((cur & mask) >> shift));

      pos += take;
      need -= take;

      if (pos == 8) {
        pos = 0;
        ++idx;
      }
    }

    idx_ = idx;
    sub_byte_pos_ = pos;
    out = v;
    return true;
  }
};
*/

template<typename Storage>
class BasicBitReader {
  const Storage &buf_;
  BitSize total_size_;
  size_t idx_ = 0;
  uint8_t sub_byte_pos_ = 0;

 public:
  BasicBitReader(const Storage &data, BitSize bit_size) noexcept
      : buf_(data), total_size_(bit_size) {
    assert(total_size_.byte_size_round_up() <= buf_.size());
  }

  const Storage &storage() const noexcept { return buf_; }
  const BitSize &size() const noexcept { return total_size_; }
  BitSize cursor_position() const noexcept { return BitSize(idx_, sub_byte_pos_); }

  void set_cursor_position(BitSize pos) noexcept {
    idx_ = pos.byte_size_round_down();
    sub_byte_pos_ = pos.bit_remainder();
  }

  bool has(size_t bits) const noexcept {
    size_t has_bytes = (bits >> 3) + idx_;
    uint8_t has_bits = static_cast<uint8_t>(bits & 7) + sub_byte_pos_;

    if (has_bits >= 8) {
      has_bits -= 8;
      has_bytes++;
    }

    return has_bytes == total_size_.byte_size_round_down()
             ? has_bits <= total_size_.bit_remainder()
             : has_bytes <= total_size_.byte_size_round_down();
  }

  template<typename T>
  BitFieldResult get_bits(T &out, size_t nbits) noexcept {
    using U = typename unpack_t::underlying_or_self<T>::type;

    if (nbits == 0) {
      out = T(0);
      return BitFieldResult::Ok;
    }

    if (nbits > sizeof(U) * 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    if (!has(nbits)) {
      return BitFieldResult::ErrorExpectedMoreBits;
    }

    U acc = 0;
    size_t rem = nbits;

    while (rem) {
      uint8_t take = uint8_t(rem > 8 ? 8 : rem);
      uint8_t chunk = 0;
      auto r = get_bits_unchecked(chunk, take);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      acc = U((acc << take) | chunk);
      rem -= take;
    }

    out = static_cast<T>(acc);
    return BitFieldResult::Ok;
  }

  BitFieldResult get_bits(uint8_t &out, size_t nbits) noexcept {
    if (nbits == 0) {
       out = 0; 
       return BitFieldResult::Ok;
    }

    if (nbits > 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    if (!has(nbits)) {
      return BitFieldResult::ErrorExpectedMoreBits;
    }

    return get_bits_unchecked(out, nbits);
  }

  private:
  BitFieldResult get_bits_unchecked(uint8_t &out, size_t nbits) noexcept {
    uint8_t v = 0;
    uint8_t need = static_cast<uint8_t>(nbits);
    const size_t max_bytes = total_size_.byte_size_round_up();

    size_t idx = idx_;
    uint8_t pos = sub_byte_pos_;

    while (need) {
      if (idx >= max_bytes) {
        return BitFieldResult::ErrorExpectedMoreBits;
      }

      uint8_t cur = 0;
      auto r = buf_.try_get(idx, cur);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      const uint8_t avail = static_cast<uint8_t>(8 - pos);
      const uint8_t take  = (need < avail ? need : avail);

      const uint8_t shift = static_cast<uint8_t>(avail - take);
      const uint8_t mask  = static_cast<uint8_t>(((1u << take) - 1u) << shift);
      v = static_cast<uint8_t>((v << take) | ((cur & mask) >> shift));

      pos  = static_cast<uint8_t>(pos + take);
      need = static_cast<uint8_t>(need - take);

      if (pos == 8) {
        pos = 0; ++idx; 
      }
    }

    idx_ = idx;
    sub_byte_pos_ = pos;
    out = v;
    return BitFieldResult::Ok;
  }
};

#if SUB8_ENABLE_BOUNDED_BUF

template<size_t N>
class BoundedByteBuffer {
  uint8_t data_[N]{};
  size_t size_ = 0;
  bool overflow_ = false;

 public:
  using value_type = uint8_t;

  BoundedByteBuffer() noexcept = default;

  BoundedByteBuffer(std::initializer_list<uint8_t> init) noexcept : size_(0), overflow_(false) {
    for (auto v : init) {
      if (size_ < N) {
        data_[size_++] = v;
      } else {
        overflow_ = true;
        break;
      }
    }
  }

  constexpr size_t capacity() const noexcept { return N; }
  constexpr size_t size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }
  constexpr bool ok() const noexcept { return !overflow_; }
  uint8_t *data() noexcept { return data_; }
  const uint8_t *data() const noexcept { return data_; }

  void clear() noexcept {
    size_ = 0;
    overflow_ = false;
  }

  BitFieldResult reserve(size_t n) noexcept {
    if (n <= N) return BitFieldResult::Ok;
    overflow_ = true; 
    return BitFieldResult::ErrorOversizedLength;
  }

  BitFieldResult push_back(uint8_t v) noexcept {
    if (size_ < N) {
      data_[size_++] = v;
      return BitFieldResult::Ok;
    }
    overflow_ = true;
    return BitFieldResult::ErrorInsufficentBufferSize;
  }

  BitFieldResult back_mut_ptr(uint8_t *&out) noexcept {
    if (size_ == 0) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }

  BitFieldResult back_ptr(const uint8_t *&out) const noexcept {
    if (size_ == 0) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }

  uint8_t at(size_t i) const noexcept {
    uint8_t out;
    auto r = try_get(i, out);
    if(r != BitFieldResult::Ok) {
      return 0; // to maintain noexcept, must return 0
    }
    return out;
  }

  BitFieldResult try_get(size_t i, uint8_t &out) const noexcept {
    if (i >= size_) {
      out = 0;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = data_[i];
    return BitFieldResult::Ok;
  }
};

/*
template<size_t N> class BoundedByteBuffer {
  using value_type = uint8_t;

  uint8_t data_[N];
  size_t size_ = 0;
  bool overflow_ = false;

 public:
  BoundedByteBuffer(std::initializer_list<uint8_t> init) noexcept : size_(0), overflow_(false) {
    for (auto v : init) {
      if (size_ < N) {
        data_[size_] = v;
        size_++;
      } else {
        // Too many elements -> mark overflow and stop
        overflow_ = true;
        break;
      }
    }
  }

  BoundedByteBuffer() noexcept = default;

  constexpr size_t capacity() const noexcept { return N; }

  constexpr size_t size() const noexcept { return size_; }

  constexpr bool empty() const noexcept { return size_ == 0; }

  uint8_t *data() noexcept { return data_; }

  const uint8_t *data() const noexcept { return data_; }

  constexpr bool ok() const noexcept { return !overflow_; }

  void clear() noexcept {
    size_ = 0;
    overflow_ = false;
  }

  inline bool push_back(uint8_t v) noexcept {
    if (size_ < N) {
      data_[size_++] = v;
      return true;
    } else {
      overflow_ = true;
      return false;
    }
  }

  uint8_t &back() noexcept {
    assert(size_ > 0);
    return data_[size_ - 1];
  }

  const uint8_t &back() const noexcept {
    assert(size_ > 0);
    return data_[size_ - 1];
  }

  uint8_t &operator[](size_t idx) noexcept {
    assert(idx < size_);
    return data_[idx];
  }

  const uint8_t &operator[](size_t idx) const noexcept {
    assert(idx < size_);
    return data_[idx];
  }
};
*/
template<size_t N> using BoundedBitWriter = BasicBitWriter<BoundedByteBuffer<N>>;

template<size_t N> using BoundedBitReader = BasicBitReader<BoundedByteBuffer<N>>;
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF

class UnboundedByteBuffer {
  std::vector<uint8_t> buf_;

 public:
  using value_type = uint8_t;
  UnboundedByteBuffer() noexcept = default;
  explicit UnboundedByteBuffer(size_t reserve) { buf_.reserve(reserve); }  // can throw STL errors
  UnboundedByteBuffer(std::initializer_list<uint8_t> init) : buf_(init) {} // can throw STL errors

  size_t capacity() const noexcept { return buf_.capacity(); }
  size_t size() const noexcept { return buf_.size(); }
  bool empty() const noexcept { return buf_.empty(); }
  constexpr bool ok() const noexcept { return true; }

  uint8_t *data() noexcept { return buf_.data(); }
  const uint8_t *data() const noexcept { return buf_.data(); }

  void clear() noexcept { buf_.clear(); }

  BitFieldResult reserve(size_t n) noexcept {
    try {
      buf_.reserve(n);
      return BitFieldResult::Ok;
    } catch (const std::length_error &) {
      return BitFieldResult::ErrorOversizedLength;
    } catch (const std::bad_alloc &) {
      return BitFieldResult::ErrorBadAlloc;
    } catch (...) {
      // Will only occur if you have some non STL
      return BitFieldResult::ErrorUnidentifiedError;
    }
  }

  BitFieldResult push_back(uint8_t v) noexcept {
    try {
      buf_.push_back(v);
      return BitFieldResult::Ok;
    } catch (const std::bad_alloc &) {
      return BitFieldResult::ErrorBadAlloc;
    } catch (...) {
      // Will only occur if you have some non STL
      return BitFieldResult::ErrorUnidentifiedError;
    }
  }

  BitFieldResult back_mut_ptr(uint8_t *&out) noexcept {
    if (buf_.empty()) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &buf_.back();
    return BitFieldResult::Ok;
  }

  BitFieldResult back_ptr(const uint8_t *&out) const noexcept {
    if (buf_.empty()) {
      out = nullptr;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = &buf_.back();
    return BitFieldResult::Ok;
  }

  uint8_t at(size_t i) const noexcept {
    uint8_t out;
    auto r = try_get(i, out);
    if(r != BitFieldResult::Ok) {
      return 0; // to maintain noexcept, must return 0
    }
    return out;
  }

  BitFieldResult try_get(size_t i, uint8_t &out) const noexcept {
    if (i >= buf_.size()) {
      out = 0;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = buf_[i];
    return BitFieldResult::Ok;
  }
};

/*
class UnboundedByteBuffer {
  std::vector<uint8_t> buf_;

 public:
  UnboundedByteBuffer() {}
  UnboundedByteBuffer(size_t reserve) { buf_.reserve(reserve); }
  UnboundedByteBuffer(std::initializer_list<uint8_t> init) : buf_(init) {}

  void reserve(size_t reserve) { buf_.reserve(reserve); }
  size_t capacity() const noexcept { return buf_.capacity(); }

  size_t size() const noexcept { return buf_.size(); }

  bool empty() const noexcept { return buf_.empty(); }

  constexpr bool ok() const noexcept { return true; }

  uint8_t *data() noexcept { return buf_.data(); }

  const uint8_t *data() const noexcept { return buf_.data(); }

  void clear() noexcept { buf_.clear(); }

  bool push_back(uint8_t v) {
    buf_.push_back(v);
    return true;
  }

  uint8_t &back() noexcept {
    assert(!buf_.empty());
    return buf_.back();
  }

  const uint8_t &back() const noexcept {
    assert(!buf_.empty());
    return buf_.back();
  }

  uint8_t &operator[](size_t i) noexcept {
    assert(i < buf_.size());
    return buf_[i];
  }

  const uint8_t &operator[](size_t i) const noexcept {
    assert(i < buf_.size());
    return buf_[i];
  }
};*/

using UnboundedBitWriter = BasicBitWriter<UnboundedByteBuffer>;
using UnboundedBitReader = BasicBitReader<UnboundedByteBuffer>;

#endif

}  // namespace sub8