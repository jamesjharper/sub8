#pragma once

// Enable: Prevents any memory allocation while throwing exceptions in exchange for loss of error context infomation
#ifndef SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#define SUB8_ENABLE_NO_MALLOC_EXCEPTIONS 0
#endif

// Enable: Prevents any throw exceptions durring assignment, ctor and arithmetic operators by replacing with
// runtime assertions. Allows "noexcept" semantics in exchange for UB operations.
// Note: add, sub, mul, div, set_value operators can be used instead instead to prevent UB.
#ifndef SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_ENABLE_NEVER_THROW_EXCEPTIONS 0
#endif

#if SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_OPT_NO_EXCEPT noexcept
#else
#define SUB8_OPT_NO_EXCEPT
#endif

// Enable: Disables access to any method capable of throwing an exception or asserting an error 
#if SUB8_ENABLE_INFALLIBLE
#define SUB8_ENABLE_INFALLIBLE 0
#endif

// Enable: Prevents checked methods for enforcing [[nodiscard]]. Would recommend leaving on, and using *_uncheck(...) and *_or_throw(..)
#ifndef SUB8_ENABLE_NO_DISCARD_RESULTS
#define SUB8_ENABLE_NO_DISCARD_RESULTS 1
#endif

#if SUB8_ENABLE_NO_DISCARD_RESULTS
#define SUB8_NO_DISCARD [[nodiscard]]
#else
#define SUB8_NO_DISCARD
#endif

#if !SUB8_ENABLE_NO_MALLOC_EXCEPTIONS
#include <string>
#endif

namespace sub8 {

// Error Types
enum class BitFieldResult : uint8_t {
  Ok = 0,
  WarningNotSupportedConfiguration,
  ErrorInvalidBitFieldValue,
  ErrorTooManyElements,
  ErrorTooManyFragments,
  ErrorCanNotBeEmpty,
  ErrorValueOverflow,
  ErrorExpectedMoreBits,
  ErrorInsufficentBufferSize,
  ErrorCanNotWriteNullValue,
  ErrorCanNotReadNullValue,
  ErrorTooManyBits,
  // IO Errors
  ErrorBadAlloc,
  ErrorOversizedLength,

  ErrorOptionalValueIsEmpty,

  ErrorUnidentifiedError,

};

namespace type_info {
namespace _no_rtti {
// Hack to extract type name info without RTTI at compile time
constexpr std::string_view extract_type_name(std::string_view p) noexcept {
#if defined(__clang__)
  auto start = p.find("T = ") + 4;
  auto end = p.find(']', start);
  return p.substr(start, end - start);
#elif defined(__GNUC__)
  auto start = p.find("T = ") + 4;
  auto end = p.find(';', start);
  return p.substr(start, end - start);
#elif defined(_MSC_VER)
  auto start = p.find("name_sv<") + 8;
  auto end = p.find(">(void)", start);
  return p.substr(start, end - start);
#else
#if defined(__GXX_RTTI) || defined(_CPPRTTI)
  return typeid(T).name();
#else
  return "type";
#endif
#endif
}

template <class T> constexpr std::string_view inferred_type_name() noexcept {
#if defined(__clang__) || defined(__GNUC__)
  return _no_rtti::extract_type_name(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
  return _no_rtti::extract_type_name(__FUNCSIG__);
#else
  return "type";
#endif
}

} // namespace _no_rtti

template <typename T> inline std::string_view name() noexcept {
  if constexpr (std::is_polymorphic_v<T>) {
#if defined(__GXX_RTTI) || defined(_CPPRTTI)
    return typeid(T).name();
#else
    return _no_rtti::inferred_type_name<T>();
#endif
  } else {
    return _no_rtti::inferred_type_name<T>();
  }
}
} // namespace type_info

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
  case BitFieldResult::ErrorValueOverflow:
    return "Value is larger or smaller then the field value can hold";
  case BitFieldResult::ErrorExpectedMoreBits:
    return "Cant read value from as the buffer has less bytes available then expected";
  case BitFieldResult::ErrorTooManyBits:
    return "Cant read/write value to buffer as it has more bits the the "
           "provided storage type.";
  case BitFieldResult::ErrorInsufficentBufferSize:
    return "Can write to buffer as there is insufficent space available";
  case BitFieldResult::ErrorCanNotWriteNullValue:
    return "Can not write type \"NullValue\" to stream";
  case BitFieldResult::ErrorCanNotReadNullValue:
    return "Can not read type \"NullValue\" to stream";
  case BitFieldResult::ErrorBadAlloc:
    return "Allocation failed";
  case BitFieldResult::ErrorOversizedLength:
    return "Requested size exceeds max_size()";

  case BitFieldResult::ErrorOptionalValueIsEmpty:
    return "Can not unwrap value as optional value is empty";

  case BitFieldResult::ErrorUnidentifiedError:
    return "STL subsystem threw an unexpected exception type";
  }
  return "UnknownBitFieldResult";
}

#if SUB8_ENABLE_NEVER_THROW_EXCEPTIONS
#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE) assert(R == BitFieldResult::Ok && WHERE);

#else

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

#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE)                                                                                    \
  throw ::sub8::error::bitfield_static_error((R), (WHERE), sub8::type_info::name<TFIELDTYPE>())

#else
struct bitfield_dynamic_error final : std::runtime_error {
  BitFieldResult code;
  std::string_view field_type_;

  bitfield_dynamic_error(BitFieldResult r, const char *where, std::string_view field_type)
      : std::runtime_error(std::string(where) + " where T = " + std::string(field_type) + " failed with error \"" + to_string(r) + "\""),
        code(r), field_type_(field_type) {}

  constexpr const char *code_cstr() const noexcept { return to_string(code); }
};

using bitfield_error = bitfield_dynamic_error;

#define SUB8_THROW_BITFIELD_ERROR(R, TFIELDTYPE, WHERE)                                                                                    \
  throw ::sub8::error::bitfield_dynamic_error((R), (WHERE), sub8::type_info::name<TFIELDTYPE>())

#endif
#endif

#define SUB8_THROW_IF_ERROR(R, TFIELDTYPE, WHERE)                                                                                          \
  {                                                                                                                                        \
    BitFieldResult __r = R;                                                                                                                \
    if (__r != BitFieldResult::Ok) {                                                                                                       \
      SUB8_THROW_BITFIELD_ERROR(__r, TFIELDTYPE, WHERE);                                                                                   \
    }                                                                                                                                      \
  }

inline BitFieldResult map_std_exception_to_bitfield_result() noexcept {
  try {
    throw; // rethrow active exception
  } catch (const std::bad_alloc &) {
    return BitFieldResult::ErrorBadAlloc;
  } catch (const std::length_error &) {
    return BitFieldResult::ErrorOversizedLength;
  } catch (...) {
    return BitFieldResult::ErrorUnidentifiedError;
  }
}

} // namespace error
} // namespace sub8