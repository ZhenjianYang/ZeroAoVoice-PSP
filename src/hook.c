#include "hook.h"
#include "global.h"
#include "hook2.h"
#include "log.h"

#define CODE_NOP	0x00000000
#define CODE_JAL	0x0C000000
#define CODE_J		0x08000000
#define CODE_JMsk	0x03FFFFFF
#define CODE_BMsk	0x0000FFFF
#define CODE_CntMsk	0x0000FFFF

#define OFF_PFM_CNT_ZERO 0x30D8
#define OFF_PFM_CNT_AO 0x3138

enum HookType {
	HookType_FixOP_List = 1 << 30,
	HookType_FixOP = 1 << 29,

	HookType_JAL = 1 << 0,
	HookType_J = 1 << 1,

	HookType_ClearDelay = 1 << 10,
	HookType_SaveOldJmpAddr = 1 << 11,
	HookType_SaveOldJmpB = 1 << 12
};

static u32 HookAddrList_Zero[] = {
	0x088F4C54, //voice instruction
	0x088F652C, //dududu
	0x088F6408, //dlgse
	0x0880A010, //scode
	0x088F4B3C, //count
	0x089DD550, //ctrl
	0x0883DAE0, //pfm_cnt
	0x088F4FB4, //codeA
	0, //dis_orivoice
	0x089E4118
};

static u32 HookAddrList_Ao[] = {
	0x088FC7DC, //voice instruction
	0x088FDA4C, //dududu
	0x088FD868, //dlgse
	0x0880A09C, //scode
	0x088FC300, //count
	0x08A02C68, //ctrl
	0x0883F14C, //pfm_cnt
	0x088FC640, //codeA
	0x088FC3E8, //dis_orivoice
	0,
};
#define IDX_HOOKADDR_DIS_ORIVOICE 8

static const u32 HookOperandList[] = {
	(u32)&h_voice,
	(u32)&h_dududu,
	(u32)&h_dlgse,
	(u32)&h_scode,
	(u32)&h_count,
	(u32)&h_ctrl,
	(u32)&h_pfm_cnt,
	(u32)&h_codeA,
	CODE_NOP,//dis_orivoice
	(u32)&h_test
};
static const u32 HookOperand2List[] = {
	0, //voice instruction
	0, //dududu
	(u32)&g.ha.addr_sub_se, //dlgse
	0, //scode
	(u32)&g.ha.addr_ge, //count
	0, //ctrl
	0, //pfm_cnt
	0, //codeA
	0,//dis_orivoice
	(u32)&g.ha.addr_test,
};
static const u32 HookTyperList[] = {
	HookType_JAL, //voice instruction
	HookType_JAL, //dududu
	HookType_JAL | HookType_SaveOldJmpAddr,//dlgse
	HookType_JAL | HookType_ClearDelay,//scode
	HookType_JAL | HookType_ClearDelay | HookType_SaveOldJmpB, //count
	HookType_JAL,//ctrl
	HookType_JAL,//pfm_cnt
	HookType_JAL,//codeA
	HookType_FixOP,//dis_orivoice
	HookType_JAL | HookType_SaveOldJmpAddr,//dlgse
};

#define HookCount (sizeof(HookTyperList) / sizeof(*HookTyperList))
#define STD_BASE	0x08804000

//////////////////////////////////////////////////////////////////////////////

bool DoHook() {
	u32 * hookAddrList = g.game == Game_Zero ? HookAddrList_Zero : HookAddrList_Ao;
	u32 addrAdjust = g.mod_base - STD_BASE;
	g.off_pfm_cnt = g.game == Game_Zero ? OFF_PFM_CNT_ZERO : OFF_PFM_CNT_AO;

	if(!g.config.DisableOriginalVoice) {
		HookAddrList_Ao[IDX_HOOKADDR_DIS_ORIVOICE] = 0;
	}

	for(unsigned i = 0; i < HookCount; i++) {
		if(!hookAddrList[i]) continue;

		LOG("Hook addr = 0x%08X, type = 0x%08X, Operand = 0x%08X",
					hookAddrList[i] + addrAdjust, HookTyperList[i], HookOperandList[i]);

		u32* pDst = (u32*)(hookAddrList[i] + addrAdjust);

		if(HookTyperList[i] & HookType_FixOP_List) {
			unsigned count = CODE_CntMsk & HookTyperList[i];
			u32* pSrc = (u32*)HookOperandList[i];
			for(unsigned j = 0; j < count; j++) {
				pDst[j] = pSrc[j];
			}
		} else if (HookTyperList[i] & HookType_FixOP) {
			*pDst = HookOperandList[i];
		} else if (HookTyperList[i] & (HookType_JAL | HookType_J)) {
			u32 new_code = HookTyperList[i] & HookType_JAL ? CODE_JAL : CODE_J;
			new_code |= CODE_JMsk & (HookOperandList[i] >> 2);

			if(HookTyperList[i] & HookType_SaveOldJmpAddr) {
				*(u32*)HookOperand2List[i] = (*pDst & CODE_JMsk) << 2;
			} else if(HookTyperList[i] & HookType_SaveOldJmpB) {
				u32 len = (u32)((s16)(*pDst & CODE_JMsk) * 4);
				*(u32*)HookOperand2List[i] = (u32)pDst + len + 4;
			}

			*pDst = new_code;

			if(HookTyperList[i] & HookType_ClearDelay) {
				*(pDst + 1) = CODE_NOP;
			}
		}
	}

	InitHook2();

	return true;
}
bool CleanHook() { return true; }


