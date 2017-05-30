#include <pspmodulemgr.h>
#include <pspmodulemgr.h>

#include "za_voice.h"
#include "za_voice_i.h"
#include "type.h"
#include "io.h"
#include "sf_wav.h"
#include "sf_ogg.h"
#include "sf_at3.h"
#include "player.h"
#include "hook.h"
#include "log.h"

static GameInfo _game;
const GameInfo * const g_game = &_game;

#define UMDID_ZERO "NPJH-50311"
#define PATH_BASE_ZERO "ms0:/PSP/za_voice/zero/"

#define UMDID_AO "NPJH-50473"
#define PATH_BASE_AO "ms0:/PSP/za_voice/ao/"

static const struct {
	const char ext[4];
	InitSfCall initSfCall;
} _sfs[] = {
	{EXT_AT3, &InitAt3},
	{EXT_OGG, &InitOgg},
	{EXT_WAV, &InitWAV}
};

#define PATH_UMD_DATA "disc0:/UMD_DATA.BIN"
#define LEN_UMDID 10

static inline int StrCmp(const char* s1, const char *s2) {
	while(*s1 == *s2 && *s1) s1++, s2++;
	return (unsigned char)*s1 - (unsigned char)*s2;
}

static bool initGame(SceUID mid_boot) {
	_game.game = Game_Invalid;

	LOG("Getting UMD ID...");
	IoHandle ioh = IoFOpen(PATH_UMD_DATA, IO_O_RDONLY);
	if(!ioh) {
		LOG("Open %s failed.", PATH_UMD_DATA);
		return false;
	}
	char buff_umdid[LEN_UMDID + 1];
	int res = IoFRead(buff_umdid, LEN_UMDID, 1, ioh);
	IoFClose(ioh);
	if(!res) {
		LOG("Read UMD ID Failed");
		return false;
	}

	buff_umdid[LEN_UMDID] = '\0';
	LOG("UMD ID : %s", buff_umdid);

	unsigned len_path = 0;
	if(!StrCmp(buff_umdid, UMDID_ZERO)) {
		_game.game = Game_Zero;
		for(unsigned i = 0; i < sizeof(UMDID_ZERO); i++) _game.umdId[i] = UMDID_ZERO[i];
		for(unsigned i = 0; i < sizeof(PATH_BASE_ZERO); i++) _game.path[i] = PATH_BASE_ZERO[i];
		len_path = sizeof(PATH_BASE_ZERO) - 1;
	} else if(!StrCmp(buff_umdid, UMDID_AO)) {
		_game.game = Game_Ao;
		for(unsigned i = 0; i < sizeof(UMDID_AO); i++) _game.umdId[i] = UMDID_AO[i];
		for(unsigned i = 0; i < sizeof(PATH_BASE_AO); i++) _game.path[i] = PATH_BASE_AO[i];
		len_path = sizeof(PATH_BASE_AO) - 1;
	}
	if(_game.game == Game_Invalid) {
		LOG("No matched UMDID found.");
		return false;
	}

	SceKernelModuleInfo info;
	info.size = sizeof(info);
	if (sceKernelQueryModuleInfo(mid_boot, &info)) {
		LOG("Got module info failed.");
		return false;
	}
	_game.base = info.segmentaddr[0];

	LOG("finding Voice dir ...");
	_game.ext[0] = '\0';
	for(unsigned i = 0; i < sizeof(_sfs) / sizeof(*_sfs); i++) {
		LOG("Checking %s ...", _sfs[i].ext);
		if(!_sfs[i].initSfCall) {
			LOG("Not supported. skip");
			continue;
		}

		for(unsigned j = 0; j < sizeof(_game.ext); j++) _game.path[len_path + j] = _sfs[i].ext[j];
		LOG("Checking dir %s ...", _game.path);
		if(IoDirExists(_game.path)) {
			_game.initSfCall = _sfs[i].initSfCall;
			for(unsigned k = 0; k < sizeof(_game.ext); k++) _game.ext[k] = _sfs[i].ext[k];
			_game.path[len_path] = '\0';
			break;
		}
	}
	if (!_game.ext[0]) {
		LOG("Voice dir not found.");
		return false;
	}

	LOG("Game info:\n"
		"    game = %d\n"
		"    umdId = %s\n"
		"    path = %s\n"
		"    ext = %s\n"
		"    initSfCall = 0x%08X\n"
		"    base = 0x%08X",
		_game.game, _game.umdId,
		_game.path, _game.ext,
		(unsigned)_game.initSfCall,
		_game.base
	);

	return true;
};


int InitZaVoice(unsigned args, void *argp)
{
	SceUID mid_boot = args >= 4 ? *(SceUID*)(argp) : -1;
	if (mid_boot < 0 || !initGame(mid_boot)) return 0;
	LOG("Init Game infomation Finished.");

	if (!DoHook()) return 0;
	LOG("DoHook Finished.");

	if (!InitPlayer()) return 0;
	LOG("InitPlayer Finished.");

	LOG("All init Done.");
	return 0;
}

int EndZaVoice(unsigned args, void *argp) {
	CleanHook();
	EndPlayer();
	return 0;
}
