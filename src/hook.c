#include "hook.h"
#include "hook_asm.h"
#include "player.h"
#include "za_voice_i.h"
#include "io.h"
#include "log.h"

#define CODE_NOP	0x00000000
#define CODE_JAL	0x0C000000
#define CODE_J		0x08000000
#define CODE_JMsk	0x03FFFFFF
#define CODE_CntMsk	0x0000FFFF

enum HookType {
	HookType_FixOP_List = 1 << 30,
	HookType_FixOP = 1 << 29,

	HookType_JAL = 1 << 0,
	HookType_J = 1 << 1,

	HookType_ClearDelay = 1 << 10
};

static const u32 HookAddrList_Zero[] = {
	0x088F4C54,
};

static const u32 HookAddrList_Ao[] = {
	0x0,
};

static const u32* HookAddrList;
static const u32 HookOperandList[] = {
	(u32)&h_voice,
};
static const u32 HookTyperList[] = {
	HookType_JAL,
};
static u32 _AddrAdjust;
#define HookCount (sizeof(HookTyperList) / sizeof(*HookTyperList))

#define STD_BASE	0x08804000

#define MAX_VOICEID_LEN	10
#define VOLUME_MAX 0x8000
#define VOICE_FILE_PREFIX "/v"
static char _buff_voicefile[64];
static int _len_prefix;

bool DoHook() {
	HookAddrList = g_game->game == Game_Zero ? HookAddrList_Zero : HookAddrList_Ao;
	_AddrAdjust = g_game->base - STD_BASE;
	for(unsigned i = 0; i < HookCount; i++) {
		if(!HookAddrList[i]) continue;

		u32* pDst = (u32*)(HookAddrList[i] + _AddrAdjust);

		if(HookTyperList[i] & HookType_FixOP_List) {
			unsigned count = CODE_CntMsk & HookTyperList[i];
			u32* pSrc = (u32*)HookOperandList[i];
			for(unsigned j = 0; j < count; j++) {
				pDst[j] = pSrc[j];
			}
		} else if (HookTyperList[i] & HookType_FixOP) {
			*pDst = HookOperandList[i];
		} else if (HookTyperList[i] & (HookType_JAL | HookType_J)) {
			unsigned new_code = HookTyperList[i] & HookType_JAL ? CODE_JAL : CODE_J;
			new_code |= CODE_JMsk & (HookOperandList[i] >> 2);
			*pDst = new_code;

			if(HookTyperList[i] & HookType_ClearDelay) {
				*(pDst + 1) = CODE_NOP;
			}
		}
	}

	for(_len_prefix = 0; g_game->path[_len_prefix]; _len_prefix++) {
		_buff_voicefile[_len_prefix] = g_game->path[_len_prefix];
	}
	for(int i = 0; g_game->ext[i]; i++, _len_prefix++) {
		_buff_voicefile[_len_prefix] = g_game->ext[i];
	}
	for(unsigned i = 0; i < sizeof(VOICE_FILE_PREFIX); i++) {
		_buff_voicefile[_len_prefix + i] = VOICE_FILE_PREFIX[i];
	}
	_len_prefix += sizeof(VOICE_FILE_PREFIX) - 1;

	LOG("addr voice file: 0x%08X", (unsigned)_buff_voicefile);
	LOG("len prefix: %d", _len_prefix);

	return true;
}
bool CleanHook() { return true; }


void H_voice(const char* p) {
	if(*p != 'v') return;

	const char* q = p - 1;
	while(q >= p - MAX_VOICEID_LEN - 1 && *q != '#') q--;
	if(*q != '#' || p - q <= 1) return;

	q++;
	char* t = _buff_voicefile + _len_prefix;
	while(q < p) *t++ = *q++;
	*t++ = '.';
	for(unsigned i = 0; i < sizeof(g_game->ext); i++) *t++ = g_game->ext[i];

	PlaySound(_buff_voicefile, VOLUME_MAX, g_game->initSfCall);
}
