#ifndef TYPES_H
#define TYPES_H

#define NULL 0
#define false 0
#define true 1

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int64_t intptr_t;
typedef uint64_t uintptr_t;

typedef uint32_t size_t;

typedef int64_t system_time_t;
typedef uint64_t usecond_t;
typedef uint32_t second_t;

#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64

typedef __builtin_va_list	va_list;

#define va_start(ap,last) 	__builtin_va_start((ap),(last))
#define va_arg		__builtin_va_arg
#define va_copy		__builtin_va_copy
#define va_end(ap)		__builtin_va_end(ap)

#define INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline)) 

#endif // TYPES_H