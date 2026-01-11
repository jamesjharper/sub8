#pragma once

// ---- basic concatenation / expansion ----
#define SUB8_PP_CAT(a, b) SUB8_PP_CAT_I(a, b)
#define SUB8_PP_CAT_I(a, b) a##b
#define SUB8_PP_EXPAND(x) x

// ---- wrap types that contain commas ----
// Usage: (SUB8_T(NestedBitField<ItemExample, 5>), list)
#define SUB8_T(...) __VA_ARGS__

// ---- count args (up to 16) ----
#define SUB8_PP_NARG(...) SUB8_PP_NARG_I(__VA_ARGS__, SUB8_PP_RSEQ_N())
#define SUB8_PP_NARG_I(...) SUB8_PP_ARG_N(__VA_ARGS__)
#define SUB8_PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define SUB8_PP_RSEQ_N() 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// ---- FOR_EACH for "pair" args (up to 16) ----
// Each arg must be: (Type, name)
// IMPORTANT: do NOT put a trailing comma after the last pair.
#define SUB8_PP_FOR_EACH_PAIR(macro, ...) \
  SUB8_PP_EXPAND(SUB8_PP_CAT(SUB8_PP_FOR_EACH_PAIR_, SUB8_PP_NARG(__VA_ARGS__))(macro, __VA_ARGS__))

#define SUB8_PP_FOR_EACH_PAIR_1(m, a1) m(a1)
#define SUB8_PP_FOR_EACH_PAIR_2(m, a1, a2) m(a1) m(a2)
#define SUB8_PP_FOR_EACH_PAIR_3(m, a1, a2, a3) m(a1) m(a2) m(a3)
#define SUB8_PP_FOR_EACH_PAIR_4(m, a1, a2, a3, a4) m(a1) m(a2) m(a3) m(a4)
#define SUB8_PP_FOR_EACH_PAIR_5(m, a1, a2, a3, a4, a5) m(a1) m(a2) m(a3) m(a4) m(a5)
#define SUB8_PP_FOR_EACH_PAIR_6(m, a1, a2, a3, a4, a5, a6) m(a1) m(a2) m(a3) m(a4) m(a5) m(a6)
#define SUB8_PP_FOR_EACH_PAIR_7(m, a1, a2, a3, a4, a5, a6, a7) m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7)
#define SUB8_PP_FOR_EACH_PAIR_8(m, a1, a2, a3, a4, a5, a6, a7, a8) m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8)
#define SUB8_PP_FOR_EACH_PAIR_9(m, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9)
#define SUB8_PP_FOR_EACH_PAIR_10(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10)
#define SUB8_PP_FOR_EACH_PAIR_11(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10) m(a11)
#define SUB8_PP_FOR_EACH_PAIR_12(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10) m(a11) m(a12)
#define SUB8_PP_FOR_EACH_PAIR_13(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10) m(a11) m(a12) m(a13)
#define SUB8_PP_FOR_EACH_PAIR_14(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10) m(a11) m(a12) m(a13) m(a14)
#define SUB8_PP_FOR_EACH_PAIR_15(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10) m(a11) m(a12) m(a13) m(a14) m(a15)
#define SUB8_PP_FOR_EACH_PAIR_16(m, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) \
  m(a1) m(a2) m(a3) m(a4) m(a5) m(a6) m(a7) m(a8) m(a9) m(a10) m(a11) m(a12) m(a13) m(a14) m(a15) m(a16)

// ============================================================
// Per-field generators
// ============================================================

// These take TWO args: (type, name)
#define SUB8_DTO_DECL_FIELD_TN(type, name) type name{};

#define SUB8_DTO_WRITE_FIELD_TN(type, name) \
  do { \
    r = write_field(bw, v.name); \
    if (r != sub8::BitFieldResult::Ok) \
      return r; \
  } while (0);

#define SUB8_DTO_READ_FIELD_TN(type, name) \
  do { \
    r = read_field(br, out.name); \
    if (r != sub8::BitFieldResult::Ok) \
      return r; \
  } while (0);

#define SUB8_DTO_EQ_FIELD_TN(type, name) \
  if (name != o.name) \
    return false;

// These take ONE arg: pair == (type, name)
// Trick: invoke the _TN macro by “appending” the pair.
#define SUB8_DTO_DECL_FIELD(pair) SUB8_DTO_DECL_FIELD_TN pair
#define SUB8_DTO_WRITE_FIELD(pair) SUB8_DTO_WRITE_FIELD_TN pair
#define SUB8_DTO_READ_FIELD(pair) SUB8_DTO_READ_FIELD_TN pair
#define SUB8_DTO_EQ_FIELD(pair) SUB8_DTO_EQ_FIELD_TN pair

