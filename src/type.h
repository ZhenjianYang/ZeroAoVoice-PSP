#pragma once

#include <stdint.h>

#ifndef PSP_LEGACY_TYPES_DEFINED
#define PSP_LEGACY_TYPES_DEFINED

typedef	uint8_t				u8;
typedef uint16_t			u16;

typedef uint32_t			u32;
typedef uint64_t			u64;

typedef int8_t				s8;
typedef int16_t				s16;

typedef int32_t				s32;
typedef int64_t				s64;

#endif

typedef unsigned int size_t;

typedef void* Handle;
#define ParseHandle(h) ((Handle)(~(unsigned)(h)))

#ifndef __cplusplus
typedef enum bool {
	false = 0,
	true = 1
} bool;
#endif // __cplusplus

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL (void*)0
#endif // __cplusplus
#endif // !NULL

