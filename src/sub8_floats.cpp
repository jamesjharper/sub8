#include "sub8_floats.h"

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
