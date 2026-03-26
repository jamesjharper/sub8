

This document describes how to implement a yaml config for Sub8

```yaml

// there are types which are defined by a class within the source
// users could define this, but this is really so that we can encode for the inbuilt types
foundation_types:
    Integer:
        inc: "sub8_primitives.h"
        class_name: "Integer"
        namespace: "sub8"
        template:
            TBitLength:
                name: "bitwidth"
                index: 0
                type: "uint32_t"
            TSigned:
                name: "signed"
                index: 1
                type: "bool"

    VbrInteger:
        inc: "sub8_primitives.h"
        class_name: "VbrInteger"
        namespace: "sub8"
        template:
            TSigned:
                name: "signed"
                index: 0
                type: bool
            TSegments:
                name: "segments"
                index: 1
                type: "uint32_t..."

    Float:
        inc: "sub8_floats.h"
        class_name: "FloatingPoint"
        namespace: "sub8"
        template:
            TExpBits:
                name: "exponent"
                index: 0
                type: uint32_t
            TFracBits:
                name: "fraction"
                index: 1
                type: "uint32_t"

    Optional:
        class_name: "TOptionalContainer"
        inc: "sub8_optional.h"
        namespace: "sub8"
        template:
            TOptionalContainer:
                name: "optional_container_type"
                index: 0
                type: "typename"
                default_value: "sub8::non_alloc_optional" 
            TOptionalValue:
                name: "type"
                index: 1
                type: "typename"

    Array:
        class_name: "ArrayBitField"
        inc: "sub8_arrays.h"
        namespace: "sub8"
        template:
            ... How to do this?


// These are special types which have a code gen element to their use
// their configuration might need some tweaks to better generalize what they are trying to do
    Enumeration:
        inc: "sub8_enums.h"
        class_name: "Enumeration"
        namespace: "sub8"
        template:
            EnumT:
                name: "type" // These value will require special attention it can be commuted though inspection of the values
                index: 0
                type: typename
            MinEnumValue:  // These values will require special attention as they can/should be computed while defining the code 
                name: "min_enum"
                index: 1
                type: "uint32_t" 
            MinEnumValue:
                name: "max_enum"
                index: 2
                type: "uint32_t"





types:
    // there would be a set of inbuilt pre configured types as per below. These would be configured in a separate "core" file and included as needed
    i8:
        type: Integer
        bitwidth: 8 
        signed: true
        root_type: false // This is a special setting that instructs that parser to only emit this into the final header files if its actually used. 
        namespace: "sub8"

    u64p32:
        type: VbrInteger
        segments: [32,32] 
        signed: false
        root_type: false
        namespace: "sub8"

    f32:
        type: Float
        exponent: 8
        fraction: 32
        root_type: false
        namespace: "sub8"


    optional_f32:
        type: Optional
        type: f32
        root_type: false
        namespace: "users::name::space" // or none

    MsgType:
        type: Enumeration
        values:
            Ping: 10,
            Pong: 11,
            Data: 12,
            Ack: 13,
            Nack: 14,
        root_type: true // this is the default value
        namespace: "users::name::space" // or none

    // And more types declared within this block
    // if more then one types block exist across files, then they are all merged


messages:
    some_dto_object:
        type: MsgType
        data: 
            type: Variant
            values:
                Ping:
                    yes: Boolean
                Pong: 
                    count: u8

```

this block will translate into 

Generated Header file
```cpp

    #include "sub8_primitives.h" 
    #include "sub8_enums.h" 

    // i8
    namespace sub8 {
        extern template struct sub8::Integer</* bitwidth */8, /* Signed */ true>;
        using i8 = sub8::Integer</* bitwidth */8, /* Signed */ true>;
    }
    
    // v64p32
    namespace sub8 {
        extern template struct sub8::VbrInteger</* signed */ false, 32, 32>;
        using v64p32 = sub8::VbrInteger</* signed */ false, 32, 32>;
    }

    // f32
    namespace sub8 {
        extern template struct sub8::FloatingPoint<8, 23>;
        using f32 = sub8::FloatingPoint<8, 23>;
    }

    // optional_f32
    namespace users::name::space {
        extern template struct sub8::OptionalT<sub8::non_alloc_optional, sub8::f32>
        using optional_f32 = sub8::OptionalT<sub8::non_alloc_optional, sub8::f32>;
    }

    namespace users::name::space {
        enum class MsgType : uint8_t {
            Ping = 10,
            Pong = 11,
            Data = 12,
            Ack  = 13,
            Nack = 14,
        };
    }

```

Generated CPP file
```cpp
    // i8
    template struct sub8::Integer</* bitwidth */8, /* Signed */ true>;

    // v64p32
    template struct sub8::VbrInteger</* signed */ false, 32, 32>;

    // f32
    template struct sub8::FloatingPoint<8, 23>;

    // optional_f32
    template struct sub8::OptionalT<sub8::non_alloc_optional, sub8::f32>

    enum class MsgType : uint8_t {
  Ping = 10,
  Pong = 11,
  Data = 12,
  Ack  = 13,
  Nack = 14,
};
```