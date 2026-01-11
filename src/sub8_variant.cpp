#include "sub8_variant.h"

#if SUB8_ENABLE_VARIANT
template struct sub8::FixedLengthBitField<sub8::VariantValueType, /*DataBitsPerGroup=*/5>;
#endif