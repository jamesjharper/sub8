// Auto-generated (stage3_emit)

#include "example.h"
#include <utility>


// ============================================================
// BEGIN INLINE ./sub8_strings.cpp
// ============================================================

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_strings.h"
#endif 

#if SUB8_ENABLE_FIVE_BIT_STRING

#define B5_CHAR_ADDRESS(ROW, EXT, CS, TBL)                                                                                                 \
  B5CharAddress { ROW, EXT, CS, TBL }

namespace sub8::b5::ser::lut {
#include "sub8_strings_b5_encoding_lut.h"
const size_t B5EncoderLutSize = sizeof(B5EncoderLut) / sizeof(B5EncoderLut[0]);
} // namespace sub8::b5::ser::lut

namespace sub8::b5::de::lut {
#include "sub8_strings_b5_decoding_lut.h"
}
#endif

// ============================================================
// END INLINE ./sub8_strings.cpp
// ============================================================


sub8::BitSize users::name::space::HelloRequestMessage::actual_size() const noexcept {
  sub8::BitSize len;
  len += say_it_n_time.actual_size();
  len += hello_phrase.actual_size();
  return len;
}

bool users::name::space::HelloRequestMessage::operator==(const users::name::space::HelloRequestMessage& o) const noexcept {
  if (say_it_n_time != o.say_it_n_time) return false;
  if (hello_phrase != o.hello_phrase) return false;
  return true;
}

sub8::BitSize users::name::space::MessageItem::actual_size() const noexcept {
  sub8::BitSize len;
  len += feild_1.actual_size();
  len += feild_2.actual_size();
  len += feild_3.actual_size();
  return len;
}

bool users::name::space::MessageItem::operator==(const users::name::space::MessageItem& o) const noexcept {
  if (feild_1 != o.feild_1) return false;
  if (feild_2 != o.feild_2) return false;
  if (feild_3 != o.feild_3) return false;
  return true;
}

sub8::BitSize users::name::space::HelloResponseMessage::actual_size() const noexcept {
  sub8::BitSize len;
  len += response_phrase.actual_size();
  len += list.actual_size();
  return len;
}

bool users::name::space::HelloResponseMessage::operator==(const users::name::space::HelloResponseMessage& o) const noexcept {
  if (response_phrase != o.response_phrase) return false;
  if (list != o.list) return false;
  return true;
}

sub8::BitSize users::name::space::HelloMessage::actual_size() const noexcept {
  auto size = HelloMessageTypeField::ActualSize;
  switch (static_cast<uint32_t>(type)) {
    case 0u: return sub8::BitSize::from_bits(0) + size;
    case 1u: size += variant.hello_request.actual_size(); break;
    case 2u: size += variant.hello_response.actual_size(); break;
    default: return sub8::BitSize::from_bits(0) + size;
  }
  return size;
}

void users::name::space::HelloMessage::construct_null() noexcept { new (&variant.null_v) sub8::NullValue{}; type = HelloMessageType::NullValue; }

void users::name::space::HelloMessage::destroy_active() noexcept {
  switch (static_cast<uint32_t>(type)) {
    case 0u: variant.null_v.~NullValue(); break;
    case 1u: variant.hello_request.~HelloRequestMessage(); break;
    case 2u: variant.hello_response.~HelloResponseMessage(); break;
    default: break;
  }
}

users::name::space::HelloMessage::HelloMessage() noexcept { construct_null(); }

users::name::space::HelloMessage::HelloMessage(const users::name::space::HelloMessage& o) {
  type = o.type;
  switch (static_cast<uint32_t>(type)) {
    case 0u: new (&variant.null_v) sub8::NullValue(o.variant.null_v); break;
    case 1u: new (&variant.hello_request) users::name::space::HelloRequestMessage(o.variant.hello_request); break;
    case 2u: new (&variant.hello_response) users::name::space::HelloResponseMessage(o.variant.hello_response); break;
    default: construct_null(); break;
  }
}

