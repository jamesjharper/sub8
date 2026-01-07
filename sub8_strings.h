#pragma once

// Enable: 5bit String Fields
// 5bit strings which us control chars to shift between character sheets and enable a extended 10bit encoding mode.
#ifndef SUB8_ENABLE_FIVE_BIT_STRING
#define SUB8_ENABLE_FIVE_BIT_STRING 1
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS
// Auto enable foundational string type
#if SUB8_ENABLE_FIVE_BIT_STRING
#define SUB8_ENABLE_STRING_FIELDS 1
#else
#define SUB8_ENABLE_STRING_FIELDS 0
#endif
#endif

#if SUB8_ENABLE_STRING_FIELDS
// Enable / Disable specific characters widths
#ifndef SUB8_ENABLE_STRING_FIELDS__CHAR
#define SUB8_ENABLE_STRING_FIELDS__CHAR 1
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__WCHAR
#define SUB8_ENABLE_STRING_FIELDS__WCHAR 1
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__U8_CHAR
#if __cplusplus >= 202002L
#define SUB8_ENABLE_STRING_FIELDS__U8_CHAR 1
#else
#define SUB8_ENABLE_STRING_FIELDS__U8_CHAR 0
#endif
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__U16_CHAR
#if __cplusplus >= 201103L
#define SUB8_ENABLE_STRING_FIELDS__U16_CHAR 1
#else
#define SUB8_ENABLE_STRING_FIELDS__U16_CHAR 0
#endif
#endif

#ifndef SUB8_ENABLE_STRING_FIELDS__U32_CHAR
#if __cplusplus >= 201103L
#define SUB8_ENABLE_STRING_FIELDS__U32_CHAR 1
#else
#define SUB8_ENABLE_STRING_FIELDS__U32_CHAR 0
#endif
#endif
#else
#define SUB8_ENABLE_STRING_FIELDS__CHAR 0
#define SUB8_ENABLE_STRING_FIELDS__WCHAR 0
#define SUB8_ENABLE_STRING_FIELDS__U8_CHAR 0
#define SUB8_ENABLE_STRING_FIELDS__U16_CHAR 0
#define SUB8_ENABLE_STRING_FIELDS__U32_CHAR 0
#endif

#if SUB8_ENABLE_STRING_FIELDS

#include "sub8.h"
#include "sub8_errors.h"
#include "sub8_io.h"

#include <uchar.h>

namespace sub8 {
namespace utf {
template<typename CharT> struct UtfMaxUnits;

#if SUB8_ENABLE_STRING_FIELDS__CHAR
template<> struct UtfMaxUnits<char> {
  static constexpr size_t value = sizeof(char) / 4;
};
#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
template<> struct UtfMaxUnits<wchar_t> {
  static constexpr size_t value = (sizeof(wchar_t) == 2 ? 2 : 1);
};
#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR
struct UtfMaxUnits<char8_t> {
  static constexpr size_t value = 4;
};
#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR
template<> struct UtfMaxUnits<char16_t> {
  static constexpr size_t value = 2;  // StorageTypeF-16
};
#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR
template<> struct UtfMaxUnits<char32_t> {
  static constexpr size_t value = 1;  // StorageTypeF-32
};
#endif

template<typename T> size_t encode_codepoint_to_utf(char32_t cp, T *out) noexcept;

template<> inline size_t encode_codepoint_to_utf<char>(char32_t cp, char *out) noexcept {
  // Reject invalid code points (surrogates / > U+10FFFF)
  if (cp > 0x10FFFFu) {
    return 0;
  }
  if (cp >= 0xD800u && cp <= 0xDFFFu) {
    return 0;  // StorageTypeF-16 surrogate range, not a Unicode scalar value
  }

  if (cp <= 0x7Fu) {
    out[0] = static_cast<uint8_t>(cp);
    return 1;
  }
  if (cp <= 0x7FFu) {
    out[0] = static_cast<uint8_t>(0xC0u | ((cp >> 6) & 0x1Fu));
    out[1] = static_cast<uint8_t>(0x80u | (cp & 0x3Fu));
    return 2;
  }
  if (cp <= 0xFFFFu) {
    out[0] = static_cast<uint8_t>(0xE0u | ((cp >> 12) & 0x0Fu));
    out[1] = static_cast<uint8_t>(0x80u | ((cp >> 6) & 0x3Fu));
    out[2] = static_cast<uint8_t>(0x80u | (cp & 0x3Fu));
    return 3;
  }

  // 4-byte sequence
  out[0] = static_cast<uint8_t>(0xF0u | ((cp >> 18) & 0x07u));
  out[1] = static_cast<uint8_t>(0x80u | ((cp >> 12) & 0x3Fu));
  out[2] = static_cast<uint8_t>(0x80u | ((cp >> 6) & 0x3Fu));
  out[3] = static_cast<uint8_t>(0x80u | (cp & 0x3Fu));
  return 4;
}

template<> inline size_t encode_codepoint_to_utf<char16_t>(char32_t cp, char16_t *out) noexcept {
  // Reject invalid code points (surrogates / > U+10FFFF)
  if (cp > 0x10FFFFu) {
    return 0;
  }
  if (cp >= 0xD800u && cp <= 0xDFFFu) {
    return 0;  // stand-alone surrogates invalid as scalar values
  }

  if (cp <= 0xFFFFu) {
    out[0] = static_cast<uint16_t>(cp);
    return 1;
  }

  // Encode surrogate pair
  char32_t v = cp - 0x10000u;
  uint16_t high = static_cast<uint16_t>(0xD800u + ((v >> 10) & 0x3FFu));
  uint16_t low = static_cast<uint16_t>(0xDC00u + (v & 0x3FFu));

  out[0] = high;
  out[1] = low;
  return 2;
}

template<> inline size_t encode_codepoint_to_utf<char32_t>(char32_t cp, char32_t *out) noexcept {
  // Reject invalid code points (surrogates / > U+10FFFF)
  if (cp > 0x10FFFFu) {
    return 0;
  }
  if (cp >= 0xD800u && cp <= 0xDFFFu) {
    return 0;
  }

  out[0] = static_cast<uint32_t>(cp);
  return 1;
}

// Codepoint iterator is used to abstract the underlying storage,
// ie UTF-8/16/32 from the encoding layer which only cares about individual code points
class UtfCodepointIterator {
 public:
  enum class Encoding : uint8_t { Utf8, Utf16, Utf32 };

