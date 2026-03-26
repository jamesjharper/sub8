#pragma once
#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_errors.h"
#include "sub8_type_information.h"
#endif

#include <cassert>

// Feature flags":
// ===============

// Supported read/write buffer types
// ---------------------------------

// Enable: Support for buffers with known compile time size
#ifndef SUB8_ENABLE_BOUNDED_BUF
#define SUB8_ENABLE_BOUNDED_BUF 1
#endif

// Enable: Support heap based buffers (uses vectors internally)
#ifndef SUB8_ENABLE_UNBOUNDED_BUF
#define SUB8_ENABLE_UNBOUNDED_BUF 1
#endif

// Enable: Support pointer based buffers.
#ifndef SUB8_ENABLE_VIEW_BUF
#define SUB8_ENABLE_VIEW_BUF 1
#endif

#if SUB8_ENABLE_UNBOUNDED_BUF
#include <vector>
#endif

namespace sub8 {

namespace packing {

template <typename T> using underlying_or_self_t = typename unpack_t::underlying_or_self<T>::type;

template <typename T> using packed_t = std::make_unsigned_t<underlying_or_self_t<T>>;

template <typename S> using make_unsigned_t = typename std::make_unsigned<S>::type;

template <typename S> constexpr make_unsigned_t<S> zigzag_encode(S x) noexcept {
  static_assert(std::is_signed<S>::value, "zigzag::encode expects signed type");
  using U = make_unsigned_t<S>;
  return (static_cast<U>(x) << 1) ^ static_cast<U>(x >> (std::numeric_limits<S>::digits));
}

template <typename S> constexpr S zigzag_decode(make_unsigned_t<S> u) noexcept {
  static_assert(std::is_signed<S>::value, "zigzag::decode expects signed type");
  using U = make_unsigned_t<S>;
  return static_cast<S>((u >> 1) ^ static_cast<U>(-static_cast<typename std::make_signed<U>::type>(u & 1)));
}

template <typename T> constexpr packed_t<T> pack(T v) noexcept {
  using U = underlying_or_self_t<T>;
  using P = packed_t<T>;

  if constexpr (std::is_signed_v<U>) {
    return static_cast<P>(zigzag_encode<U>(static_cast<U>(v)));
  } else {
    return static_cast<P>(static_cast<U>(v));
  }
}

template <typename T> constexpr T unpack(packed_t<T> code) noexcept {
  using U = underlying_or_self_t<T>;

  if constexpr (std::is_signed_v<U>) {
    const U v = zigzag_decode<U>(code);
    return static_cast<T>(v);
  } else {
    return static_cast<T>(static_cast<U>(code));
  }
}

} // namespace packing

// Bit buffers
// =======================

// Object for addressing bit buffer lengths and position
class BitSize {
  size_t byte_size_ = 0;      // whole bytes
  uint8_t bit_remainder_ = 0; // 0..7 extra bits beyond whole bytes

  // Keeps invariant: bit_remainder_ in [0,7] by carrying into bytes.
  constexpr void normalize() noexcept {
    if (bit_remainder_ >= 8) {
      byte_size_ += static_cast<size_t>(bit_remainder_ / 8);
      bit_remainder_ = static_cast<uint8_t>(bit_remainder_ % 8);
    }
  }

public:
  constexpr BitSize() noexcept = default;

  constexpr BitSize(size_t byte_size, uint8_t bit_remainder) noexcept : byte_size_{byte_size}, bit_remainder_{bit_remainder} {
    normalize();
  }

  static constexpr BitSize from_bytes(size_t bytes) noexcept { return BitSize{bytes, 0}; }

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

  constexpr BitSize &operator+=(const BitSize &other) noexcept {
    byte_size_ += other.byte_size_;
    bit_remainder_ = static_cast<uint8_t>(bit_remainder_ + other.bit_remainder_);
    normalize();
    return *this;
  }

