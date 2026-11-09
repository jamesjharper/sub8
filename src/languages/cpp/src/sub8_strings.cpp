// B5 lookup tables have one external definition shared by all translation units.
// The two generated headers below contain table initializers, not declarations.
#include "sub8_strings.h"

#if SUB8_ENABLE_STRING_FIELDS && SUB8_ENABLE_FIVE_BIT_STRING

namespace sub8::b5::ser::lut {
#include "sub8_strings_b5_encoding_lut.h"

const size_t B5EncoderLutSize = sizeof(B5EncoderLut) / sizeof(B5EncoderLut[0]);
} // namespace sub8::b5::ser::lut

namespace sub8::b5::de::lut {
#include "sub8_strings_b5_decoding_lut.h"
} // namespace sub8::b5::de::lut

#endif