  UtfCodepointIterator() noexcept : encoding_(Encoding::Utf8), start_(nullptr), cur_(nullptr), end_(nullptr) {}

#if SUB8_ENABLE_STRING_FIELDS__CHAR
  explicit UtfCodepointIterator(const char *utf8_z, size_t len) noexcept
      : encoding_(Encoding::Utf8), start_(utf8_z), cur_(utf8_z), end_(utf8_z ? utf8_z + len : utf8_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
  explicit UtfCodepointIterator(const wchar_t *utf_wide_z, size_t len) noexcept
      : encoding_((sizeof(wchar_t) == 2 ? Encoding::Utf16 : Encoding::Utf32)),
        start_(utf_wide_z),
        cur_(utf_wide_z),
        end_(utf_wide_z ? utf_wide_z + len : utf_wide_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR
  explicit UtfCodepointIterator(const char8_t *utf8_z, size_t len) noexcept
      : encoding_(Encoding::Utf8), start_(utf8_z), cur_(utf8_z), end_(utf8_z ? utf8_z + len : utf8_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR
  explicit UtfCodepointIterator(const char16_t *utf16_z, size_t len) noexcept
      : encoding_(Encoding::Utf16), start_(utf16_z), cur_(utf16_z), end_(utf16_z ? utf16_z + len : utf16_z) {}
#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR
  explicit UtfCodepointIterator(const char32_t *utf32_z, size_t len) noexcept
      : encoding_(Encoding::Utf32), start_(utf32_z), cur_(utf32_z), end_(utf32_z ? utf32_z + len : utf32_z) {}
#endif

  void reset() noexcept { cur_ = start_; }

  // Advance and decode next codepoint.
  bool try_get_next_utf32_char(char32_t &out_cp) noexcept {
    switch (encoding_) {
      case Encoding::Utf8:
        return decode_utf8(out_cp, /* advance_cur */ true);
      case Encoding::Utf16:
        return decode_utf16(out_cp, /* advance_cur */ true);
      case Encoding::Utf32:
        return decode_utf32(out_cp, /* advance_cur */ true);
      default:
        return false;
    }
  }

  bool try_peek_next_utf32_char(char32_t &out_cp) noexcept {
    switch (encoding_) {
      case Encoding::Utf8:
        return decode_utf8(out_cp, /* advance_cur */ false);
      case Encoding::Utf16:
        return decode_utf16(out_cp, /* advance_cur */ false);
      case Encoding::Utf32:
        return decode_utf32(out_cp, /* advance_cur */ false);
      default:
        return false;
    }
  }

 private:
  Encoding encoding_;
  const void *start_{nullptr};
  const void *cur_{nullptr};
  const void *end_{nullptr};

  // ---- StorageTypeF-8 decoding ----
  bool decode_utf8(char32_t &out_cp, bool advance_cur = true) noexcept {
    const char *p = static_cast<const char *>(cur_);
    const char *end = static_cast<const char *>(end_);

    if (!p || !end || p >= end) {
      return false;
    }

    const unsigned char *s = reinterpret_cast<const unsigned char *>(p);
    unsigned char b0 = *s;

    // Helper for continuation bytes
    auto is_cont = [](unsigned char b) noexcept { return (b & 0xC0u) == 0x80u; };

    // 1-byte (ASCII)
    if (b0 < 0x80u) {
      out_cp = static_cast<char32_t>(b0);
      p = reinterpret_cast<const char *>(s + 1);
      if (advance_cur)
        cur_ = p;
      return true;
    }

    // Determine sequence length
    int len = 0;
    char32_t cp = 0;

    if ((b0 & 0xE0u) == 0xC0u) {  // 110xxxxx -> 2-byte
      len = 2;
      cp = b0 & 0x1Fu;
    } else if ((b0 & 0xF0u) == 0xE0u) {  // 1110xxxx -> 3-byte
      len = 3;
      cp = b0 & 0x0Fu;
    } else if ((b0 & 0xF8u) == 0xF0u) {  // 11110xxx -> 4-byte
      len = 4;
      cp = b0 & 0x07u;
    } else {
      // Invalid leading byte: replacement, advance one byte
      out_cp = 0xFFFDu;
      p = reinterpret_cast<const char *>(s + 1);
      if (p > end)
        p = end;
      if (advance_cur)
        cur_ = p;
      return true;
    }

    // Ensure we have enough bytes
    if (p + len > end) {
      // truncated sequence at end
      out_cp = 0xFFFDu;
      if (advance_cur)
        cur_ = end;
      return true;
    }

    const unsigned char *q = s + 1;
    for (int i = 1; i < len; ++i) {
      unsigned char bx = q[i - 1];
      if (!is_cont(bx)) {
        // Invalid continuation -> replacement, skip just the first byte
        out_cp = 0xFFFDu;
        p = reinterpret_cast<const char *>(s + 1);
        if (p > end)
          p = end;
        if (advance_cur)
          cur_ = p;
        return true;
      }
      cp = (cp << 6) | (bx & 0x3Fu);
    }

    // Minimal form & range checks
    if ((len == 2 && cp < 0x80u) || (len == 3 && cp < 0x800u) || (len == 4 && cp < 0x10000u) || cp > 0x10FFFFu ||
        (cp >= 0xD800u && cp <= 0xDFFFu)) {
      out_cp = 0xFFFDu;
    } else {
      out_cp = cp;
    }

    p = reinterpret_cast<const char *>(s + len);
    if (p > end)
      p = end;
    if (advance_cur)
      cur_ = p;
    return true;
  }

  // ---- StorageTypeF-16 decoding ----
  bool decode_utf16(char32_t &out_cp, bool advance_cur = true) noexcept {
    const char16_t *p = static_cast<const char16_t *>(cur_);
    const char16_t *end = static_cast<const char16_t *>(end_);

    if (!p || !end || p >= end) {
      return false;
    }

    char16_t w1 = *p++;
    // Single-unit BMP (non-surrogate)
    if (w1 < 0xD800u || w1 > 0xDFFFu) {
      out_cp = static_cast<char32_t>(w1);
      if (advance_cur)
        cur_ = p;
      return true;
    }

    // Surrogates
    if (w1 >= 0xD800u && w1 <= 0xDBFFu) {  // high surrogate
      if (p >= end) {
        out_cp = 0xFFFDu;  // truncated
        if (advance_cur)
          cur_ = end;
        return true;
      }
      char16_t w2 = *p;
      if (w2 >= 0xDC00u && w2 <= 0xDFFFu) {
        ++p;
        char32_t hi = static_cast<char32_t>(w1 - 0xD800u);
        char32_t lo = static_cast<char32_t>(w2 - 0xDC00u);
        out_cp = ((hi << 10) | lo) + 0x10000u;
        if (advance_cur)
          cur_ = p;
        return true;
      } else {
        // Isolated high surrogate
        out_cp = 0xFFFDu;
        if (advance_cur)
          cur_ = p;
        return true;
      }
    }

    // Isolated low surrogate
    out_cp = 0xFFFDu;
    if (advance_cur)
      cur_ = p;
    return true;
  }

  // ---- StorageTypeF-32 decoding ----
  bool decode_utf32(char32_t &out_cp, bool advance_cur = true) noexcept {
    const char32_t *p = static_cast<const char32_t *>(cur_);
    const char32_t *end = static_cast<const char32_t *>(end_);

    if (!p || !end || p >= end) {
      return false;
    }

    char32_t cp = *p++;
    if (cp > 0x10FFFFu || (cp >= 0xD800u && cp <= 0xDFFFu)) {
      out_cp = 0xFFFDu;
    } else {
      out_cp = cp;
    }

    if (advance_cur)
      cur_ = p;
    return true;
  }
};

template<typename T> struct UtfCodepointMapper {
  using value_type = T;
  static value_type map(char32_t cp) noexcept;
};

// Look ahead iterator is used to "look ahead" n code points. This is
// used to allow the encoding layer to factor for non-scalar glyphs
// and allow for encoding optimizations
template<typename Mapper, size_t LookAheadDistance = 3> class UtfCodepointLookAheadIteratorT {
 public:
  using value_type = typename Mapper::value_type;

  UtfCodepointLookAheadIteratorT() noexcept = default;

  explicit UtfCodepointLookAheadIteratorT(utf::UtfCodepointIterator iter) noexcept : iter_(iter) {
    fill_initial_buffer();
  }

  bool empty() const noexcept { return buf_len_ == 0; }

  void reset() noexcept {
    iter_.reset();
    buf_idx_ = 0;
    buf_len_ = 0;
    fill_initial_buffer();
  }

  bool try_pop(value_type &out_value) noexcept {
    if (buf_len_ == 0)
      return false;

    out_value = buf_[buf_idx_];
    advance_buffer(1);
    fill_to_capacity();
    return true;
  }

  bool try_peek_ahead(size_t offset, value_type &out_value) const noexcept {
    if (offset >= buf_len_)
      return false;

    out_value = buf_[(buf_idx_ + offset) % LookAheadDistance];
    return true;
  }

  bool try_peek_next(value_type &out_value) const noexcept { return try_peek_ahead(0, out_value); }

 private:
  void advance_buffer(size_t distance) noexcept {
    if (distance >= buf_len_) {
      buf_idx_ = 0;
      buf_len_ = 0;
      return;
    }

    buf_idx_ = (buf_idx_ + distance) % LookAheadDistance;
    buf_len_ -= static_cast<uint8_t>(distance);
  }

  void enqueue(const value_type &value) noexcept {
    buf_[(buf_idx_ + buf_len_) % LookAheadDistance] = value;

    if (buf_len_ < LookAheadDistance) {
      ++buf_len_;
    } else {
      buf_idx_ = (buf_idx_ + 1) % LookAheadDistance;
    }
  }

  void fill_initial_buffer() noexcept {
    for (size_t i = 0; i < LookAheadDistance; ++i) {
      value_type value{};
      if (!read_next(value))
        break;
      enqueue(value);
    }
  }

  void fill_to_capacity() noexcept {
    while (buf_len_ < LookAheadDistance) {
      value_type value{};
      if (!read_next(value))
        break;
      enqueue(value);
    }
  }

  bool read_next(value_type &out_value) noexcept {
    char32_t cp = 0;
    if (!iter_.try_get_next_utf32_char(cp))
      return false;

    out_value = Mapper::map(cp);
    return true;
  }

 private:
  utf::UtfCodepointIterator iter_{};

  value_type buf_[LookAheadDistance]{};  // <- now an array of B5CharAddress
  uint8_t buf_idx_ = 0;
  uint8_t buf_len_ = 0;
};

template<> struct UtfCodepointMapper<char32_t> {
  using value_type = char32_t;

  static value_type map(char32_t cp) noexcept { return cp; }
};

template<size_t LookAheadDistance = 3>
using UtfCodepointLookAheadIterator = UtfCodepointLookAheadIteratorT<UtfCodepointMapper<char32_t>, LookAheadDistance>;

}  // namespace utf

template<
    // CharT: underlying string charater type
    class CharT,

    // Encoder / Decoder: Text encoder to be use for transcoding the string
    typename Encoder, typename Decoder>
class BasicStringField {
 public:
  using Type = CharT;
  using ValueType = std::basic_string<CharT>;
  using InitType = std::basic_string<CharT>;

  using StorageType = typename unpack_t::underlying_or_self<CharT>::type;
  using EncoderType = Encoder;
  using DecoderType = Decoder;

  BasicStringField() noexcept = default;
  BasicStringField(const BasicStringField &) noexcept = default;
  BasicStringField(const std::basic_string<CharT> &init) noexcept { set_value(init); }

  std::basic_string<CharT> data_;

  size_t size() const noexcept { return data_.size(); }

  const std::basic_string<CharT> &value() const noexcept { return data_; }

  std::basic_string<CharT> &value() noexcept { return data_; }

  explicit operator const std::basic_string<CharT> &() const noexcept { return data_; }
  bool operator==(const BasicStringField &o) const noexcept { return data_ == o.data_; }
  bool operator!=(const BasicStringField &o) const noexcept { return !(*this == o); }

  BasicStringField &operator=(const BasicStringField &) noexcept = default;

  BasicStringField &operator=(const std::basic_string<CharT> &s) noexcept {
    auto r = set_value(s);
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  BasicStringField &operator=(const CharT *cstr) noexcept {
    auto r = set_value(std::basic_string<CharT>(cstr));
    assert(r == BitFieldResult::Ok);
    (void) r;
    return *this;
  }

  void clear() noexcept { data_.clear(); }

  BitFieldResult set_value(const std::basic_string<CharT> &s) noexcept {
    data_ = s;
    return BitFieldResult::Ok;
  }

  // Expose raw data for write_field
  const CharT *data_ptr() const noexcept { return data_.data(); }
  size_t data_len() const noexcept { return data_.size(); }
};

template<typename Storage, class CharT, typename Encoder, typename Decoder>
inline BitFieldResult write_field(BasicBitWriter<Storage> &bw, const BasicStringField<CharT, Encoder, Decoder> &field) {
  Encoder enc;
  utf::UtfCodepointIterator it(field.data_ptr(), field.data_len());

  auto init_result = enc.init(it);
  if (init_result != BitFieldResult::Ok) {
    return init_result;
  }

  uint32_t code = 0;
  uint8_t bit_len = 0;

  while (enc.try_encode(code, bit_len)) {
    if (bit_len == 0)
      continue;
    auto r = bw.put_bits(code, bit_len);
    if (r != BitFieldResult::Ok) {
      return r;
    }
  }

  return BitFieldResult::Ok;
}

template<typename Storage, class CharT, typename Encoder, typename Decoder>
inline BitFieldResult read_field(BasicBitReader<Storage> &br, BasicStringField<CharT, Encoder, Decoder> &out) {
  out.clear();

  Decoder dec;
  auto init_result = dec.init();
  if (init_result != BitFieldResult::Ok) {
    return init_result;
  }

  char32_t cp = 0;
  CharT buf[utf::UtfMaxUnits<CharT>::value];

  while (!dec.end_of_sequence()) {
    auto expected_bits = dec.expected_next_bit_len();
    uint8_t get_n_bits = (expected_bits < 32) ? expected_bits : 32;

    uint32_t raw_bits = 0;
    if (get_n_bits != 0 && !br.get_bits(raw_bits, get_n_bits)) {
      return BitFieldResult::ErrorExpectedMoreBits;
    }

    if (dec.try_decode_byte(raw_bits, get_n_bits, cp)) {
      // Break on terminating null
      if (cp == 0) {
        break;
      }

      size_t n = utf::encode_codepoint_to_utf<CharT>(cp, buf);
      // Garbage char, omit
      if (n <= 0)
        continue;

      out.data_.append(buf, n);
    }
  }

  if (dec.flush(cp)) {
    size_t n = utf::encode_codepoint_to_utf<CharT>(cp, buf);
    if (n > 0) {
      out.data_.append(buf, n);
    }
  }

  return BitFieldResult::Ok;
}

#if SUB8_ENABLE_STRING_FIELDS__CHAR
template<typename Encoder, typename Decoder> using StringField = BasicStringField<char, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
template<typename Encoder, typename Decoder> using wStringField = BasicStringField<wchar_t, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR
template<typename Encoder, typename Decoder> using u8StringField = BasicStringField<char8_t, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR
template<typename Encoder, typename Decoder> using u16StringField = BasicStringField<char16_t, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR
template<typename Encoder, typename Decoder> using u32StringField = BasicStringField<char32_t, Encoder, Decoder>;
#endif

#if SUB8_ENABLE_FIVE_BIT_STRING

// Hints for LLMs and Debugging
// clang-format off

// | Dec | Binary  | TO   | T1   | T2   | T3                     |    | T0 (GREEK) | T1 (GREEK) |    | T0 (CYRILLIC) | T1 (CYRILLIC) |
// |-----|---------|------|------|------|------------------------|    |------------|------------|    |---------------|---------------|
// | 0   | 0b00000 | a    | A    | 0    | Std Table              |    |     α      |     Α      |    |      а        |      А        |
// | 1   | 0b00001 | b    | B    | 1    | Greek Table            |    |     β      |     Β      |    |      б        |      Б        |
// | 2   | 0b00010 | c    | C    | 2    | Cyrillic Table         |    |     σ      |     Σ      |    |      ц        |      Ц        |
// | 3   | 0b00011 | d    | D    | 3    | reserved               |    |     δ      |     Δ      |    |      д        |      Д        |
// | 4   | 0b00100 | e    | E    | 4    | reserved               |    |     ε      |     Ε      |    |      є        |      Є        |
// | 5   | 0b00101 | f    | F    | 5    | reserved               |    |     φ      |     Φ      |    |      ф        |      Ф        |
// | 6   | 0b00110 | g    | G    | 6    | reserved               |    |     γ      |     Γ      |    |      ґ        |      Ґ        |
// | 7   | 0b00111 | h    | H    | 7    | reserved               |    |     χ      |     Χ      |    |      х        |      Х        |
// | 8   | 0b01000 | i    | I    | 8    | reserved               |    |     ι      |     Ι      |    |      і        |      І        |
// | 9   | 0b01001 | j    | J    | 9    | reserved               |    |     ζ      |     Ζ      |    |      ж        |      Ж        |
// | 10  | 0b01010 | k    | K    | {    | modifier 0             |    |     κ      |     Κ      |    |      к        |      К        |
// | 11  | 0b01011 | l    | L    | }    | modifier 1             |    |     λ      |     Λ      |    |      л        |      Л        |
// | 12  | 0b01100 | m    | M    | [    | modifier 2             |    |     μ      |     Μ      |    |      м        |      М        |
// | 13  | 0b01101 | n    | N    | ]    | modifier 3             |    |     ν      |     Ν      |    |      н        |      Н        |
// | 14  | 0b01110 | o    | O    | :    | modifier 4             |    |     ο      |     Ο      |    |      о        |      О        |
// | 15  | 0b01111 | p    | P    | ,    | modifier 5             |    |     π      |     Π      |    |      п        |      П        |
// | 16  | 0b10000 | q    | Q    | "    | modifier 6             |    |     θ      |     Θ      |    |      к        |      К        |
// | 17  | 0b10001 | r    | R    | .    | modifier 7             |    |     ρ      |     Ρ      |    |      р        |      Р        |
// | 18  | 0b10010 | s    | S    | +    | modifier 8             |    |     ς      |     Σ      |    |      с        |      С        |
// | 19  | 0b10011 | t    | T    | =    | modifier 9             |    |     τ      |     Τ      |    |      т        |      Т        |
// | 20  | 0b10100 | u    | U    | '    | modifier 10            |    |     υ      |     Υ      |    |      у        |      У        |
// | 21  | 0b10101 | v    | V    | /    | modifier 11            |    |     ω      |     Ω      |    |      в        |      В        |
// | 22  | 0b10110 | w    | W    | \\   | modifier 12            |    |     ψ      |     Ψ      |    |      ў        |      Ў        |
// | 23  | 0b10111 | x    | X    | \t   | modifier 13            |    |     ξ      |     Ξ      |    |      х        |      Х        |
// | 24  | 0b11000 | y    | Y    | \n   | modifier 14            |    |     η      |     Η      |    |      ї        |      Ї        |
// | 25  | 0b11001 | z    | Z    | \r   | modifier 15            |    |     ζ      |     Ζ      |    |      з        |      З        |
// | 26  | 0b11010 | ' '  | ' '  | ' '  | modifier 16            |    |    ' '     |    ' '     |    |     ' '       |     ' '       |
// | 27  | 0b11011 |  _   |  _   |  _   | modifier 17            |    |     _      |     _      |    |      _        |      _        |
// | 28  | 0b11100 |  -   |  -   |  -   | Switch Character Set   |    |     -      |     -      |    |      -        |      -        |
// | 29  | 0b11101 | T1   | T0   | T1   | T1                     |    |     T1     |     T0     |    |     T1        |      T0       |
// | 30  | 0b11110 | T2   | T2   | T0   | T2                     |    |     T2     |     T2     |    |     T2        |      T2       |
// | 31  | 0b11111 | T3   | T3   | T3   | T0                     |    |     T3     |     T3     |    |     T3        |      T3       |

// clang-format on
namespace b5 {

struct B5CharAddress final {
  using address_t = uint32_t;

  //
  // Bit layout (LSB = bit 0)
  //
  // [25:22] AVAILABLE TABLE FLAGS   (4 bits)
  // [21:10] AVAILABLE CHARSET FLAGS (12 bits)
  // [ 9: 5] ADDRESS                 (5 bits)
  // [ 4: 0] EXT                     (5 bits)
  //
  // Unused: bits 26–31
  //

  static constexpr address_t EXT_SHIFT = 0;       // 5 bits
  static constexpr address_t ADDR_SHIFT = 5;      // 5 bits
  static constexpr address_t ADDR10_SHIFT = 0;    // 10 bits
  static constexpr address_t CHARSET_SHIFT = 10;  // 12 bits
  static constexpr address_t TABLE_SHIFT = 22;    // 4 bits

  static constexpr address_t EXT_MASK = 0b1'1111;              // 5 bits
  static constexpr address_t ADDR_MASK = 0b1'1111;             // 5 bits
  static constexpr address_t ADDR10_MASK = 0b11'1111'1111;     // 10 bits
  static constexpr address_t CHARSET_MASK = 0b1111'1111'1111;  // 12 bits
  static constexpr address_t TABLE_MASK = 0b1111;              // 4 bits

  static constexpr uint8_t SWITCH_CHARSET_CODE = 28;
  static constexpr uint8_t MAX_CHARSET_CODE = 10;

  static constexpr uint8_t T1_CODE = 29;
  static constexpr uint8_t T2_CODE = 30;
  static constexpr uint8_t T3_CODE = 31;

  enum ApplicableTables : uint8_t {
    APPLICABLE_TABLES_NONE = 0b0000,
    APPLICABLE_TABLES_T1 = 0b0001,
    APPLICABLE_TABLES_T2 = 0b0010,
    APPLICABLE_TABLES_T3 = 0b0100,
    APPLICABLE_TABLES_T0 = 0b1000,  // Equal to the current table
    APPLICABLE_TABLES_ALL = 0b1111,
  };

  enum Table : uint8_t {
    TABLE_T1 = 0,
    TABLE_T2 = 1,
    TABLE_T3 = 2,
    TABLE_T0 = 3,  // Equal to the current table
    TABLE_NONE = 255,
  };

  enum ApplicableCharsets : uint16_t {
    APPLICABLE_CHARSET_NONE = 0b0000'0000'0000,
    APPLICABLE_CHARSET_LATIN = 0b0000'0000'0001,
    APPLICABLE_CHARSET_GREEK = 0b0000'0000'0010,
    APPLICABLE_CHARSET_CYRILLIC = 0b0000'0000'0100,
    // reserved bits ...
    APPLICABLE_CHARSET_EXTENDED = (1u << MAX_CHARSET_CODE),
    APPLICABLE_CHARSET_ALL = 0b1111'1111'1111
  };

  enum Charset : uint8_t {
    CHARSET_NONE = 32,
    CHARSET_LATIN = 0,
    CHARSET_GREEK = 1,
    CHARSET_CYRILLIC = 2,
    // reserved ...
    CHARSET_EXTENDED = MAX_CHARSET_CODE
  };

  address_t value{0};

  constexpr B5CharAddress() = default;

  constexpr B5CharAddress(address_t raw_value) : value(raw_value) {}

  constexpr B5CharAddress(uint8_t addr5, uint8_t ext5, uint16_t charset12, uint8_t table4)
      : value(pack(addr5, ext5, charset12, table4)) {}

  static constexpr address_t pack(uint8_t addr5, uint8_t ext5, uint16_t charset12, uint8_t table4) {
    return ((address_t(ext5 & EXT_MASK) << EXT_SHIFT) | (address_t(addr5 & ADDR_MASK) << ADDR_SHIFT) |
            (address_t(charset12 & CHARSET_MASK) << CHARSET_SHIFT) | (address_t(table4 & TABLE_MASK) << TABLE_SHIFT));
  }

  static constexpr address_t make(uint8_t addr5, uint8_t ext5, uint16_t charset12, uint8_t table4) {
    return pack(addr5, ext5, charset12, table4);
  }

  static constexpr address_t make_null_address() { return pack(0, 0, APPLICABLE_CHARSET_NONE, APPLICABLE_TABLES_NONE); }

  static constexpr address_t make_char_not_found_address() {
    return pack(T3_CODE, T2_CODE, APPLICABLE_CHARSET_NONE, APPLICABLE_TABLES_NONE);
  }

  static constexpr address_t make_terminating_address() {
    return pack(T3_CODE, T3_CODE, APPLICABLE_CHARSET_EXTENDED, APPLICABLE_TABLES_ALL);
  }

  static constexpr Charset into_single_charset(ApplicableCharsets charset) {
    uint16_t cs = static_cast<uint16_t>(charset);
    if (cs == 0)
      return CHARSET_NONE;

    // isolate lowest set bit
    uint16_t lsb = cs & (~cs + 1);

    // find bit index
    uint8_t idx = 0;
    while ((lsb >> idx) != 1u) {
      ++idx;
    }

    return static_cast<Charset>(idx);
  }

  static constexpr ApplicableCharsets into_single_applicable_charset(Charset charset) noexcept {
    if (charset == B5CharAddress::CHARSET_NONE) {
      return B5CharAddress::APPLICABLE_CHARSET_NONE;
    }
    if (charset >= B5CharAddress::CHARSET_EXTENDED) {
      return B5CharAddress::APPLICABLE_CHARSET_EXTENDED;
    }
    return static_cast<ApplicableCharsets>(1u << static_cast<uint8_t>(charset));
  }

  static constexpr ApplicableCharsets into_common_charsets(ApplicableCharsets charset_a, ApplicableCharsets charset_b) {
    uint16_t csa = static_cast<uint16_t>(charset_a);
    uint16_t csb = static_cast<uint16_t>(charset_b);
    return static_cast<ApplicableCharsets>(csa & csb);
  }

  static constexpr bool has_common_charsets(ApplicableCharsets charset_a, ApplicableCharsets charset_b) {
    return into_common_charsets(charset_a, charset_b) != APPLICABLE_CHARSET_NONE;
  }

  static constexpr Table into_single_table(ApplicableTables table) {
    uint8_t t = static_cast<uint8_t>(table);
    if (t == 0)
      return TABLE_NONE;

    uint8_t lsb = t & (~t + 1);

    switch (lsb) {
      case APPLICABLE_TABLES_T0:
        return TABLE_T0;
      case APPLICABLE_TABLES_T1:
        return TABLE_T1;
      case APPLICABLE_TABLES_T2:
        return TABLE_T2;
      case APPLICABLE_TABLES_T3:
        return TABLE_T3;
      default:
        return TABLE_NONE;  // invalid mask
    }
  }

  static constexpr ApplicableTables into_single_applicable_table(Table table) noexcept {
    if (table > B5CharAddress::TABLE_T0) {
      return B5CharAddress::APPLICABLE_TABLES_NONE;
    }
    return static_cast<ApplicableTables>(1u << static_cast<uint8_t>(table));
  }

  static constexpr ApplicableTables into_common_tables(ApplicableTables table_a, ApplicableTables table_b) {
    uint8_t ta = static_cast<uint8_t>(table_a);
    uint8_t tb = static_cast<uint8_t>(table_b);
    return static_cast<ApplicableTables>(ta & tb);
  }

  static constexpr bool has_common_tables(ApplicableTables table_a, ApplicableTables table_b) {
    return into_common_tables(table_a, table_b) != APPLICABLE_TABLES_NONE;
  }

  // Address fields

  constexpr uint8_t char_code_ext() const { return static_cast<uint8_t>((value >> EXT_SHIFT) & EXT_MASK); }

  constexpr uint8_t char_code() const { return static_cast<uint8_t>((value >> ADDR_SHIFT) & ADDR_MASK); }

  constexpr uint16_t char_code_10_bit() const { return static_cast<uint16_t>((value >> ADDR10_SHIFT) & ADDR10_MASK); }

  constexpr ApplicableCharsets applicable_charsets() const {
    return ApplicableCharsets(static_cast<uint16_t>((value >> CHARSET_SHIFT) & CHARSET_MASK));
  }

  constexpr ApplicableTables applicable_tables() const {
    return ApplicableTables(static_cast<uint8_t>((value >> TABLE_SHIFT) & TABLE_MASK));
  }

  constexpr size_t table_index() const { return (static_cast<size_t>(into_single_table(applicable_tables())) + 1) % 4; }

  constexpr bool is_available_on_table(Table single_table_scope) const {
    auto applicable_table = into_single_applicable_table(single_table_scope);
    return into_common_tables(applicable_table, applicable_tables()) != APPLICABLE_TABLES_NONE;
  }

  constexpr Table pick_best_table(ApplicableTables table_scope) const {
    // Picks the best char set based on the past and previous ApplicableCharsetss
    auto t1 = applicable_tables();
    auto t2 = into_common_tables(t1, table_scope);
    if (t2 == APPLICABLE_TABLES_NONE) {
      return into_single_table(t1);
    }
    return into_single_table(t2);
  }

  constexpr Charset charset() const { return into_single_charset(applicable_charsets()); }

  constexpr Charset pick_common_charset(ApplicableCharsets charset_scope) const {
    // Picks the best char set based on the past and previous ApplicableCharsetss
    auto cs1 = applicable_charsets();
    auto cs2 = into_common_charsets(cs1, charset_scope);
    return into_single_charset(cs2);
  }

  constexpr bool is_available_in_charset(Charset c) const {
    return into_common_charsets(applicable_charsets(), into_single_applicable_charset(c)) != APPLICABLE_CHARSET_NONE;
  }

  constexpr bool is_extended_char() const {
    return into_common_charsets(applicable_charsets(), APPLICABLE_CHARSET_EXTENDED) != APPLICABLE_CHARSET_NONE;
  }

  constexpr bool is_null() const { return char_code_10_bit() == 0 && applicable_tables() == APPLICABLE_TABLES_NONE; }

  constexpr bool is_terminator() const { return char_code_10_bit() == 1023; }

  constexpr bool is_not_found() const { return char_code_10_bit() == 1022; }

  constexpr bool operator==(const B5CharAddress &other) const { return value == other.value; }

  constexpr bool operator!=(const B5CharAddress &other) const { return value != other.value; }
};

}  // namespace b5

namespace b5::ser::lut {

#define B5_LATIN static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_LATIN)
#define B5_GREEK static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_GREEK)
#define B5_CYRILLIC static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_CYRILLIC)
#define B5_EXTENDED static_cast<uint16_t>(b5::B5CharAddress::APPLICABLE_CHARSET_EXTENDED)

#define B5_T0 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T0)
#define B5_T1 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T1)
#define B5_T2 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T2)
#define B5_T3 static_cast<uint8_t>(b5::B5CharAddress::APPLICABLE_TABLES_T3)

struct B5EncoderLutNode final {
  char32_t ch;
  B5CharAddress addr;
};

// insert generated lut in sub8.cpp
extern const B5EncoderLutNode B5EncoderLut[];
extern const size_t B5EncoderLutSize;

inline B5CharAddress into_b5_char_address(char32_t utf32_in) {
  size_t left = 0;
  size_t right = B5EncoderLutSize;

  while (left < right) {
    const size_t mid = left + (right - left) / 2;
    const char32_t mid_ch = B5EncoderLut[mid].ch;

    if (utf32_in < mid_ch)
      right = mid;
    else if (utf32_in > mid_ch)
      left = mid + 1;
    else
      return B5EncoderLut[mid].addr;
  }
  return B5CharAddress::make_char_not_found_address();
}

inline uint8_t into_table_control_code(B5CharAddress::Table target_table,
                                       B5CharAddress::Table current_table = B5CharAddress::TABLE_NONE) {
  if (target_table == B5CharAddress::TABLE_NONE) {
    return B5CharAddress::T1_CODE + static_cast<uint8_t>(current_table);
  }

  if (target_table == B5CharAddress::TABLE_T0) {
    // Switching back to T0 is indicated by the current table control char
    target_table = current_table;
  }
  return B5CharAddress::T1_CODE + static_cast<uint8_t>(target_table);
}

inline uint8_t into_charset_control_code(B5CharAddress::Charset target_charset) {
  return static_cast<uint8_t>(target_charset);
}
}  // namespace b5::ser::lut

namespace utf {
template<> struct UtfCodepointMapper<b5::B5CharAddress> {
  using value_type = b5::B5CharAddress;

  static value_type map(char32_t cp) noexcept { return b5::ser::lut::into_b5_char_address(cp); }
};

}  // namespace utf

namespace b5::ser {

using B5LookAheadIterator = utf::UtfCodepointLookAheadIteratorT<utf::UtfCodepointMapper<B5CharAddress>, 2>;

class B5CodeSequenceEncoder {
 public:
  B5CodeSequenceEncoder() noexcept = default;

  explicit B5CodeSequenceEncoder(utf::UtfCodepointIterator iter, bool emit_terminating_zero = false,
                                 bool starting_in_multi_code_state = false,
                                 B5CharAddress::Charset starting_charset = B5CharAddress::CHARSET_LATIN,
                                 B5CharAddress::Table starting_table = B5CharAddress::TABLE_T0) noexcept
      : iter_(B5LookAheadIterator(iter)),
        previous_table(starting_table),
        previous_charset_(starting_charset),
        previous_multi_code_state_(starting_in_multi_code_state),
        starting_table_(starting_table),
        starting_charset_(starting_charset),
        starting_multi_code_state_(starting_in_multi_code_state),
        emit_terminating_zero_(emit_terminating_zero) {}

  size_t try_encode_next(uint8_t *buffer, size_t len) noexcept {
    size_t seq_len = 0;
    B5CharAddress head = B5CharAddress::make_null_address();
    B5CharAddress look_ahead_1 = B5CharAddress::make_null_address();

    if (!iter_.try_pop(head)) {
      return seq_len;
    }

    bool end_of_string = !iter_.try_peek_ahead(0, look_ahead_1);

    // -----------------------------------------------------------------
    // 0) Char not found
    // -----------------------------------------------------------------
    if (head.is_not_found()) {
      seq_len += emit_not_found_char(buffer, len);
      return seq_len;
    }

    // -----------------------------------------------------------------
    // 1) Select Table
    // -----------------------------------------------------------------

    // if the char is not available on the previously selected table
    if (!head.is_available_on_table(previous_table)) {
      // Try pick a table which will fit the next two chars
      auto target_table = head.pick_best_table(look_ahead_1.applicable_tables());
      if (target_table != B5CharAddress::TABLE_NONE) {
        // Emit code(s) to switch table
        seq_len += emit_table_control_char(target_table, buffer, len);
        previous_table = target_table;
      }
    }

    // -----------------------------------------------------------------
    // 2) Multi-code (10-bit) mode, controlled by T3 + T1 / exit logic
    // -----------------------------------------------------------------
    if (!previous_multi_code_state_) {
      // Enter 10bit mode if next two char are in the extended character sets
      if (head.is_extended_char() && (!look_ahead_1.is_null() && look_ahead_1.is_extended_char())) {
        // emit chars to go to multi byte mode
        seq_len += emit_enter_multi_byte_mode_control_char(buffer, len);
        previous_multi_code_state_ = true;
      }
    } else {
      if (!head.is_extended_char() && (!look_ahead_1.is_null() && !look_ahead_1.is_extended_char())) {
        // emit chars to go to exit multi byte mode
        seq_len += emit_exit_multi_byte_mode_control_char(buffer, len);
        previous_multi_code_state_ = false;
      }
    }

    // -----------------------------------------------------------------
    // 3) Charset selection / shifting (only in 5-bit mode)
    // -----------------------------------------------------------------

    // if not in 10bit mode, then extend char are shifted
    bool is_15bit_extended_char = head.is_extended_char() && !previous_multi_code_state_;

    if (!previous_multi_code_state_ && !is_15bit_extended_char && !head.is_available_in_charset(previous_charset_)) {
      // Check to see if the next char is also in the new character set. If it is,
      // the lock to new character set, otherwise shift only
      auto common_charset = head.pick_common_charset(look_ahead_1.applicable_charsets());

      if (common_charset != B5CharAddress::CHARSET_NONE) {
        // Emit char set lock
        seq_len += emit_switch_char_set_control_char(common_charset, buffer, len);
        // switch char set
        previous_charset_ = common_charset;
      } else {
        // Emit char set shift
        is_15bit_extended_char |= true;
      }
    }

    // -----------------------------------------------------------------
    // 4) Emit primary code
    // -----------------------------------------------------------------
    if (!head.is_null()) {
      seq_len += emit_code(head.char_code(), buffer, len);
    }

    // -----------------------------------------------------------------
    // 5) Emit extended shift control for 15bit chars
    // -----------------------------------------------------------------
    if (is_15bit_extended_char) {
      seq_len += emit_extended_shift_control_char(buffer, len);
    }

    // -----------------------------------------------------------------
    // 6) Emit extended code for 10 and 15bit chars
    // -----------------------------------------------------------------
    if (previous_multi_code_state_ || is_15bit_extended_char) {
      seq_len += emit_code(head.char_code_ext(), buffer, len);
    }

    // -----------------------------------------------------------------
    // 7) Emit terminating code at end of stream if enabled
    // -----------------------------------------------------------------
    if (emit_terminating_zero_ && end_of_string) {
      seq_len += emit_terminating_null_control_char(buffer, len);
    }

    return seq_len;
  }

  bool empty() const noexcept { return iter_.empty(); }

  void reset() noexcept {
    iter_.reset();
    previous_table = starting_table_;
    previous_charset_ = starting_charset_;
    previous_multi_code_state_ = starting_multi_code_state_;
  }

 private:
  size_t emit_table_control_char(B5CharAddress::Table table, uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> {Tx}
    uint8_t code = lut::into_table_control_code(table, previous_table);
    emit_code(code, buffer, len);
    return 1;
  }

  size_t emit_not_found_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> T3 | T2
    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T3), buffer, len);
    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T2), buffer, len);
    return 2;
  }