  friend constexpr BitSize operator-(BitSize a, const BitSize &b) noexcept {
    a -= b;
    return a;
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

  friend constexpr BitSize operator*(BitSize a, const size_t b) noexcept {
    a *= b;
    return a;
  }

  constexpr BitSize &operator*=(const size_t b) noexcept {
    const size_t bits = static_cast<size_t>(bit_remainder_) * b;
    byte_size_ = byte_size_ * b + (bits / 8u);
    bit_remainder_ = static_cast<uint8_t>(bits % 8u);
    return *this;
  }
};

template <typename Storage> 
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

  template <typename T> BitFieldResult put_bits(T value, uint8_t nbits) noexcept {
    using U = typename unpack_t::underlying_or_self<T>::type;
    static_assert(std::is_unsigned<U>::value, "BitWriter::put_bits expects unsigned values");

    if (nbits == 0) {
      return BitFieldResult::Ok;
    }

    const U v = static_cast<U>(value);
    if (nbits <= 8) {
      return put_bits(static_cast<uint8_t>(v), static_cast<uint8_t>(nbits));
    }

    if (nbits > sizeof(U) * 8) {
      return BitFieldResult::ErrorTooManyBits;
    }

    size_t remaining = nbits;
    while (remaining) {
      const uint8_t take = static_cast<uint8_t>(remaining >= 8 ? 8 : remaining);
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
    if (nbits == 0) {
      return BitFieldResult::Ok;
    }

    if (nbits > 8) {
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

      if (br != BitFieldResult::Ok) 
        return br;

      if (dst == nullptr) 
      return BitFieldResult::ErrorUnidentifiedError;

      *dst |= static_cast<uint8_t>(chunk << (remaining_space - take));

      pos = static_cast<uint8_t>(pos + take);
      nbits = static_cast<uint8_t>(nbits - take);

      if (pos == 8)
        pos = 0;
    }

    sub_byte_pos_ = pos;
    return BitFieldResult::Ok;
  }
};

template <typename Storage>
class BasicBitReader {
  Storage *buf_ = nullptr;

  // Inline owned storage for the rvalue case.
  using OwnedStorage = typename std::aligned_storage<sizeof(Storage), alignof(Storage)>::type;
  bool owns_ = false;
  OwnedStorage owned_{};

  BitSize total_size_{};
  size_t idx_ = 0;
  uint8_t sub_byte_pos_ = 0;

  Storage *owned_ptr() noexcept {
#if __cplusplus >= 201703L
    return std::launder(reinterpret_cast<Storage *>(&owned_));
#else
    return reinterpret_cast<Storage *>(&owned_);
#endif
  }
  const Storage *owned_ptr() const noexcept {
#if __cplusplus >= 201703L
    return std::launder(reinterpret_cast<const Storage *>(&owned_));
#else
    return reinterpret_cast<const Storage *>(&owned_);
#endif
  }

public:
  // Borrowing ctor (no copies)
  explicit BasicBitReader(Storage &data) noexcept : buf_(&data), total_size_(buf_->size()) {}

  // Borrowing ctor with explicit bit_size
  BasicBitReader(Storage &data, BitSize bit_size) noexcept : buf_(&data), total_size_(bit_size) {
    assert(total_size_.byte_size_round_up() <= buf_->size());
  }

  // Owning ctor (stores Storage by value inside this reader)
  explicit BasicBitReader(Storage &&data) noexcept(std::is_nothrow_move_constructible_v<Storage>)
      : buf_(nullptr), owns_(true) {
    ::new (&owned_) Storage(std::move(data));
    buf_ = owned_ptr();
    total_size_ = BitSize::from_bytes(buf_->size());
  }

  // Owning ctor with explicit bit_size
  BasicBitReader(Storage &&data, BitSize bit_size) noexcept(std::is_nothrow_move_constructible_v<Storage>)
      : buf_(nullptr), owns_(true), total_size_(bit_size) {
    ::new (&owned_) Storage(std::move(data));
    buf_ = owned_ptr();
    assert(total_size_.byte_size_round_up() <= buf_->size());
  }

