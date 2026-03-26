

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "./../src/sub8.h"
#include <catch2/catch_all.hpp>

using namespace sub8;

// Inline reading and writing
// --------------------------------------------------------

TEST_CASE("Inline reading and writing") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  write_or_throw<Boolean>(bit_writer, true);
  write_or_throw<U8>(bit_writer, 3);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto bool_read_val = read_or_throw<Boolean>(bit_reader);
  REQUIRE(bool_read_val == true);

  // using the read<T>() ensures `noexcept` for environments where throwing an
  // exception is undesirable
  U8 uint8_val;
  auto read_result = read(bit_reader, uint8_val);
  REQUIRE(read_result == BitFieldResult::Ok);
  REQUIRE(uint8_val == 3);
}

// Specialized non byte aligned types data transmission
// --------------------------------------------------------

TEST_CASE("Specialized data transmission types") {
  // Define a 3bit number which is stored as a uint8_t;
  using Uint3ValueField = Integer</*Bit Width*/ 3>;

  // Define a 16bit floating point with a exponent bit width of 8 and fraction
  // bit width of 7. AKA a "brain Float" or bfloat which is used by some ML
  // models Note: be mindful of floating point quantization error when
  // converting between custom float types
  using BFloat16ValueField = FloatingPoint</* Exponent Bit Width*/ 8,
    /* Fraction Bit Width*/ 7>;

  // Define a 24bit variable length number. This number will be transmitted in
  // upto 3 groups of 8bits. Each group is prefixed with continuation bit, and
  // will only use as may groups as the value requires. Signed ints are
  // transmitted using zigzag encoding to mitigate 2's complement on the MSB's
  using Int24Pack8ValueField = VbrInteger</* Signed */ true, 8, 8, 8>;

  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  write_or_throw<Uint3ValueField>(bit_writer, 1);
  write_or_throw<BFloat16ValueField>(bit_writer, 1.5f);
  write_or_throw<Int24Pack8ValueField>(bit_writer, 5234);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto uint3_read_val = read_or_throw<Uint3ValueField>(bit_reader);
  auto bfloat_read_val = read_or_throw<BFloat16ValueField>(bit_reader);
  auto int24p8_read_val = read_or_throw<Int24Pack8ValueField>(bit_reader);

  REQUIRE(uint3_read_val == 1);
  REQUIRE(bfloat_read_val == 1.5f);
  REQUIRE(int24p8_read_val == 5234);
}

// Optional data transmission types
// --------------------------------------------------------

TEST_CASE("Optional data transmission types") {
  // Define a 8bit number which is "optional". Th
  using OptionalU8 = Optional<U8>;

  // StdOptional is additionally available for an STL implementation,
  // however take note that this type allocates from the heap
  using StdOptionalU8 = StdOptional<U8>;

  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer

  // both types are functionally identical
  write_or_throw<OptionalU8>(bit_writer, OptionalU8::make_or_throw(1));
  write_or_throw<OptionalU8>(bit_writer, OptionalU8::make_none());

  write_or_throw<StdOptionalU8>(bit_writer, StdOptionalU8::make_or_throw(3));
  write_or_throw<StdOptionalU8>(bit_writer, StdOptionalU8::make_none());

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto opt_uint8_read_val = read_or_throw<OptionalU8>(bit_reader);
  auto opt_uint8_read_val_none = read_or_throw<OptionalU8>(bit_reader);
  auto std_opt_uint8_read_val = read_or_throw<StdOptionalU8>(bit_reader);
  auto std_opt_uint8_read_val_none = read_or_throw<StdOptionalU8>(bit_reader);

  REQUIRE(opt_uint8_read_val.has_value() == true);
  REQUIRE(opt_uint8_read_val == 1);
  REQUIRE(opt_uint8_read_val_none.has_value() == false);

  REQUIRE(std_opt_uint8_read_val.has_value() == true);
  REQUIRE(std_opt_uint8_read_val == 3);
  REQUIRE(std_opt_uint8_read_val_none.has_value() == false);
}

enum class MsgType : uint8_t {
  Ping = 10,
  Pong = 11,
  Data = 12,
  Ack = 13,
};

TEST_CASE("Enum type") {
  using MsgTypeField = Enumeration<MsgType, MsgType::Ping, MsgType::Ack>;

  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  write_or_throw<MsgTypeField>(bit_writer, MsgType::Data);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto enum_read_val = read_or_throw<MsgTypeField>(bit_reader);
  REQUIRE(enum_read_val == MsgType::Data);
}

// Macro Generated Data Transfer Data Transfer Object
// --------------------------------------------------------

SUB8_DECLARE_DTO(MacroGeneratedDataTransferObjectExample, (U16, feild_1), (U32, feild_2));