  size_t emit_enter_multi_byte_mode_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> T3 | T1

    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T3), buffer, len);
    emit_code(lut::into_table_control_code(B5CharAddress::TABLE_T1), buffer, len);
    return 2;
  }

  size_t emit_exit_multi_byte_mode_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> T3
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    emit_code(t3_code, buffer, len);
    return 1;
  }

  size_t emit_extended_shift_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // 15 bit char code
    // EMIT -> {code} | T3 | {ext code}
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    emit_code(t3_code, buffer, len);
    return 1;
  }

  size_t emit_switch_char_set_control_char(B5CharAddress::Charset charset, uint8_t *&buffer, size_t &len) noexcept {
    // EMIT -> 28 | T3 | {char set code}
    uint8_t change_char_set_code = 28;
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    uint8_t char_set_ext_code = lut::into_charset_control_code(charset);

    emit_code(change_char_set_code, buffer, len);
    emit_code(t3_code, buffer, len);
    emit_code(char_set_ext_code, buffer, len);
    return 3;
  }

  size_t emit_terminating_null_control_char(uint8_t *&buffer, size_t &len) noexcept {
    // EMIT ->  T3 | T3
    uint8_t t3_code = lut::into_table_control_code(B5CharAddress::TABLE_T3);
    emit_code(t3_code, buffer, len);
    emit_code(t3_code, buffer, len);
    return 2;
  }

  size_t emit_code(uint8_t code, uint8_t *&buffer, size_t &len) noexcept {
    if (len == 0) {
      return 1;
    }
    buffer[0] = code & 0x1Fu;

    buffer++;
    len--;
    return 1;
  }

  B5LookAheadIterator iter_{};

  B5CharAddress::Table previous_table{B5CharAddress::TABLE_T0};
  B5CharAddress::Charset previous_charset_{B5CharAddress::CHARSET_LATIN};
  bool previous_multi_code_state_{false};

  B5CharAddress::Table starting_table_{B5CharAddress::TABLE_T0};
  B5CharAddress::Charset starting_charset_{B5CharAddress::CHARSET_LATIN};
  bool starting_multi_code_state_{false};
  bool emit_terminating_zero_{false};
};
}  // namespace b5::ser

