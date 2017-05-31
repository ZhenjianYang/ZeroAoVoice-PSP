#pragma once

#include "sf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct InitPlayerParam {
	int* p_h_dududu_volume;
	int* p_h_dlgse_volume;
} InitPlayerParam;

bool InitPlayer(InitPlayerParam* initPlayerParam);

bool EndPlayer();

bool PlaySound(const char* filename, int volume, InitSfCall initSf);

bool StopSound();

#ifdef __cplusplus
}
#endif // __cplusplus

