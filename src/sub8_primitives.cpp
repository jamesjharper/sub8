#include "sub8_primitives.h"

#if SUB8_ENABLE_BOOL
template struct sub8::BasicValueField<bool, 1>;
#endif

// Unsigned integer fields can use the bitfield templates directly.
#if SUB8_ENABLE_UINT4
template struct sub8::FixedLengthBitField<uint8_t, /*DataBitsPerGroup=*/4>;
#endif

#if SUB8_ENABLE_UINT8
template struct sub8::FixedLengthBitField<uint8_t, /*DataBitsPerGroup=*/8>;
#endif

#if SUB8_ENABLE_UINT16
template struct sub8::FixedLengthBitField<uint16_t, /*DataBitsPerGroup=*/16>;
#endif

#if SUB8_ENABLE_UINT32
template struct sub8::FixedLengthBitField<uint32_t, /*DataBitsPerGroup=*/32>;
template struct sub8::VariableLengthBitField<uint32_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 4>;
template struct sub8::VariableLengthBitField<uint32_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 2>;
#endif

#if SUB8_ENABLE_UINT64
template struct sub8::FixedLengthBitField<uint64_t, /*DataBitsPerGroup=*/64>;
template struct sub8::VariableLengthBitField<uint64_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 8>;
template struct sub8::VariableLengthBitField<uint64_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 4>;
template struct sub8::VariableLengthBitField<uint64_t, /*DataBitsPerGroup=*/32, /* Max Groups */ 2>;
#endif

#if SUB8_ENABLE_INT8
template struct sub8::FixedLengthBitField<int8_t, /*DataBitsPerGroup=*/8>;
#endif

#if SUB8_ENABLE_INT16
template struct sub8::FixedLengthBitField<int16_t, /*DataBitsPerGroup=*/16>;
#endif

#if SUB8_ENABLE_INT32
template struct sub8::FixedLengthBitField<int32_t, /*DataBitsPerGroup=*/32>;
template struct sub8::VariableLengthBitField<int32_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 4>;
template struct sub8::VariableLengthBitField<int32_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 2>;
#endif

#if SUB8_ENABLE_INT64
template struct sub8::FixedLengthBitField<int64_t, /*DataBitsPerGroup=*/64>;
template struct sub8::VariableLengthBitField<int64_t, /*DataBitsPerGroup=*/8, /* Max Groups */ 8>;
template struct sub8::VariableLengthBitField<int64_t, /*DataBitsPerGroup=*/16, /* Max Groups */ 4>;
template struct sub8::VariableLengthBitField<int64_t, /*DataBitsPerGroup=*/32, /* Max Groups */ 2>;
#endif

#if SUB8_ENABLE_FLOAT16
template struct sub8::FixedLengthFloatField<float, 5, 10>;
template struct sub8::FixedLengthFloatField<float, 8, 7>;
#endif

#if SUB8_ENABLE_FLOAT24
template struct sub8::FixedLengthFloatField<float, 7, 16>;
#endif

#if SUB8_ENABLE_FLOAT32
template struct sub8::FixedLengthFloatField<float, 8, 23>;
#endif

#if SUB8_ENABLE_FLOAT48
template struct sub8::FixedLengthFloatField<double, 11, 48>;
#endif

#if SUB8_ENABLE_FLOAT64
template struct sub8::FixedLengthFloatField<double, 11, 52>;
#endif

#if SUB8_ENABLE_VARIANT
template struct sub8::FixedLengthBitField<sub8::VariantValueType, /*DataBitsPerGroup=*/5>;
#endif