  // No copying (safe and simple)
  BasicBitReader(const BasicBitReader &) = delete;
  BasicBitReader &operator=(const BasicBitReader &) = delete;

  // Move support
  BasicBitReader(BasicBitReader &&other) noexcept(std::is_nothrow_move_constructible_v<Storage>) {
    *this = std::move(other);
  }

  BasicBitReader &operator=(BasicBitReader &&other) noexcept(std::is_nothrow_move_constructible_v<Storage>) {
    if (this == &other) {
      return *this;
    }

    // Clean up existing owned storage
    if (owns_) {
      owned_ptr()->~Storage();
      owns_ = false;
      buf_ = nullptr;
    }


    
    total_size_ = other.total_size_;
    idx_ = other.idx_;
    sub_byte_pos_ = other.sub_byte_pos_;

    if (other.owns_) {
      // Move-construct our owned Storage from theirs
      ::new (&owned_) Storage(std::move(*other.owned_ptr()));
      buf_ = owned_ptr();
      owns_ = true;

      // Destroy other's owned storage and mark as non-owning
      other.owned_ptr()->~Storage();
      other.owns_ = false;
      other.buf_ = nullptr;

    } else {
      buf_ = other.buf_;
      owns_ = false;
      // Make the "other" buffer inert
      other.buf_ = nullptr;
      other.total_size_ = BitSize{};
    }
    other.idx_ = 0;
    other.sub_byte_pos_ = 0;

    return *this;
  }

  ~BasicBitReader() noexcept {
    if (owns_) {
      owned_ptr()->~Storage();
    }
  }

  Storage &storage() noexcept { return *buf_; }
  const Storage &storage() const noexcept { return *buf_; }
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

    return has_bytes == total_size_.byte_size_round_down() ? has_bits <= total_size_.bit_remainder()
                                                           : has_bytes <= total_size_.byte_size_round_down();
  }

  template <typename T> BitFieldResult get_bits(T &out, size_t nbits) noexcept {
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
      auto r = buf_->try_get(idx, cur);
      if (r != BitFieldResult::Ok) {
        return r;
      }

      const uint8_t avail = static_cast<uint8_t>(8 - pos);
      const uint8_t take = (need < avail ? need : avail);

      const uint8_t shift = static_cast<uint8_t>(avail - take);
      const uint8_t mask = static_cast<uint8_t>(((1u << take) - 1u) << shift);
      v = static_cast<uint8_t>((v << take) | ((cur & mask) >> shift));

      pos = static_cast<uint8_t>(pos + take);
      need = static_cast<uint8_t>(need - take);

      if (pos == 8) {
        pos = 0;
        ++idx;
      }
    }

    idx_ = idx;
    sub_byte_pos_ = pos;
    out = v;
    return BitFieldResult::Ok;
  }
};

#if SUB8_ENABLE_BOUNDED_BUF

