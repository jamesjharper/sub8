#pragma once

// Auto-generated (stage3_emit) GDCAR5

#include <cstddef>
#include <cstdint>
#include <utility>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 1
#endif

#ifndef SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_ENABLE_NEVER_THROW_EXCEPTIONS 0
#endif

#ifndef SUB8_ENABLE_INFALLIBLE
#define SUB8_ENABLE_INFALLIBLE 0
#endif

#ifndef SUB8_ENABLE_NO_DISCARD_RESULTS
#define SUB8_ENABLE_NO_DISCARD_RESULTS 0
#endif

#ifndef SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#define SUB8_ENABLE_NO_MALLOC_EXCEPTIONS 0
#endif

#ifndef SUB8_ENABLE_STL_TYPE
#define SUB8_ENABLE_STL_TYPE 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_VECTOR
#define SUB8_ENABLE_STL_TYPE_VECTOR 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_ARRAY
#define SUB8_ENABLE_STL_TYPE_ARRAY 1
#endif

#ifndef SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST
#define SUB8_ENABLE_STL_TYPE_INITIALIZER_LIST 1
#endif

#ifndef SUB8_ENABLE_BASIC_STRING
#define SUB8_ENABLE_BASIC_STRING 1
#endif

#ifndef SUB8_ENABLE_OPTIONAL_FIELDS
#define SUB8_ENABLE_OPTIONAL_FIELDS 1
#endif

#ifndef SUB8_ENABLE_BOUNDED_BUF
#define SUB8_ENABLE_BOUNDED_BUF 1
#endif

#ifndef SUB8_ENABLE_UNBOUNDED_BUF
#define SUB8_ENABLE_UNBOUNDED_BUF 1
#endif

#ifndef SUB8_ENABLE_VIEW_BUF
#define SUB8_ENABLE_VIEW_BUF 1
#endif

#ifndef SUB8_ENABLE_CHECKED_ARITHMETIC
#define SUB8_ENABLE_CHECKED_ARITHMETIC 1
#endif

#include "./sub8_errors.h"

#include "./sub8_type_information.h"

#include "./sub8_io.h"

#include "./sub8_api.h"

#include "./sub8_strings.h"

#include "./sub8_primitives.h"

#include "./sub8_floats.h"

#include "./sub8_arrays.h"

#include "./sub8_enums.h"

using HelloRequestMessage_hello_phrase_T1 = sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>;

using u16 = sub8::Integer<16, false>;

namespace users {
namespace name {
namespace space {
struct HelloRequestMessage {
  using Type = HelloRequestMessage;
  using InitType = HelloRequestMessage;
  using ValueType = HelloRequestMessage;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MaxPossibleSize;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MaxPossibleSize;
    return len;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MinPossibleSize;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MinPossibleSize;
    return len;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  const HelloRequestMessage& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const HelloRequestMessage& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }

  sub8::Integer<16, false> say_it_n_time{};
  sub8::BoundedString<64,
    sub8::B5Encoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<128, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>> hello_phrase{};

  bool operator==(const HelloRequestMessage& o) const noexcept;
  bool operator!=(const HelloRequestMessage& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::HelloRequestMessage& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = write_field(bw, v.say_it_n_time);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.hello_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::HelloRequestMessage& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = read_field(br, out.say_it_n_time);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.hello_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}
} // namespace sub8

using bfloat16 = sub8::FloatingPoint<8, 7>;

using u32 = sub8::Integer<32, false>;

namespace users {
namespace name {
namespace space {
struct MessageItem {
  using Type = MessageItem;
  using InitType = MessageItem;
  using ValueType = MessageItem;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MaxPossibleSize;
    len += sub8::Integer<32, false>::MaxPossibleSize;
    len += sub8::FloatingPoint<8, 7>::MaxPossibleSize;
    return len;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::Integer<16, false>::MinPossibleSize;
    len += sub8::Integer<32, false>::MinPossibleSize;
    len += sub8::FloatingPoint<8, 7>::MinPossibleSize;
    return len;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  const MessageItem& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const MessageItem& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }

