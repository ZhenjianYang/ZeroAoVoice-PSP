#include "draw.h"

#include "basic_type.h"
#include "sceDmac.h"

#include "global.h"
#include "log.h"

#include <pspge.h>

#define CH_HEIGTH 16
#define CH_WIDTH 8

#define MAX_CH_ONE_INFO 16
#define INFO_WIDTH 128

#define MAX_UINT 0xFFFFFFFF
#define PSP_VRAM_WIDTH 512
#define VRAM 0x44000000

typedef u16 BitType;
#define BACK_COLOR 0x0000
#define FRONT_COLOR 0xFFFF

typedef struct DrawInfo {
	unsigned dead_time;
	BitType data[CH_HEIGTH][INFO_WIDTH];
} DrawInfo;

static DrawInfo _dis[InfoType_TotalCount];
#define CNT_DI (sizeof(_dis) / sizeof(*_dis))

int need_draw = 0;

#include "font.c"


typedef u8 (*PCHFONT)[CH_HEIGTH][CH_WIDTH];
static PCHFONT fonts = (PCHFONT)fonts_raw;


int Draw() {
	if(!g.pfm_cnt) return 0;

	unsigned cur_time = *g.pfm_cnt;

	int cnt = 0;
	BitType* p = (BitType*)VRAM;
	for(unsigned i = 0; i < CNT_DI; i++) {
		if(_dis[i].dead_time > cur_time) {
			cnt++;
			for(int j = 0;j < CH_HEIGTH; j++) {
				sceDmacMemcpy(p, _dis[i].data[j], sizeof(_dis[i].data[j]));
				p += PSP_VRAM_WIDTH;
			}
		}
	}
	return need_draw = cnt;
}

bool AddInfo(const Info *info) {
	LOG("Add info.\n"
		"    type = %d\n"
		"    time = %d\n"
		"    text = %s",
		info->type, info->time, info->text);
	if(!info || info->type >= CNT_DI || info->type < 0) return false;

	unsigned cur_time = g.pfm_cnt ? *g.pfm_cnt : 0;
	DrawInfo* di = _dis + info->type;
	di->dead_time = info->time == INFOTIME_INFINITY ? MAX_UINT : cur_time + info->time;

	LOG("Modify DrawInfo : \n"
		"    di = 0x%08X\n"
		"    dead_time = %d\n"
		"    data = 0x%08X",
		(u32)di, di->dead_time, (u32)di->data);

	for(unsigned i = 0; i < sizeof(di->data) / sizeof(BitType); i ++) {
		*((BitType*)di->data + i) = BACK_COLOR;
	}

	int x = 0;
	for(int i = 0; i < MAX_CH_ONE_INFO; i++, x += CH_WIDTH) {
		if(info->text[i] <= 0) break;
		for(int y = 0; y < CH_HEIGTH; y++) {
			for(int xx = 0; xx < CH_WIDTH; xx++) {
				u8 bit = fonts[(int)info->text[i]][y][xx];
				unsigned bit5 = (bit * 0x1F / 0xFF) & 0x1F;
				di->data[y][x + xx] = (BitType)(bit5 | (bit5 << 5) | (bit << 10) | (bit5 ? 1 << 15 : 0));
			}
		}
	}

	need_draw = 1;
	return true;
}

bool RemoveInfo(int type) {
	if(type >= CNT_DI || type < 0) return false;
	 _dis[type].dead_time = 0;

	return true;
}



