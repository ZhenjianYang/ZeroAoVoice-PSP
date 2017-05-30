#pragma once

#include "sf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool InitPlayer();

bool EndPlayer();

bool PlaySound(const char* filename, int volume, InitSfCall initSf);

bool StopSound();

#ifdef __cplusplus
}
#endif // __cplusplus