TEST_CASE("Macro Generated Data Transfer Data Transfer Object") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  MacroGeneratedDataTransferObjectExample transmit_dto;
  transmit_dto.feild_1 = 10;
  transmit_dto.feild_2 = 1;

  // Write object to bit stream
  write_or_throw(bit_writer, transmit_dto);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_dto = read_or_throw<MacroGeneratedDataTransferObjectExample>(bit_reader);

  REQUIRE(receive_dto.feild_1 == transmit_dto.feild_1);
  REQUIRE(receive_dto.feild_2 == transmit_dto.feild_2);
}

// Hand crafted Data Transfer Data Transfer Object
// --------------------------------------------------------

struct ManualDataTransferObjectExample {
  U16 feild_1{};
  U32 feild_2{};
};

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult write_field(BasicBitWriter<Storage> &bw,
  const ManualDataTransferObjectExample &v) {
  if (auto r = write_field(bw, v.feild_1); r != BitFieldResult::Ok)
    return r;
  if (auto r = write_field(bw, v.feild_2); r != BitFieldResult::Ok)
    return r;
  return BitFieldResult::Ok;
}

template <typename Storage> SUB8_NO_DISCARD inline BitFieldResult read_field(BasicBitReader<Storage> &br,
  ManualDataTransferObjectExample &out) {
  if (auto r = read_field(br, out.feild_1); r != BitFieldResult::Ok)
    return r;
  if (auto r = read_field(br, out.feild_2); r != BitFieldResult::Ok)
    return r;
  return BitFieldResult::Ok;
}

TEST_CASE("Hand crafted Data Transfer Data Transfer Object") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  ManualDataTransferObjectExample transmit_dto;
  transmit_dto.feild_1 = 10;
  transmit_dto.feild_2 = 1;

  // Write object to bit stream
  write_or_throw(bit_writer, transmit_dto);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  ManualDataTransferObjectExample receive_dto;
  read(bit_reader, receive_dto);

  REQUIRE(receive_dto.feild_1 == transmit_dto.feild_1);
  REQUIRE(receive_dto.feild_2 == transmit_dto.feild_2);
}

using Uint3Path = BufferArray<3 /* bits per element */, 0, 3 /* max elements */>;
using Uint10Array = BufferArray<10 /* bits per element */, 0, 6 /* max elements */>;
using Uint5FixedArray = FixedSizeBufferArray<5 /* bits per element */, 6 /* max elements */>;

// Reading and writing Paths, Array and fixed Arrays
// --------------------------------------------------------

TEST_CASE("Array Example") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer

  // Write a path. Path is a sequence of values delimited by a continuation bit
  // Will be more bit efficient for item lengths shorter then 5.
  write_or_throw<Uint3Path>(bit_writer, {1, 2, 3});

  auto init_with_std_array = std::array<uint8_t, 3>({4, 5, 6});
  write_or_throw<Uint3Path>(bit_writer, init_with_std_array);

  // Write an array. Array is a sequence of values prefixed by a length.
  auto init_with_vector = std::vector<uint16_t>({7, 8, 9});
  write_or_throw<Uint10Array>(bit_writer, init_with_vector);

  // Write an fixed array. Array is a sequence of a fixed known length.
  // Use this type if the sequence is known to always been the same
  uint16_t init_with_stack_array[] = {10, 11, 12, 13};
  write_or_throw<Uint5FixedArray>(bit_writer, init_with_stack_array);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto path1 = read_or_throw<Uint3Path>(bit_reader);
  auto path2 = read_or_throw<Uint3Path>(bit_reader);
  auto array = read_or_throw<Uint10Array>(bit_reader);
  auto fixed_array = read_or_throw<Uint5FixedArray>(bit_reader);
  // TODO: CHECK THE READ METHODS

  REQUIRE(path1 == Uint3Path({1, 2, 3}));
  REQUIRE(path2 == Uint3Path({4, 5, 6}));
  REQUIRE(array == Uint10Array({7, 8, 9}));
  REQUIRE(fixed_array == Uint5FixedArray({10, 11, 12, 13, 0, 0})); // Note the unused space is null padded
}

// Nested Objects
// --------------------------------------------------------

SUB8_DECLARE_DTO(NestedExample, (U16, feild_1), (U32, feild_2));

SUB8_DECLARE_DTO(NestedObjectExample, (NestedExample, inner));

TEST_CASE("Nested Object Type Example") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  NestedObjectExample transmit_object{.inner = NestedExample{.feild_1 = {1}, .feild_2 = {45}}};

  write_or_throw(bit_writer, transmit_object);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_dto = read_or_throw<NestedObjectExample>(bit_reader);

  REQUIRE(receive_dto.inner.feild_1 == transmit_object.inner.feild_1);
  REQUIRE(receive_dto.inner.feild_2 == transmit_object.inner.feild_2);
}

// Nested List Objects
// --------------------------------------------------------

SUB8_DECLARE_DTO(ItemExample, (U16, feild_1), (U32, feild_2));

using ItemExampleArray = ObjectArray<ItemExample, 0, 5>;