namespace b5::de {

namespace lut {
struct B5DecoderExtEntry final {
  uint16_t code10;
  char32_t ch;
};

inline constexpr std::size_t B5_NUM_TABLES = 3;
inline constexpr std::size_t B5_NUM_CODES = 32;
extern const size_t B5_NUM_CHARSETS;
extern const char32_t B5DecoderLut[][B5_NUM_TABLES][B5_NUM_CODES];

extern const B5DecoderExtEntry *B5DecoderLutExt[];

extern const size_t B5DecoderLutExtCount[];

inline char32_t get_extended_char_from_b5_char_address(const B5CharAddress b5) noexcept {
  if (b5.is_terminator() || b5.is_null()) {
    return U'\0';
  }

  if (b5.is_not_found()) {
    return U'\uFFFD';  // � not found
  }

  const size_t table = b5.table_index();
  const uint16_t key = b5.char_code_10_bit();

  const auto *lut = B5DecoderLutExt[table];
  const auto count = B5DecoderLutExtCount[table];

  size_t left = 0;
  size_t right = count;
  while (left < right) {
    const size_t mid = left + ((right - left) / 2);
    const auto mid_key = lut[mid].code10;

    if (key < mid_key) {
      right = mid;
    } else if (key > mid_key) {
      left = mid + 1;
    } else {
      return lut[mid].ch;
    }
  }

  return U'\uFFFD';  // � not found
}

inline char32_t get_char_from_b5_char_address(b5::B5CharAddress b5) {
  if (b5.is_terminator() || b5.is_null()) {
    return U'\0';
  }

  if (b5.is_not_found()) {
    return U'\uFFFD';  // � not found
  }

  if (b5.is_extended_char()) {
    return get_extended_char_from_b5_char_address(b5);
  }

  const size_t cs_idx = static_cast<size_t>(b5.charset());
  const size_t code = static_cast<size_t>(b5.char_code());

  size_t tbl_idx = b5.table_index();

  if (cs_idx >= B5_NUM_CHARSETS || tbl_idx >= B5_NUM_TABLES || code >= B5_NUM_CODES) {
    return U'\uFFFD';  // �
  }
  return B5DecoderLut[cs_idx][tbl_idx][code];
}

}  // namespace lut

inline B5CharAddress::ApplicableCharsets code_into_charset(uint8_t code) noexcept {
  if (code <= B5CharAddress::MAX_CHARSET_CODE) {
    return static_cast<B5CharAddress::ApplicableCharsets>(1u << code);
  }
  return B5CharAddress::APPLICABLE_CHARSET_EXTENDED;
}

inline uint16_t into_charset_bit_flags(B5CharAddress::Charset cs) noexcept {
  return static_cast<uint16_t>(code_into_charset(static_cast<uint8_t>(cs)));
}

inline uint8_t into_table_bit_flags(B5CharAddress::Table table) noexcept {
  return static_cast<uint8_t>(B5CharAddress::into_single_applicable_table(table));
}

inline uint8_t into_table_bit_flags(uint8_t code, uint8_t current_table) noexcept {
  if (code < B5CharAddress::T1_CODE || code > B5CharAddress::T3_CODE) {
    return static_cast<uint8_t>(B5CharAddress::APPLICABLE_TABLES_NONE);
  }
  uint8_t table = into_table_bit_flags(static_cast<B5CharAddress::Table>(code - B5CharAddress::T1_CODE));
  if (table == current_table) {
    return static_cast<uint8_t>(B5CharAddress::APPLICABLE_TABLES_T0);
  }
  return table;
}

constexpr bool is_control_code(uint8_t code) noexcept {
  return code >= B5CharAddress::T1_CODE && code < (B5CharAddress::T3_CODE + 1);
}

class B5CodeSequenceBuffer {
  static constexpr uint8_t MAX_CODE_LEN = 3;