  sub8::Integer<16, false> feild_1{};
  sub8::Integer<32, false> feild_2{};
  sub8::FloatingPoint<8, 7> feild_3{};

  bool operator==(const MessageItem& o) const noexcept;
  bool operator!=(const MessageItem& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::MessageItem& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = write_field(bw, v.feild_1);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.feild_2);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.feild_3);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::MessageItem& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = read_field(br, out.feild_1);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.feild_2);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.feild_3);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}
} // namespace sub8

using HelloResponseMessage_list_T3 = sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited>;

using HelloResponseMessage_response_phrase_T2 = sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>;

namespace users {
namespace name {
namespace space {
struct HelloResponseMessage {
  using Type = HelloResponseMessage;
  using InitType = HelloResponseMessage;
  using ValueType = HelloResponseMessage;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MaxPossibleSize;
    len += sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited>::MaxPossibleSize;
    return len;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize len;
    len += sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>>::MinPossibleSize;
    len += sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited>::MinPossibleSize;
    return len;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  const HelloResponseMessage& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const HelloResponseMessage& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }

  sub8::BoundedString<64,
    sub8::B5Encoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>,
    sub8::B5Decoder<64, false, false,
                    sub8::b5::B5CharAddress::CHARSET_LATIN, sub8::b5::B5CharAddress::TABLE_T0>> response_phrase{};
  sub8::Array<sub8::ObjectElement<users::name::space::MessageItem>, 0, 5, sub8::ArrayEncoding::Delimited> list{};

  bool operator==(const HelloResponseMessage& o) const noexcept;
  bool operator!=(const HelloResponseMessage& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::HelloResponseMessage& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = write_field(bw, v.response_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = write_field(bw, v.list);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::HelloResponseMessage& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  r = read_field(br, out.response_phrase);
  if (r != sub8::BitFieldResult::Ok) return r;
  r = read_field(br, out.list);
  if (r != sub8::BitFieldResult::Ok) return r;
  return sub8::BitFieldResult::Ok;
}
} // namespace sub8

namespace users {
namespace name {
namespace space {
enum class HelloMessageType : uint32_t {
  NullValue = 0,
  hello_request = 1,
  hello_response = 2,
};

struct HelloMessage__VariantTagMeta {
  inline static constexpr uint32_t kCodes[] = {
    0u,
    1u,
    2u,
  };
  inline static constexpr size_t kCount = sizeof(kCodes) / sizeof(kCodes[0]);
  inline static constexpr uint32_t MinCodeU32 = []() constexpr noexcept {
    uint32_t m = kCodes[0];
    for (size_t i = 1; i < kCount; ++i) if (kCodes[i] < m) m = kCodes[i];
    return m;
  }();
  inline static constexpr uint32_t MaxCodeU32 = []() constexpr noexcept {
    uint32_t m = kCodes[0];
    for (size_t i = 1; i < kCount; ++i) if (kCodes[i] > m) m = kCodes[i];
    return m;
  }();
  inline static constexpr HelloMessageType MinEnum = static_cast<HelloMessageType>(MinCodeU32);
  inline static constexpr HelloMessageType MaxEnum = static_cast<HelloMessageType>(MaxCodeU32);
};

using HelloMessageTypeFieldBase = sub8::Enumeration<HelloMessageType, HelloMessage__VariantTagMeta::MinEnum, HelloMessage__VariantTagMeta::MaxEnum>;
struct HelloMessageTypeField : HelloMessageTypeFieldBase {
  using HelloMessageTypeFieldBase::HelloMessageTypeFieldBase;
  HelloMessageTypeField() noexcept = default;
};

struct HelloMessage {
  HelloMessageType type{HelloMessageType::NullValue};
  union VariantValue {
    sub8::NullValue null_v;
    users::name::space::HelloRequestMessage hello_request;
    users::name::space::HelloResponseMessage hello_response;
    VariantValue() {}
    ~VariantValue() {}
  } variant;

  using Type = HelloMessage;
  using InitType = HelloMessage;
  using ValueType = HelloMessage;

