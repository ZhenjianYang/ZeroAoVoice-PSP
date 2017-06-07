#pragma once

#include "hook_data.h"
#include "config.h"
#include "voice_pack.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define FPS 30
#define TIME_UNITS_PER_SEC 1000
#define CTRL_OK 0x4000

enum {
	Game_Unknown = 0,
	Game_Zero = 1,
	Game_Ao = 2,
};

typedef struct Global {
	int game;
	char umdId[12];
	char path[32];

	char voice_ext[4];
	void* initSf;

	unsigned mod_base;
	unsigned off_pfm_cnt;
	unsigned* pfm_cnt;

	Order order;
	AutoPlay autoPlay;
	HookAddr ha;

	Config config;
	VoicePack vp;
} Global;

extern Global g;

#ifdef __cplusplus
}
#endif // __cplusplus