 public:
  B5CodeSequenceBuffer() noexcept = default;

  void enqueue_buffer(uint8_t code_in) noexcept {
    buf_[(idx_ + len_) % MAX_CODE_LEN] = code_in;
    if (len_ < MAX_CODE_LEN) {
      ++len_;
    } else {
      idx_ = (idx_ + 1) % MAX_CODE_LEN;
    }
  }

  void advance_buffer(uint8_t distance = 1) noexcept {
    if (distance >= len_) {
      idx_ = 0;
      len_ = 0;
      return;
    }

    idx_ = (idx_ + distance) % MAX_CODE_LEN;
    len_ -= distance;
  }

  uint8_t at(uint8_t i) const noexcept { return buf_[(idx_ + i) % MAX_CODE_LEN]; }

  uint8_t size() const noexcept { return len_; }
  bool empty() const noexcept { return len_ == 0; }
  bool full() const noexcept { return len_ == MAX_CODE_LEN; }

 private:
  uint8_t idx_ = 0;
  uint8_t len_ = 0;
  uint8_t buf_[MAX_CODE_LEN]{};  // circular storage
};

class B5CodeSequenceDecoder {
 public:
  B5CodeSequenceDecoder() noexcept = default;

  explicit B5CodeSequenceDecoder(bool starting_in_multi_code_state = false,
                                 B5CharAddress::Charset starting_charset = B5CharAddress::CHARSET_LATIN,
                                 B5CharAddress::Table starting_table = B5CharAddress::TABLE_T0) noexcept
      : current_charset12_(into_charset_bit_flags(starting_charset)),
        current_table4_(into_table_bit_flags(starting_table)),
        current_multi_code_state_(starting_in_multi_code_state) {}