// ============================================================
// DTO generator
// ============================================================

#define SUB8_DECLARE_DTO(NAME, ...) \
  struct NAME { \
    using Type = NAME; \
    using InitType = NAME; \
    using ValueType = NAME; \
\
    const NAME &value() const noexcept { return *this; } \
    sub8::BitFieldResult set_value(const NAME &v) noexcept { \
      *this = v; \
      return sub8::BitFieldResult::Ok; \
    } \
\
    SUB8_PP_FOR_EACH_PAIR(SUB8_DTO_DECL_FIELD, __VA_ARGS__) \
\
    bool operator==(const NAME &o) const noexcept { \
      SUB8_PP_FOR_EACH_PAIR(SUB8_DTO_EQ_FIELD, __VA_ARGS__) \
      return true; \
    } \
    bool operator!=(const NAME &o) const noexcept { return !(*this == o); } \
  }; \
\
  template<typename Storage> \
  inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage> &bw, const NAME &v) { \
    sub8::BitFieldResult r = sub8::BitFieldResult::Ok; \
    SUB8_PP_FOR_EACH_PAIR(SUB8_DTO_WRITE_FIELD, __VA_ARGS__) \
    return sub8::BitFieldResult::Ok; \
  } \
\
  template<typename Storage> inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage> &br, NAME &out) { \
    sub8::BitFieldResult r = sub8::BitFieldResult::Ok; \
    SUB8_PP_FOR_EACH_PAIR(SUB8_DTO_READ_FIELD, __VA_ARGS__) \
    return sub8::BitFieldResult::Ok; \
  }

// ===============================
// Variant generator
// ===============================

#define SUB8__VAR_ENUM_ITEM(Member, Code, GetterSetterType, FieldType) Member = Code, \


#define SUB8__VAR_UNION_MEMBER(Member, Code, GetterSetterType, FieldType) FieldType Member; \


#define SUB8__VAR_CODE_ITEM(Member, Code, GetterSetterType, FieldType) Code, \


#define SUB8__VAR_EQ_CASE(Member, Code, GetterSetterType, FieldType) \
  case Code: return variant.Member == o.variant.Member; \


// write/read of the *already-constructed* member
#define SUB8__VAR_WRITE_CASE(Member, Code, GetterSetterType, FieldType) \
  case Code: return write_field(bw, field.variant.Member); \


// NOTE: read now needs to emplace before reading; this macro is used by the updated read_field below.
#define SUB8__VAR_READ_CASE_EMPLACE(Member, Code, GetterSetterType, FieldType) \
  case Code: {                                                                \
    out.destroy_active();                                                     \
    new (&out.variant.Member) FieldType();                                    \
    return read_field(br, out.variant.Member);                                \
  } \


#define SUB8__VAR_DESTROY_CASE(Member, Code, GetterSetterType, FieldType) \
  case Code: variant.Member.~FieldType(); break; \


#define SUB8__VAR_COPY_CTOR_CASE(Member, Code, GetterSetterType, FieldType) \
  case Code: new (&variant.Member) FieldType(o.variant.Member); break; \


#define SUB8__VAR_MOVE_CTOR_CASE(Member, Code, GetterSetterType, FieldType) \
  case Code: new (&variant.Member) FieldType(std::move(o.variant.Member)); break; \


// Getter/setter: setter must destroy current active member, emplace target member, then set_value.
#define SUB8__VAR_GETTER_SETTER(Member, Code, GetterSetterType, FieldType)                     \
  inline const GetterSetterType* get_##Member() const noexcept {                                        \
    if (static_cast<uint32_t>(type) == static_cast<uint32_t>(Code))                            \
      return &variant.Member.value();                                                          \
    return nullptr;                                                                            \
  }                                                                                            \
  inline sub8::BitFieldResult set_##Member(GetterSetterType v) noexcept {                               \
    destroy_active();                                                                          \
    new (&variant.Member) FieldType();                                                         \
    auto r = variant.Member.set_value(v);                                                      \
    if (r == sub8::BitFieldResult::Ok) {                                                       \
      type = enum_from_id(static_cast<uint32_t>(Code));                                        \
      return sub8::BitFieldResult::Ok;                                                         \
    }                                                                                          \
    /* revert to null on failure */                                                            \
    variant.Member.~FieldType();                                                               \
    construct_null();                                                                          \
    return r;                                                                                  \
  }                                                                                            \
  inline bool is_##Member() const noexcept {                                                   \
    return static_cast<uint32_t>(type) == static_cast<uint32_t>(Code);                         \
  }                                                                                            \
  static Type make_##Member(GetterSetterType in) noexcept {                                    \
    auto v = make_null();                                                                      \
    (void)v.set_##Member(in);                                                                  \
    return v;                                                                                  \
  } \