  static constexpr sub8::BitSize MaxPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize payload = sub8::NullValue::MaxPossibleSize;
    if (users::name::space::HelloRequestMessage::MaxPossibleSize > payload) payload = users::name::space::HelloRequestMessage::MaxPossibleSize;
    if (users::name::space::HelloResponseMessage::MaxPossibleSize > payload) payload = users::name::space::HelloResponseMessage::MaxPossibleSize;
    return payload + HelloMessageTypeField::MaxPossibleSize;
  }();

  static constexpr sub8::BitSize MinPossibleSize = []() constexpr noexcept -> sub8::BitSize {
    sub8::BitSize payload = sub8::NullValue::MinPossibleSize;
    if (users::name::space::HelloRequestMessage::MinPossibleSize < payload) payload = users::name::space::HelloRequestMessage::MinPossibleSize;
    if (users::name::space::HelloResponseMessage::MinPossibleSize < payload) payload = users::name::space::HelloResponseMessage::MinPossibleSize;
    return payload + HelloMessageTypeField::MinPossibleSize;
  }();

  sub8::BitSize max_possible_size() { return MaxPossibleSize; }
  sub8::BitSize min_possible_size() { return MinPossibleSize; }
  sub8::BitSize actual_size() const noexcept;

  void construct_null() noexcept;
  void destroy_active() noexcept;

  HelloMessage() noexcept;
  HelloMessage(const HelloMessage& o);
  HelloMessage(HelloMessage&& o) noexcept;
  HelloMessage& operator=(const HelloMessage& o);
  HelloMessage& operator=(HelloMessage&& o) noexcept;
  ~HelloMessage();

  const HelloMessage& value() const noexcept { return *this; }
  sub8::BitFieldResult set_value(const HelloMessage& v) noexcept { *this = v; return sub8::BitFieldResult::Ok; }
  bool is_null() const noexcept;

  const users::name::space::HelloRequestMessage* get_hello_request() const noexcept;
  bool is_hello_request() const noexcept;
  sub8::BitFieldResult set_hello_request(const users::name::space::HelloRequestMessage& v) noexcept;

  const users::name::space::HelloResponseMessage* get_hello_response() const noexcept;
  bool is_hello_response() const noexcept;
  sub8::BitFieldResult set_hello_response(const users::name::space::HelloResponseMessage& v) noexcept;

  bool operator==(const HelloMessage& o) const noexcept;
  bool operator!=(const HelloMessage& o) const noexcept { return !(*this == o); }
};
} // namespace space
} // namespace name
} // namespace users

namespace sub8 {
template <typename Storage>
inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const users::name::space::HelloMessage& v) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  users::name::space::HelloMessageTypeField t; t.set_value(static_cast<users::name::space::HelloMessageType>(static_cast<uint32_t>(v.type)));
  r = write_field(bw, t);
  if (r != sub8::BitFieldResult::Ok) return r;
  switch (static_cast<uint32_t>(v.type)) {
    case 0u: return sub8::BitFieldResult::Ok;
    case 1u: return write_field(bw, v.variant.hello_request);
    case 2u: return write_field(bw, v.variant.hello_response);
    default: return sub8::BitFieldResult::Ok;
  }
}

template <typename Storage>
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, users::name::space::HelloMessage& out) {
  sub8::BitFieldResult r = sub8::BitFieldResult::Ok;
  users::name::space::HelloMessageTypeField t;
  r = read_field(br, t);
  if (r != sub8::BitFieldResult::Ok) return r;
  out.destroy_active();
  out.type = static_cast<users::name::space::HelloMessageType>(static_cast<uint32_t>(t.value()));
  switch (static_cast<uint32_t>(out.type)) {
    case 0u: out.construct_null(); return sub8::BitFieldResult::Ok;
    case 1u: new (&out.variant.hello_request) users::name::space::HelloRequestMessage{}; return read_field(br, out.variant.hello_request);
    case 2u: new (&out.variant.hello_response) users::name::space::HelloResponseMessage{}; return read_field(br, out.variant.hello_response);
    default: out.construct_null(); return sub8::BitFieldResult::Ok;
  }
}
} // namespace sub8

