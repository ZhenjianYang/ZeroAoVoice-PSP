#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define scode_TEXT		0x55
#define scode_SAY		0x5C
#define scode_TALK		0x5D
#define scode_MENU		0x5E
#define scode_MENUEND	0x5F

typedef struct AutoPlay {
	int scode;

	int text_end;
	int text_cnt;

	unsigned fm_start;
	unsigned fm_auto;

	unsigned fm_voice_start;
	unsigned fm_voice_auto;
} AutoPlay;


typedef struct Order {
	int disableDududu;
	int disableDlgSe;
} Order;

typedef struct HookAddr {
	unsigned addr_sub_se;
	unsigned addr_ge;
} HookAddr;


#ifdef __cplusplus
}
#endif // __cplusplus
