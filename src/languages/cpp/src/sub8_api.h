#pragma once

#include <cstdint>

#ifndef SUB8_INLINE_HEADERS
#define SUB8_INLINE_HEADERS 0
#endif
#if SUB8_INLINE_HEADERS == 0
#include "sub8_errors.h"
#endif

namespace sub8 {

template <typename Storage> class BasicBitReader;
template <typename Storage> class BasicBitWriter;

// Reads and returns the value as the feild type
template <typename TFeildType, typename Storage> inline BitFieldResult read(BasicBitReader<Storage> &br, TFeildType &out) {
  TFeildType f{};
  auto r = read_field(br, f);
  if (r == BitFieldResult::Ok) {
    out = f;
  }
  return r;
}

template <typename TFeildType, typename Storage> inline TFeildType read_or_throw(BasicBitReader<Storage> &br) {
  TFeildType ret{};
  auto r = read_field(br, ret);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType, "sub8::read_or_throw<TFeildType>(BasicBitReader<Storage>)");
  }
  return ret;
}

// Write

template <typename TFeildType, typename Storage> inline void write_or_throw(BasicBitWriter<Storage> &br, TFeildType in) {
  auto r = write_field(br, in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
      "sub8::write_or_throw<TFeildType>(BasicBitWriter<"
      "Storage>, TFeildType)");
  }
}

template <typename TFeildType, typename Storage> inline void write_or_throw(BasicBitWriter<Storage> &br, typename TFeildType::InitType in) {
  TFeildType v{};
  auto r = v.set_value(in);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
      "sub8::write_or_throw<TFeildType>(BasicBitWriter<"
      "Storage>, TFeildType::InitType)");
  }

  r = write_field(br, v);
  if (r != BitFieldResult::Ok) {
    SUB8_THROW_BITFIELD_ERROR(r, TFeildType,
      "sub8::write_or_throw<TFeildType>(BasicBitWriter<"
      "Storage>, TFeildType::InitType)");
  }
}
} // namespace sub8