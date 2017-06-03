#pragma once

#include "basic_type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct Play {
	const char* filename;
	int volume;
	void* initSf;
} Play;

bool InitPlayer();

bool EndPlayer();

bool PlaySound(struct Play* play);

bool StopSound();

bool SetVolume(int volume);

#ifdef __cplusplus
}
#endif // __cplusplus

