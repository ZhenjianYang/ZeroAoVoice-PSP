#pragma once

#include "sf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define EXT_AT3 "at3"

#define FORMAT_TAG_ATRAC3		0x0270
#define FORMAT_TAG_ATRAC3PLUS	0xFFFE

bool InitAt3(SoundFile* sf) __attribute__((weak));

#ifdef __cplusplus
}
#endif // __cplusplus
