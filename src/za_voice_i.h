#pragma once

#include "type.h"
#include "sf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum Game {
	Game_Invalid = 0,
	Game_Zero,
	Game_Ao
} Game;

typedef struct GameInfo {
	Game game;
	char umdId[12];
	char path[32];

	char ext[4];
	InitSfCall initSfCall;

	u32 base;
} GameInfo;

extern const GameInfo * const g_game;

#ifdef __cplusplus
}
#endif // __cplusplus