  bool try_decode_next_address(B5CodeSequenceBuffer &buff, B5CharAddress &b5_out, bool end_of_stream) noexcept {
    const uint8_t size = buff.size();

    if (size < 2 && !end_of_stream) {
      return false;
    }

    if (size == 0) {
      return false;
    }

    const uint8_t a = buff.at(0);
    const uint8_t b = (size >= 2) ? buff.at(1) : 128;
    const uint8_t c = (size >= 3) ? buff.at(2) : 128;

    // ------------------------------------------------------------
    // 1) Handle control codes
    // ------------------------------------------------------------
    if (is_control_code(a)) {
      if (a == B5CharAddress::T3_CODE) {
        if (size < 2) {
          return false;
        }
        // Terminating null T3 + T3
        if (b == B5CharAddress::T3_CODE) {
          b5_out = B5CharAddress::make_terminating_address();
          buff.advance_buffer(2);
          return true;
        }

        // char not found T3 + T2
        if (b == B5CharAddress::T2_CODE) {
          b5_out = B5CharAddress::make_char_not_found_address();
          buff.advance_buffer(2);
          return true;
        }

        // Enter multi-code (10-bit) mode: T3 + T1
        if (!current_multi_code_state_ && b == B5CharAddress::T1_CODE) {
          current_multi_code_state_ = true;
          buff.advance_buffer(2);
          return false;
        }

        // Exit multi-code mode: T3 + ...
        if (current_multi_code_state_) {
          current_multi_code_state_ = false;
          buff.advance_buffer(1);
          return false;
        }
      } else {
        // Table switch: T{n}
        current_table4_ = into_table_bit_flags(a, current_table4_);
        buff.advance_buffer(1);
        return false;
      }
    }

    if (size == 1 && end_of_stream) {
      buff.advance_buffer(1);
      return decode_single(a, b5_out);
    }

    // ------------------------------------------------------------
    // 2) 10-bit encoding mode (multi-code mode)
    // ------------------------------------------------------------
    if (current_multi_code_state_) {
      if (size < 2) {
        return false;
      }
      buff.advance_buffer(2);
      return decode_double(a, b, b5_out);
    }

    // ------------------------------------------------------------
    // 3) 15-bit shift encoding:
    //    {code} | T3 | {ext code}
    //    or Charset switch: 28 | T3 | {charset code}
    // ------------------------------------------------------------
    if (b == B5CharAddress::T3_CODE) {
      if (size < 3) {
        return false;
      }
      if (!is_control_code(c)) {
        buff.advance_buffer(3);
        return decode_double(a, c, b5_out);
      }
    }

    // ------------------------------------------------------------
    // 4) 5-bit encoding: {code}
    // ------------------------------------------------------------
    buff.advance_buffer(1);
    return decode_single(a, b5_out);
  }

