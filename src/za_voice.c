#include <pspmodulemgr.h>
#include <pspmodulemgr.h>

#include "basic_type.h"
#include "za_voice.h"
#include "io.h"
#include "sf_wav.h"
#include "sf_ogg.h"
#include "sf_at3.h"
#include "player.h"
#include "hook.h"
#include "config.h"
#include "global.h"
#include "message.h"
#include "draw.h"
#include "log.h"

Global g;

#define UMDID_ZERO "NPJH-50311"
#define PATH_BASE_ZERO "ms0:/PSP/za_voice/zero/"
#define PATH_VP_ZERO "ms0:/PSP/za_voice/zero/voice.pak"

#define UMDID_AO "NPJH-50473"
#define PATH_BASE_AO "ms0:/PSP/za_voice/ao/"
#define PATH_VP_AO "ms0:/PSP/za_voice/ao/voice.pak"

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

static inline int Strcmp(const char* s1, const char *s2) {
	while(*s1 == *s2 && *s1) s1++, s2++;
	return (unsigned char)*s1 - (unsigned char)*s2;
}

static bool initGame(SceUID mid_boot) {
	g.game = Game_Unknown;

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
	if(!Strcmp(buff_umdid, UMDID_ZERO)) {
		g.game = Game_Zero;
		for(unsigned i = 0; i < sizeof(UMDID_ZERO); i++) g.umdId[i] = UMDID_ZERO[i];
		for(unsigned i = 0; i < sizeof(PATH_BASE_ZERO); i++) g.path[i] = PATH_BASE_ZERO[i];
		len_path = sizeof(PATH_BASE_ZERO) - 1;
	} else if(!Strcmp(buff_umdid, UMDID_AO)) {
		g.game = Game_Ao;
		for(unsigned i = 0; i < sizeof(UMDID_AO); i++) g.umdId[i] = UMDID_AO[i];
		for(unsigned i = 0; i < sizeof(PATH_BASE_AO); i++) g.path[i] = PATH_BASE_AO[i];
		len_path = sizeof(PATH_BASE_AO) - 1;
	}
	if(g.game == Game_Unknown) {
		LOG("No matched UMDID found.");
		return false;
	}

	SceKernelModuleInfo info;
	info.size = sizeof(info);
	if (sceKernelQueryModuleInfo(mid_boot, &info)) {
		LOG("Got module info failed.");
		return false;
	}
	g.mod_base = info.segmentaddr[0];

	g.voice_ext[0] = '\0';
	LOG("finding Voice pack ...");
	const char* path_vpk = g.game == Game_Zero ? PATH_VP_ZERO : PATH_VP_AO;
	if(VP_Init(&g.vp, path_vpk)) {
		LOG("Voice pack found:\n"
			"    count = %d\n"
			"    ext = %s",
			g.vp.count, g.vp.ext);

		for(unsigned i = 0; i < sizeof(_sfs) / sizeof(*_sfs); i++) {
			if(!Strcmp(_sfs[i].ext, g.vp.ext)) {
				if(_sfs[i].initSfCall) {
					for(unsigned j = 0; j < sizeof(g.voice_ext); j++) {
						g.voice_ext[j] = _sfs[i].ext[j];
					}
					g.initSf = (void*)_sfs[i].initSfCall;
				} else {
					LOG("Format %s in voice pack not supported. Skip", g.voice_ext);
					VP_Finish(&g.vp);
				}
			}
		}
	}

	if(!g.voice_ext[0]) {
		LOG("finding Voice dir ...");
		for(unsigned i = 0; i < sizeof(_sfs) / sizeof(*_sfs); i++) {
			LOG("Checking %s ...", _sfs[i].ext);
			if(!_sfs[i].initSfCall) {
				LOG("Not supported. skip");
				continue;
			}

			for(unsigned j = 0; j < sizeof(g.voice_ext); j++) g.path[len_path + j] = _sfs[i].ext[j];
			LOG("Checking dir %s ...", g.path);
			if(IoDirExists(g.path)) {
				g.initSf = (void*)_sfs[i].initSfCall;
				for(unsigned k = 0; k < sizeof(g.voice_ext); k++) g.voice_ext[k] = _sfs[i].ext[k];
				g.path[len_path] = '\0';
				break;
			}
		}
	}
	if (!g.voice_ext[0]) {
		LOG("Voice pack and voice dir not found.");
		return false;
	}

	return true;
};


int InitZaVoice(unsigned args, void *argp)
{
	SceUID mid_boot = args >= 4 ? *(SceUID*)(argp) : -1;
	if (mid_boot < 0 || !initGame(mid_boot)) return 0;
	LOG("Init Game infomation Finished.");

	LoadConfig(&g.config, PATH_CONFIG);
	LOG("Config info:\n"
		"    Volume                   = %d\n"
		"    AutoPlay                 = %d\n"
		"    WaitTimePerChar          = %d\n"
		"    WaitTimeDialog           = %d\n"
		"    WaitTimeDialogWithVoice  = %d\n"
		"    SkipVoice                = %d\n"
		"    DisableDialogTextSE      = %d\n"
		"    DisableDialogSwitchSE    = %d\n"
		"    ShowInfo                 = %d\n"
		"    PPSSPP                   = %d",
		g.config.Volume,
		g.config.AutoPlay,
		g.config.WaitTimePerChar,
		g.config.WaitTimeDialog,
		g.config.WaitTimeDialogWithVoice,
		g.config.SkipVoice,
		g.config.DisableDialogTextSE,
		g.config.DisableDialogSwitchSE,
		g.config.ShowInfo,
		g.config.PPSSPP
	);
	SaveConfig(&g.config, PATH_CONFIG);

	if (!DoHook()) return 0;
	LOG("DoHook Finished.");

	if (!InitPlayer()) return 0;
	LOG("InitPlayer Finished.");

	LOG("Game info:\n"
		"    game = %d\n"
		"    umdId = %s\n"
		"    path = %s\n"
		"    ext = %s\n"
		"    initSfCall = 0x%08X",
		g.game, g.umdId,
		g.path, g.voice_ext,
		(unsigned)g.initSf
	);

	LOG("\n    g = 0x%08X\n"
		"    mod_base = 0x%08X\n"
		"    off_pfm_cnt = 0x%08X\n"
		"    pfm_cnt = 0x%08X",
		(u32)&g, g.mod_base, g.off_pfm_cnt, (u32)g.pfm_cnt
	);

	InitDraw();
	LOG("All init Done.");

	Info info = {
			Msg_Time_Hello,
			Msg_Hello
	};
	AddInfo(&info);
	return 0;
}

int EndZaVoice(unsigned args, void *argp) {
	CleanHook();
	EndPlayer();
	EndDraw();
	VP_Finish(&g.vp);
	return 0;
}
