#pragma once

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int bool32;
typedef char bool8;

// Ensure all types are of the correct size.
_Static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
_Static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
_Static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
_Static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

_Static_assert(sizeof(int8) == 1, "Expected i8 to be 1 byte.");
_Static_assert(sizeof(int16) == 2, "Expected i16 to be 2 bytes.");
_Static_assert(sizeof(int32) == 4, "Expected i32 to be 4 bytes.");
_Static_assert(sizeof(int64) == 8, "Expected i64 to be 8 bytes.");

_Static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
_Static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define FAILURE -1

#define NET_NAME_LEN        31
#define NET_NAME_LEN_PLUS  32
#define NET_TEXT_LEN        512
#define LOOP_SLEEP_TIME     100000

// Macro to disable warning if var unused
#define UNUSED(x) (void)(x)



// Converts a bool value to a string
static inline const char* bool_To_String(bool boolValue) { return boolValue ? " true" : "false"; }