 private:
  // Decode a single 5-bit code into a B5CharAddress
  bool decode_single(uint8_t a, B5CharAddress &b5_out) noexcept {
    b5_out = B5CharAddress(a, 0, current_charset12_, current_table4_);
    return true;
  }

  // Decode a double-code sequence (10-bit or 15-bit shift)
  bool decode_double(uint8_t a, uint8_t b, B5CharAddress &b5_out) noexcept {
    auto char_set = code_into_charset(b);

    // Charset switch: #28 | T3 | {charset code}
    if (a == B5CharAddress::SWITCH_CHARSET_CODE &&
        char_set != B5CharAddress::ApplicableCharsets::APPLICABLE_CHARSET_EXTENDED) {
      current_charset12_ = static_cast<uint16_t>(char_set);
      return false;
    }

    // Extend char set char: {code} | T3 | {ext code} or {code} | {ext code}
    b5_out = B5CharAddress(a, b, static_cast<uint16_t>(char_set), current_table4_);
    return true;
  }

 private:
  uint16_t current_charset12_{0};
  uint8_t current_table4_{0};
  bool current_multi_code_state_{false};
};

};  // namespace b5::de

// Encoder
template<
    // MaxSymbols: Max encoded symbol length of path.
    //
    // When `TerminateSequence == false`:
    //   The string is prefixed with the length, value which is optimized
    //   to be the shortest possible encoded length. Ie a max length of 15 will
    //   require an additional 4bits to encode the length. The size penalty will increase
    //   for each n^2 increment in length
    //   This process requires the string to be pre-passed to calculate the the actual encoded length.
    //
    // When `TerminateSequence == true`:
    //   The string is terminated with a 10bit termination code. This will use more
    //   bits for any string under a 1,024 length. However the a null terminated sequence
    //   does not require pre-pass step to calculate the full length.
    //
    size_t MaxSymbols = 255,

    // TerminateSequence: Terminates sequence with 10bit null char
    // If enabled, string length is not prefixed and instead a terminating null is encoded.
    bool TerminateSequence = false,

    // StartingMultiCodeState: Indicate if the encoder should start in 10bit multi code or 5bit mode.
    bool StartingMultiCodeState = false,

    // StartingCharset: Indicate the starting charater set
    b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,

    // StartingCharset: Indicate the starting table set (ie lower case / upper case / numeric)
    b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
class B5Encoder {
  b5::ser::B5CodeSequenceEncoder iter_{};

  // Total number of 5-bit symbols (controls + data) that will follow the header
  size_t total_symbol_length_ = 0;
  size_t remaining_header_bits_ = 0;

  static constexpr size_t ENCODED_CHAR_BUFFER_LEN = 6;
  uint8_t code_buffer_[ENCODED_CHAR_BUFFER_LEN]{};
  size_t code_buffer_size_ = 0;
  size_t code_buffer_idx_ = 0;