template <size_t N> class BoundedByteBuffer {
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
    if (n <= N)
      return BitFieldResult::Ok;
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
    if (r != BitFieldResult::Ok) {
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

template <size_t N> using BoundedBitWriter = BasicBitWriter<BoundedByteBuffer<N>>;

template <size_t N> using BoundedBitReader = BasicBitReader<BoundedByteBuffer<N>>;

template <typename MessageType> inline constexpr size_t recommended_bounded_buffer_byte_size() noexcept {
  return MessageType::MaxPossibleSize.byte_size_round_up();
}

template <typename MessageType> using BoundedWriterFor = sub8::BoundedBitWriter<recommended_bounded_buffer_byte_size<MessageType>()>;

template <typename MessageType> using BoundedReaderFor = sub8::BoundedBitReader<recommended_bounded_buffer_byte_size<MessageType>()>;

template <typename MessageType> inline BoundedWriterFor<MessageType> make_bounded_writer_for() noexcept {
  return BoundedWriterFor<MessageType>{};
}

template <typename MessageType> inline BoundedReaderFor<MessageType> make_bounded_reader_for() noexcept {
  return BoundedReaderFor<MessageType>{};
}

#endif

#if SUB8_ENABLE_VIEW_BUF

class ByteBufferView {
  uint8_t *data_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
  bool overflow_ = false;

public:
  constexpr ByteBufferView() noexcept = default;

  constexpr ByteBufferView(uint8_t *data, size_t size) noexcept : data_(data), size_(size), capacity_(size) {}

  constexpr ByteBufferView(uint8_t *data, size_t size, size_t capacity) noexcept : data_(data), size_(size), capacity_(capacity) {}

  constexpr size_t capacity() const noexcept { return capacity_; }
  constexpr size_t size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }
  constexpr bool ok() const noexcept { return !overflow_; }

  constexpr uint8_t *data() noexcept { return data_; }
  constexpr const uint8_t *data() const noexcept { return data_; }

  void clear() noexcept {
    size_ = 0;
    overflow_ = false;
  }

  BitFieldResult reserve(size_t n) noexcept {
    if (n <= capacity_)
      return BitFieldResult::Ok;
    overflow_ = true;
    return BitFieldResult::ErrorOversizedLength;
  }

  BitFieldResult push_back(uint8_t v) noexcept {
    if (size_ < capacity_) {
      data_[size_++] = v;
      return BitFieldResult::Ok;
    }
    overflow_ = true;
    return BitFieldResult::ErrorInsufficentBufferSize;
  }

  BitFieldResult try_get(size_t i, uint8_t &out) const noexcept {
    if (i >= size_) {
      out = 0;
      return BitFieldResult::ErrorExpectedMoreBits;
    }
    out = data_[i];
    return BitFieldResult::Ok;
  }

  BitFieldResult back_mut_ptr(uint8_t*& out) noexcept {
    if (size_ == 0) { out = nullptr; return BitFieldResult::ErrorExpectedMoreBits; }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }

  BitFieldResult back_ptr(const uint8_t*& out) const noexcept {
    if (size_ == 0) { out = nullptr; return BitFieldResult::ErrorExpectedMoreBits; }
    out = &data_[size_ - 1];
    return BitFieldResult::Ok;
  }
};

using BitWriter = BasicBitWriter<ByteBufferView>;
using BitReader = BasicBitReader<ByteBufferView>;


inline BitReader make_reader(uint8_t *data, BitSize size) noexcept {
  ByteBufferView view(data, size.byte_size_round_up());
  return BitReader(std::move(view), size); 
}

#endif

#if SUB8_ENABLE_UNBOUNDED_BUF

class UnboundedByteBuffer {
  std::vector<uint8_t> buf_;

public:
  using value_type = uint8_t;
  UnboundedByteBuffer() noexcept = default;
  explicit UnboundedByteBuffer(size_t reserve) { buf_.reserve(reserve); } // can throw STL errors
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
    if (r != BitFieldResult::Ok) {
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

using UnboundedBitWriter = BasicBitWriter<UnboundedByteBuffer>;
using UnboundedBitReader = BasicBitReader<UnboundedByteBuffer>;

template <typename MessageType> constexpr size_t recommended_unbounded_buffer_byte_size(uint8_t percentage_0_100) noexcept {
  const size_t min_b = MessageType::MinPossibleSize.byte_size_round_up();
  const size_t max_b = MessageType::MaxPossibleSize.byte_size_round_up();

  if (max_b <= min_b)
    return min_b;

  const size_t delta = max_b - min_b;

  const size_t p = (percentage_0_100 > 100u) ? 100u : size_t{percentage_0_100};
  return min_b + (delta * p) / 100u;
}

template <typename MessageType> inline sub8::BitFieldResult init_unbounded_writer_for(sub8::UnboundedBitWriter &out,
  uint8_t reserve_percentage_0_100 = 0) noexcept {
  out.storage().clear();
  const size_t reserve_bytes = recommended_unbounded_buffer_byte_size<MessageType>(reserve_percentage_0_100);
  return out.storage().reserve(reserve_bytes);
}

#endif

} // namespace sub8