users::name::space::HelloMessage::HelloMessage(users::name::space::HelloMessage&& o) noexcept {
  type = o.type;
  switch (static_cast<uint32_t>(type)) {
    case 0u: new (&variant.null_v) sub8::NullValue(std::move(o.variant.null_v)); break;
    case 1u: new (&variant.hello_request) users::name::space::HelloRequestMessage(std::move(o.variant.hello_request)); break;
    case 2u: new (&variant.hello_response) users::name::space::HelloResponseMessage(std::move(o.variant.hello_response)); break;
    default: construct_null(); break;
  }
}

users::name::space::HelloMessage& users::name::space::HelloMessage::operator=(const users::name::space::HelloMessage& o) {
  if (this == &o) return *this;
  destroy_active();
  type = o.type;
  switch (static_cast<uint32_t>(type)) {
    case 0u: new (&variant.null_v) sub8::NullValue(o.variant.null_v); break;
    case 1u: new (&variant.hello_request) users::name::space::HelloRequestMessage(o.variant.hello_request); break;
    case 2u: new (&variant.hello_response) users::name::space::HelloResponseMessage(o.variant.hello_response); break;
    default: construct_null(); break;
  }
  return *this;
}

users::name::space::HelloMessage& users::name::space::HelloMessage::operator=(users::name::space::HelloMessage&& o) noexcept {
  if (this == &o) return *this;
  destroy_active();
  type = o.type;
  switch (static_cast<uint32_t>(type)) {
    case 0u: new (&variant.null_v) sub8::NullValue(std::move(o.variant.null_v)); break;
    case 1u: new (&variant.hello_request) users::name::space::HelloRequestMessage(std::move(o.variant.hello_request)); break;
    case 2u: new (&variant.hello_response) users::name::space::HelloResponseMessage(std::move(o.variant.hello_response)); break;
    default: construct_null(); break;
  }
  return *this;
}

users::name::space::HelloMessage::~HelloMessage() { destroy_active(); }

bool users::name::space::HelloMessage::is_null() const noexcept { return static_cast<uint32_t>(type) == 0u; }

const users::name::space::HelloRequestMessage* users::name::space::HelloMessage::get_hello_request() const noexcept {
  if (static_cast<uint32_t>(type) == 1u) return &variant.hello_request;
  return nullptr;
}

bool users::name::space::HelloMessage::is_hello_request() const noexcept { return static_cast<uint32_t>(type) == 1u; }

sub8::BitFieldResult users::name::space::HelloMessage::set_hello_request(const users::name::space::HelloRequestMessage& v) noexcept {
  destroy_active();
  new (&variant.hello_request) users::name::space::HelloRequestMessage(v);
  type = static_cast<HelloMessageType>(1u);
  return sub8::BitFieldResult::Ok;
}

const users::name::space::HelloResponseMessage* users::name::space::HelloMessage::get_hello_response() const noexcept {
  if (static_cast<uint32_t>(type) == 2u) return &variant.hello_response;
  return nullptr;
}

bool users::name::space::HelloMessage::is_hello_response() const noexcept { return static_cast<uint32_t>(type) == 2u; }

sub8::BitFieldResult users::name::space::HelloMessage::set_hello_response(const users::name::space::HelloResponseMessage& v) noexcept {
  destroy_active();
  new (&variant.hello_response) users::name::space::HelloResponseMessage(v);
  type = static_cast<HelloMessageType>(2u);
  return sub8::BitFieldResult::Ok;
}

bool users::name::space::HelloMessage::operator==(const users::name::space::HelloMessage& o) const noexcept {
  if (static_cast<uint32_t>(type) != static_cast<uint32_t>(o.type)) return false;
  switch (static_cast<uint32_t>(type)) {
    case 0u: return true;
    case 1u: return variant.hello_request == o.variant.hello_request;
    case 2u: return variant.hello_response == o.variant.hello_response;
    default: return true;
  }
}