SUB8_DECLARE_DTO(ObjectArrayExample, (ItemExampleArray, list));

TEST_CASE("Nested Array Type Example") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer
  ObjectArrayExample transmit_object{
    .list = ItemExampleArray{ItemExample{.feild_1 = {1}, .feild_2 = {2}}, ItemExample{.feild_1 = {3}, .feild_2 = {4}}}};

  write_or_throw(bit_writer, transmit_object);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_dto = read_or_throw<ObjectArrayExample>(bit_reader);

  REQUIRE(receive_dto.list[0].feild_1 == transmit_object.list[0].feild_1);
  REQUIRE(receive_dto.list[0].feild_2 == transmit_object.list[0].feild_2);
  REQUIRE(receive_dto.list[1].feild_1 == transmit_object.list[1].feild_1);
  REQUIRE(receive_dto.list[1].feild_2 == transmit_object.list[1].feild_2);
}

// Variant types
// --------------------------------------------------------

TEST_CASE("Variant Example") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer

  auto transmit_var_1 = make_uint4_variant(2);
  auto transmit_var_2 = make_null_variant();

  write_or_throw(bit_writer, transmit_var_1);
  write_or_throw(bit_writer, transmit_var_2);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_var_1 = read_or_throw<PrimitiveVariant>(bit_reader);
  auto receive_var_2 = read_or_throw<PrimitiveVariant>(bit_reader);

  REQUIRE(receive_var_1 == transmit_var_1);
  REQUIRE(receive_var_2 == transmit_var_2);
}

// Custom Variant types
// --------------------------------------------------------

#define VARIANT_VALUE_CASES(define_type)                                                                                                   \
  define_type(boolean, 1, bool, Boolean) define_type(u4, 2, uint8_t, U4) define_type(u8, 3, uint8_t, U8)

SUB8_DECLARE_VARIANT_FIELD(CustomPrimitiveVariant, VARIANT_VALUE_CASES);

TEST_CASE("Custom Variant Example") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer

  auto transmit_var_1 = CustomPrimitiveVariant::make_boolean(true);
  auto transmit_var_2 = CustomPrimitiveVariant::make_u4(1);

  write_or_throw(bit_writer, transmit_var_1);
  write_or_throw(bit_writer, transmit_var_2);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_var_1 = read_or_throw<CustomPrimitiveVariant>(bit_reader);
  auto receive_var_2 = read_or_throw<CustomPrimitiveVariant>(bit_reader);

  REQUIRE(receive_var_1 == transmit_var_1);
  REQUIRE(receive_var_2 == transmit_var_2);
}

// Strings
// --------------------------------------------------------

TEST_CASE("String Example") {
  // Initialize a target write buffer
  UnboundedBitWriter bit_writer;

  // Write fields to writer

  auto transmit_string = BoundedB5String<64>("héllö wor1d ");

  write_or_throw(bit_writer, transmit_string);

  // Initialize read buffer
  UnboundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_string = read_or_throw<BoundedB5String<64>>(bit_reader);
  REQUIRE(receive_string.value() == transmit_string.value());
}

// Putting it all together
// --------------------------------------------------------

SUB8_DECLARE_DTO(HelloRequestMessage, 
  (U16, say_it_n_time), 
  (BoundedB5String<64>, hello_phrase)
);

using BFloat16ValueField = FloatingPoint<8, 7>;

SUB8_DECLARE_DTO(MessageItem, 
  (U16, feild_1), 
  (U32, feild_2),
  (BFloat16ValueField, feild_3)
);

using MessageItemArray = ObjectArray<MessageItem, 0, 5>;

SUB8_DECLARE_DTO(HelloResponseMessage, 
  (BoundedB5String<64>,  response_phrase ), 
  (MessageItemArray, list)
);


#define HELLO_MESSAGE_SCHEMA(define_type)                                                                                                  \
  define_type(hello_request, /* id */ 1, /* type*/ HelloRequestMessage, /* storage type*/ HelloRequestMessage)                             \
  define_type(hello_response, /* id */ 2, /* type*/ HelloResponseMessage, /* storage type*/ HelloResponseMessage)

SUB8_DECLARE_VARIANT_FIELD(HelloMessage, HELLO_MESSAGE_SCHEMA);

TEST_CASE("Putting it all together Example!") {
  // Initialize a target write buffer
  auto bit_writer = make_bounded_writer_for<HelloMessage>();

  // Write fields to writer
  HelloRequestMessage msg{};
  msg.hello_phrase = "hello how are you";
  msg.say_it_n_time = 10;

  auto transmit_message = HelloMessage::make_hello_request(msg);

  write_or_throw(bit_writer, transmit_message);

  // Initialize read buffer
  BoundedBitReader bit_reader(bit_writer.storage(), bit_writer.size());

  // read values in the same order as written
  auto receive_message = read_or_throw<HelloMessage>(bit_reader);

  REQUIRE(receive_message == transmit_message);
}
