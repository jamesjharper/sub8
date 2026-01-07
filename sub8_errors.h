#pragma once

#ifndef SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#define SUB8_ENABLE_NO_MALLOC_EXCEPTIONS 0
#endif

#if !SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#include <string>
#endif

#include "sub8_type_infomation.h"

namespace sub8 {

// Error Types
enum class BitFieldResult : uint8_t {
  Ok = 0,
  WarningNotSupportedConfiguration,
  ErrorInvalidBitFieldValue,
  ErrorTooManyElements,
  ErrorTooManyFragments,
  ErrorCanNotBeEmpty,
  ErrorValueTooLarge,
  ErrorExpectedMoreBits,
  ErrorInsufficentBufferSize,
  ErrorCanNotWriteNullValue,
  ErrorCanNotReadNullValue,
};

namespace error {
inline const char *to_string(BitFieldResult r) {
  switch (r) {
    case BitFieldResult::Ok:
      return "Ok";
    case BitFieldResult::WarningNotSupportedConfiguration:
      return "WarningNotSupportedConfiguration";
    case BitFieldResult::ErrorInvalidBitFieldValue:
      return "ErrorInvalidBitFieldValue";
    case BitFieldResult::ErrorTooManyElements:
      return "Value has more elements then the field value can hold";
    case BitFieldResult::ErrorTooManyFragments:
      return "Value has more fragments then the field value expected";
    case BitFieldResult::ErrorCanNotBeEmpty:
      return "Value can not be empty.";
    case BitFieldResult::ErrorValueTooLarge:
      return "Value is larger or smaller then the field value can hold";
    case BitFieldResult::ErrorExpectedMoreBits:
      return "Can read value from as the buffer has less bytes available then expected";
    case BitFieldResult::ErrorInsufficentBufferSize:
      return "Can write to buffer as there is insufficent space available";
    case BitFieldResult::ErrorCanNotWriteNullValue:
      return "Can not write type \"NullValueField\" to stream";
    case BitFieldResult::ErrorCanNotReadNullValue:
      return "Can not read type \"NullValueField\" to stream";
  }
  return "UnknownBitFieldResult";
}

#if SUB8_ENABLE_NO_MALLOC_EXCEPTIONS

// Heap-free exceptions
struct bitfield_static_error final : std::exception {
  BitFieldResult code;
  const char *where_;
  std::string_view field_type_;

  bitfield_static_error(BitFieldResult r, const char *where, std::string_view field_type)
      : std::exception(), code(r), where_(where), field_type_(field_type) {}

  const char *what() const noexcept override { return to_string(code); }
};

using bitfield_error = bitfield_static_error;

#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE) \
  throw ::sub8::error::bitfield_static_error((R), (WHERE), sub8::type_info::name<TFIELDTYPE>())

#else
struct bitfield_dynamic_error final : std::runtime_error {
  BitFieldResult code;
  std::string_view field_type_;

  bitfield_dynamic_error(BitFieldResult r, const char *where, std::string_view field_type)
      : std::runtime_error(std::string(where) + " where T = " + std::string(field_type) + " failed with error \"" +
                           to_string(r) + "\""),
        code(r),
        field_type_(field_type) {}

  constexpr const char *code_cstr() const noexcept { return to_string(code); }
};

using bitfield_error = bitfield_dynamic_error;

#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE) \
  throw ::sub8::error::bitfield_dynamic_error((R), (WHERE), sub8::type_info::name<TFIELDTYPE>())

#endif

}  // namespace error
}  // namespace sub8