  static constexpr uint8_t calc_required_len() {
    size_t v = MaxSymbols;
    uint8_t bits = 0;
    while (v > 0) {
      v >>= 1;
      ++bits;
    }
    return bits ? bits : uint8_t{1};
  }
  static constexpr uint8_t LengthBits = calc_required_len();
  static constexpr uint8_t SymbolBits = 5;  // 5-bit codes

 public:
  BitFieldResult init(utf::UtfCodepointIterator utf_iter) noexcept {
    iter_ = b5::ser::B5CodeSequenceEncoder(utf_iter, TerminateSequence, StartingMultiCodeState, StartingCharset,
                                           StartingTable);
    if constexpr (TerminateSequence) {
      remaining_header_bits_ = 0;
      total_symbol_length_ = MaxSymbols;
    } else {
      total_symbol_length_ = calc_encoded_length();
      remaining_header_bits_ = LengthBits;
      if (total_symbol_length_ > MaxSymbols) {
        return BitFieldResult::ErrorValueTooLarge;
      }
    }
    return BitFieldResult::Ok;
  }

  bool try_encode(uint32_t &out, uint8_t &bit_len) noexcept {
    // 1) Emit header once
    if (remaining_header_bits_ != 0) {
      out = static_cast<uint32_t>(total_symbol_length_);
      bit_len = remaining_header_bits_;
      remaining_header_bits_ = 0;
      return true;
    }

    // 2) If we still have pending 5-bit symbols, emit them first
    if (code_buffer_idx_ < code_buffer_size_) {
      out = static_cast<uint32_t>(code_buffer_[code_buffer_idx_++]);
      bit_len = SymbolBits;
      return true;
    }

    code_buffer_idx_ = 0;
    code_buffer_size_ = 0;

    while (!iter_.empty()) {
      uint8_t *buffer = code_buffer_ + code_buffer_idx_;
      size_t buffer_len = ENCODED_CHAR_BUFFER_LEN - code_buffer_idx_;
      code_buffer_size_ = iter_.try_encode_next(buffer, buffer_len) + code_buffer_idx_;

      if (code_buffer_size_ != 0) {
        out = static_cast<uint32_t>(code_buffer_[code_buffer_idx_++]);
        bit_len = SymbolBits;
        return true;
      }
    }
    return false;
  }

 private:
  size_t calc_encoded_length() {
    size_t len = 0;
    uint8_t dummy_buffer = 0;
    b5::ser::B5CodeSequenceEncoder proprocess_iter = iter_;
    proprocess_iter.reset();
    while (!proprocess_iter.empty()) {
      len += proprocess_iter.try_encode_next(&dummy_buffer, 0);
    }
    return len;
  }
};

// Decoder
template<
    // MaxSymbols: Max encoded symbol length of path.
    //
    // When `TerminateSequence == false`:
    //   The string is prefixed with the length, value which is optimized
    //   to be the shortest possible encoded length. Ie a max length of 15 will
    //   require an additional 4bits to encode the length. The size penalty will increase
    //   for each n^2 increment in length
    //   This process requires the string to be pre-passed to calculate the the actual encoded length.
    //
    // When `TerminateSequence == true`:
    //   The string is terminated with a 10bit termination code. This will use more
    //   bits for any string under a 1,024 length. However the a null terminated sequence
    //   does not require pre-pass step to calculate the full length.
    //
    size_t MaxSymbols = 255,

    // TerminateSequence: Terminates sequence with 10bit null char
    // If enabled, string length is not prefixed and instead a terminating null is encoded.
    bool TerminatedSequence = false,

    // StartingMultiCodeState: Indicate if the encoder should start in 10bit multi code or 5bit mode.
    bool StartingMultiCodeState = false,

    // StartingCharset: Indicate the starting charater set
    b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,

    // StartingCharset: Indicate the starting table set (ie lower case / upper case / numeric)
    b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
class B5Decoder {
  bool pending_read_header_ = !TerminatedSequence;
  bool waiting_for_more_symbols_ = false;
  size_t symbols_remaining_ = MaxSymbols;

  b5::de::B5CodeSequenceBuffer buff_;
  b5::de::B5CodeSequenceDecoder seq_decoder_{StartingMultiCodeState, StartingCharset, StartingTable};

  static constexpr uint8_t calc_required_len() {
    size_t v = MaxSymbols;
    uint8_t bits = 0;
    while (v > 0) {
      v >>= 1;
      ++bits;
    }
    return bits ? bits : uint8_t{1};
  }

  static constexpr uint8_t HeaderLengthBits = calc_required_len();
  static constexpr uint8_t SymbolBits = 5;  // 5-bit codes

 public:
  BitFieldResult init() { return BitFieldResult::Ok; }

  bool end_of_sequence() { return symbols_remaining_ == 0 && buff_.empty(); }

  uint8_t expected_next_bit_len() const noexcept {
    if (pending_read_header_) {
      return HeaderLengthBits;
    }

    if (buff_.full() || !waiting_for_more_symbols_) {
      return 0;
    }

    if (symbols_remaining_ != 0) {
      return SymbolBits;
    }

    return 0;
  }

  bool try_decode_byte(uint32_t code_in, uint8_t code_bit_len, char32_t &utf32_out) noexcept {
    if (code_bit_len != expected_next_bit_len()) {
      // unexpected bit width
      return false;
    }

    // 1) Read header if present
    if (pending_read_header_) {
      symbols_remaining_ = static_cast<size_t>(code_in);
      if (symbols_remaining_ > MaxSymbols) {
        symbols_remaining_ = MaxSymbols;
      }
      pending_read_header_ = false;
      return false;
    }

    // 2) enqueue code
    if (code_bit_len != 0) {
      buff_.enqueue_buffer(static_cast<uint8_t>(code_in & 0x1Fu));
      symbols_remaining_ = symbols_remaining_ == 0 ? 0 : symbols_remaining_ - 1;
    }

    // 3) try decode
    b5::B5CharAddress b5_out;
    if (seq_decoder_.try_decode_next_address(buff_, b5_out, symbols_remaining_ == 0)) {
      waiting_for_more_symbols_ = false;
      if (b5_out.is_terminator()) {
        symbols_remaining_ = 0;
      }
      utf32_out = b5::de::lut::get_char_from_b5_char_address(b5_out);
      return true;
    }
    waiting_for_more_symbols_ = true;
    return false;
  }

  bool flush(char32_t &) noexcept { return false; }
};

#if SUB8_ENABLE_STRING_FIELDS__CHAR
template<size_t MaxSymbols = 255, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
         b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
         b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using B5StringField =
    StringField<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
                B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

template<size_t MaxSymbols = 255, bool StartingMultiCodeState = false,
         b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
         b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using B5StringFieldNullTerminated =
    StringField<B5Encoder<MaxSymbols, true, StartingMultiCodeState, StartingCharset, StartingTable>,
                B5Decoder<MaxSymbols, true, StartingMultiCodeState, StartingCharset, StartingTable>>;

template<size_t MaxSymbols = 512, bool TerminatedSequence = false,
         b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
         b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using B10StringField = StringField<B5Encoder<MaxSymbols, TerminatedSequence, true, StartingCharset, StartingTable>,
                                   B5Decoder<MaxSymbols, TerminatedSequence, true, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__WCHAR
template<size_t MaxSymbols = 255, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
         b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
         b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using B5WStringField =
    wStringField<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
                 B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__U8_CHAR
template<typename ApplicableTabless = FiveBitDefaultApplicableTabless, size_t MaxSymbols = 255>
using B5U8StringField =
    u8StringField<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
                  B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__U16_CHAR
template<size_t MaxSymbols = 255, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
         b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
         b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using B5U16StringField =
    u16StringField<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
                   B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#if SUB8_ENABLE_STRING_FIELDS__U32_CHAR
template<size_t MaxSymbols = 255, bool TerminatedSequence = false, bool StartingMultiCodeState = false,
         b5::B5CharAddress::Charset StartingCharset = b5::B5CharAddress::CHARSET_LATIN,
         b5::B5CharAddress::Table StartingTable = b5::B5CharAddress::TABLE_T0>
using B5U32StringField =
    u32StringField<B5Encoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>,
                   B5Decoder<MaxSymbols, TerminatedSequence, StartingMultiCodeState, StartingCharset, StartingTable>>;

#endif

#endif  // SUB8_ENABLE_FIVE_BIT_STRING
}  // namespace sub8
#endif  // SUB8_ENABLE_STRING_FIELDS
