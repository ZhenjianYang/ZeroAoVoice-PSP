#pragma once

#include "sf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define EXT_OGG "ogg"
#define FORMAT_TAG_OGG	0x0099

bool InitOgg(SoundFile* sf) __attribute__((weak));

#ifdef __cplusplus
}
#endif // __cplusplus

