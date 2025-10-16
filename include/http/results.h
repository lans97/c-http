#pragma once

#include <stdint.h>
typedef const char* ErrorMessage;

// Declaration
#define DECLARE_RESULT_TYPE(Type, Name)                     \
    typedef struct Name Name;                               \
    static inline Name Name##_Ok(Type v);                   \
    static inline Name Name##_Error(const char* m);

// Definition
#define DEFINE_RESULT_TYPE(Type, Name)                      \
struct Name {                                               \
    bool Ok;                                                \
    union {                                                 \
        Type Value;                                         \
        ErrorMessage Err;                                   \
    };                                                      \
};                                                          \
static_assert(sizeof(Type) > 0, "Type must be complete");   \
static inline Name Name##_Ok(Type v) {                      \
    return (Name){                                          \
        .Ok = true,                                         \
        .Value = v                                          \
    };                                                      \
}                                                           \
static inline Name Name##_Error(ErrorMessage m) {           \
    return (Name){                                          \
        .Ok = false,                                        \
        .Err = m                                            \
    };                                                      \
}

// Result type declarations
DECLARE_RESULT_TYPE(int, IntResult);
DECLARE_RESULT_TYPE(uint8_t, UInt8Result);
DECLARE_RESULT_TYPE(uint16_t, UInt16Result);
DECLARE_RESULT_TYPE(bool, BoolResult);
DECLARE_RESULT_TYPE(char*, StringResult);
DECLARE_RESULT_TYPE(const char*, ConstStringResult);
DECLARE_RESULT_TYPE(char**, StringArrResult);
DECLARE_RESULT_TYPE(const char**, ConstStringArrResult);

// C Result type definitions
// Since only C standard types' size are known only these are
// defined here
DEFINE_RESULT_TYPE(int, IntResult);
DEFINE_RESULT_TYPE(uint8_t, UInt8Result);
DEFINE_RESULT_TYPE(uint16_t, UInt16Result);
DEFINE_RESULT_TYPE(bool, BoolResult);
DEFINE_RESULT_TYPE(char*, StringResult);
DEFINE_RESULT_TYPE(const char*, ConstStringResult);
DEFINE_RESULT_TYPE(char**, StringArrResult);
DEFINE_RESULT_TYPE(const char**, ConstStringArrResult);

#define HttpOk(v) _Generic((v),             \
    int: IntResult_Ok,                      \
    bool: BoolResult_Ok,                    \
    char*: StringResult_Ok,                 \
    char**: StringArrResult_Ok,             \
    const char*: ConstStringResult_Ok,      \
    const char**: ConstStringArrResult_Ok   \
)(v)

#define HttpError(v) _Generic((v),              \
    int: IntResult_Error,                       \
    bool: BoolResult_Error,                     \
    char*: StringResult_Error,                  \
    char**: StringArrResult_Error,              \
    const char*: ConstStringResult_Error,       \
    const char**: ConstStringArrResult_Error    \
)(v)
