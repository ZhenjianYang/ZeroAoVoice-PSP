#pragma once

#include "sf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define EXT_WAV "wav"

#define FORMAT_TAG_PCM		1

bool InitWAV(SoundFile* sf) __attribute__((weak));

#ifdef __cplusplus
}
#endif // __cplusplus