// ===============================
// Main macro
// ===============================

#define SUB8_DECLARE_VARIANT_FIELD(Name, CASES)                                               \
  enum class Name##Type : uint32_t {                                                             \
    NullValueField = 0,                                                                        \
    CASES(SUB8__VAR_ENUM_ITEM)                                                                 \
  };                                                                                           \
                                                                                               \
  /* Compute min/max codes at compile time from the CASES list */                              \
  struct Name##__VariantTagMeta {                                                              \
    inline static constexpr uint32_t kCodes[] = { 0u, CASES(SUB8__VAR_CODE_ITEM) };             \
    inline static constexpr size_t   kCount   = sizeof(kCodes) / sizeof(kCodes[0]);            \
                                                                                               \
    inline static constexpr uint32_t MinCodeU32 = []() constexpr noexcept {                    \
      uint32_t m = kCodes[0];                                                                  \
      for (size_t i = 1; i < kCount; ++i) {                                                    \
        if (kCodes[i] < m) m = kCodes[i];                                                      \
      }                                                                                        \
      return m;                                                                                \
    }();                                                                                       \
                                                                                               \
    inline static constexpr uint32_t MaxCodeU32 = []() constexpr noexcept {                    \
      uint32_t m = kCodes[0];                                                                  \
      for (size_t i = 1; i < kCount; ++i) {                                                    \
        if (kCodes[i] > m) m = kCodes[i];                                                      \
      }                                                                                        \
      return m;                                                                                \
    }();                                                                                       \
                                                                                               \
    inline static constexpr Name##Type MinEnum = static_cast<Name##Type>(MinCodeU32);              \
    inline static constexpr Name##Type MaxEnum = static_cast<Name##Type>(MaxCodeU32);              \
  };                                                                                           \
                                                                                               \
  using Name##TypeFieldBase =                                                                  \
    sub8::EnumBitField<Name##Type, Name##__VariantTagMeta::MinEnum, Name##__VariantTagMeta::MaxEnum>; \
  struct Name##TypeField : Name##TypeFieldBase {                                               \
    using Name##TypeFieldBase::Name##TypeFieldBase;                                            \
    Name##TypeField() noexcept = default;                                                      \
  };                                                                                           \
                                                                                               \
  struct Name {                                                                                \
    Name##Type type{Name##Type::NullValueField};                                                   \
                                                                                               \
    union VariantValue {                                                                       \
      sub8::NullValueField null_v;                                                             \
      CASES(SUB8__VAR_UNION_MEMBER)                                                            \
      VariantValue() {}                                                                        \
      ~VariantValue() {}                                                                       \
    } variant;                                                                                 \
                                                                                               \
    using Type = Name;                                                                         \
    using InitType = Name;                                                                     \
    using ValueType = Name;                                                                    \
                                                                                               \
    /* ---- lifetime helpers ---- */                                                           \
    void construct_null() noexcept {                                                           \
      new (&variant.null_v) sub8::NullValueField{};                                             \
      type = Name##Type::NullValueField;                                                         \
    }                                                                                          \
                                                                                               \
    void destroy_active() noexcept {                                                           \
      switch (static_cast<uint32_t>(type)) {                                                   \
        case 0u: variant.null_v.~NullValueField(); break;                                      \
        CASES(SUB8__VAR_DESTROY_CASE)                                                          \
        default: break;                                                                        \
      }                                                                                        \
    }                                                                                          \
                                                                                               \
    void copy_construct_from(const Name& o) {                                                   \
      type = o.type;                                                                           \
      switch (static_cast<uint32_t>(type)) {                                                   \
        case 0u: new (&variant.null_v) sub8::NullValueField(o.variant.null_v); break;          \
        CASES(SUB8__VAR_COPY_CTOR_CASE)                                                        \
        default: new (&variant.null_v) sub8::NullValueField{}; type = Name##Type::NullValueField; break; \
      }                                                                                        \
    }                                                                                          \
                                                                                               \
    void move_construct_from(Name&& o) noexcept {                                               \
      type = o.type;                                                                           \
      switch (static_cast<uint32_t>(type)) {                                                   \
        case 0u: new (&variant.null_v) sub8::NullValueField(std::move(o.variant.null_v)); break; \
        CASES(SUB8__VAR_MOVE_CTOR_CASE)                                                        \
        default: new (&variant.null_v) sub8::NullValueField{}; type = Name##Type::NullValueField; break; \
      }                                                                                        \
    }                                                                                          \
                                                                                               \
    /* ---- special members ---- */                                                            \
    Name() noexcept { construct_null(); }                                                      \
    Name(const Name& o) { copy_construct_from(o); }                                            \
    Name(Name&& o) noexcept { move_construct_from(std::move(o)); }                             \
    ~Name() { destroy_active(); }                                                              \
                                                                                               \
    Name& operator=(const Name& o) {                                                           \
      if (this == &o) return *this;                                                            \
      destroy_active();                                                                        \
      copy_construct_from(o);                                                                  \
      return *this;                                                                            \
    }                                                                                          \
                                                                                               \
    Name& operator=(Name&& o) noexcept {                                                       \
      if (this == &o) return *this;                                                            \
      destroy_active();                                                                        \
      move_construct_from(std::move(o));                                                       \
      return *this;                                                                            \
    }                                                                                          \
                                                                                               \
    const Name& value() const noexcept { return *this; }                                       \
                                                                                               \
    bool operator==(const Name& o) const noexcept {                                            \
      if (type != o.type) return false;                                                        \
      switch (static_cast<uint32_t>(type)) {                                                   \
        case 0u: return true;                                                                  \
        CASES(SUB8__VAR_EQ_CASE)                                                               \
        default: return false;                                                                 \
      }                                                                                        \
    }                                                                                          \
    bool operator!=(const Name& o) const noexcept { return !(*this == o); }                    \
                                                                                               \
    sub8::BitFieldResult set_value(const Name& v) noexcept {                                   \
      *this = v;                                                                               \
      return sub8::BitFieldResult::Ok;                                                         \
    }                                                                                          \
                                                                                               \
    bool is_null() const noexcept { return static_cast<uint32_t>(type) == 0u; }                \
                                                                                               \
    CASES(SUB8__VAR_GETTER_SETTER)                                                             \
                                                                                               \
    static Name make_null() noexcept { return Name(); }                                        \
                                                                                               \
   private:                                                                                    \
    static constexpr Name##Type enum_from_id(uint32_t c) noexcept { return static_cast<Name##Type>(c); } \
  };                                                                                           \
                                                                                               \
  template<typename Storage>                                                                   \
  inline sub8::BitFieldResult write_field(sub8::BasicBitWriter<Storage>& bw, const Name& field) { \
    Name##TypeField tf;                                                                        \
    tf.set_value(field.type);                                                                  \
    auto r = write_field(bw, tf);                                                              \
    if (r != sub8::BitFieldResult::Ok) return r;                                               \
    switch (static_cast<uint32_t>(field.type)) {                                               \
      case 0u: return sub8::BitFieldResult::Ok;                                                \
      CASES(SUB8__VAR_WRITE_CASE)                                                              \
      default: return sub8::BitFieldResult::WarningNotSupportedConfiguration;                  \
    }                                                                                          \
  }                                                                                            \
                                                                                               \
template<typename Storage>                                                                   \
inline sub8::BitFieldResult read_field(sub8::BasicBitReader<Storage>& br, Name& out) {       \
  Name##TypeField tf;                                                                        \
  auto r = read_field(br, tf);                                                               \
  if (r != sub8::BitFieldResult::Ok) return r;                                               \
                                                                                             \
  Name##Type new_type = tf.value();                                                            \
  out.type = new_type;                                                                       \
                                                                                             \
  switch (static_cast<uint32_t>(new_type)) {                                                 \
    case 0u: {                                                                               \
      out.destroy_active();                                                                  \
      out.construct_null();                                                                  \
      return sub8::BitFieldResult::Ok;                                                       \
    }                                                                                        \
    CASES(SUB8__VAR_READ_CASE_EMPLACE)                                                       \
    default: return sub8::BitFieldResult::WarningNotSupportedConfiguration;                  \
  }                                                                                          \
} \

