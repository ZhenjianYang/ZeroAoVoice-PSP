#include "draw.h"
#include "font.h"

#include "basic_type.h"
#include "sceDmac.h"

#include "global.h"
#include "log.h"

#include <pspge.h>

#define CH_HEIGTH 16
#define MARGIN 1
#define MAX_INFO_WIDTH 256

#define MAX_UINT 0xFFFFFFFF
#define PSP_VRAM_WIDTH 512
#define PSP_VRAM_HEIGHT 272
#define VRAM 0x44000000

typedef u16 BitType;
#define BACK_COLOR 0x0000
#define FRONT_COLOR 0xFFFF

typedef struct DrawInfo {
	unsigned dead_time;
	unsigned width;
	BitType data[CH_HEIGTH][MAX_INFO_WIDTH];
} DrawInfo;

static DrawInfo _draw;

int need_draw = 0;

typedef u16 OffType;
static const u8* fnt_data;
static const OffType *off_list;
static int fnt_cnt;

bool InitDraw() {
	fnt_data = (const u8*)&font_dat_start;
	off_list = (const OffType*)fnt_data;
	fnt_cnt = *off_list / sizeof(OffType);

	LOG("Draw init: \n"
		"    off_list = 0x%08X\n"
		"    fnt_cnt = %d\n"
		"    fnt_data = 0x%08X",
		(unsigned)off_list, fnt_cnt, (unsigned)fnt_data);

	return true;
}

int Draw() {
	if(!g.pfm_cnt) return 0;
	unsigned cur_time = *g.pfm_cnt;

	if(_draw.dead_time < cur_time) return need_draw = 0;

	BitType* p = (BitType*)VRAM;
	BitType* q = p +  PSP_VRAM_WIDTH * PSP_VRAM_HEIGHT;

	for(int j = 0;j < CH_HEIGTH; j++) {
		sceDmacMemcpy(p, _draw.data[j], _draw.width * sizeof(**_draw.data));
		sceDmacMemcpy(q, _draw.data[j], _draw.width * sizeof(**_draw.data));
		p += PSP_VRAM_WIDTH;
		q += PSP_VRAM_WIDTH;
	}
	return need_draw = 1;
}

bool AddInfo(const Info *info) {
	LOG("Add info.\n"
		"    time = %d\n"
		"    text = %s",
		info->time, info->text);
	if(!info) return false;

	unsigned cur_time = g.pfm_cnt ? *g.pfm_cnt : 0;
	_draw.dead_time = info->time == INFOTIME_INFINITY ? MAX_UINT : cur_time + info->time;

	LOG("Modify DrawInfo : \n"
		"    draw = 0x%08X\n"
		"    dead_time = %d\n"
		"    data = 0x%08X",
		(u32)&_draw, _draw.dead_time, (u32)_draw.data);

	_draw.width = 0;
	for(int i = 0; info->text[i] > 0; i++) {
		if(info->text[i] >= fnt_cnt - 1) continue;

		int off_ch = off_list[(int)info->text[i]];
		int off_ch_next = off_list[(int)info->text[i] + 1];
		const u8* pch = fnt_data + off_ch;
		int width = (off_ch_next - off_ch) / CH_HEIGTH;

		if(_draw.width + width + MARGIN > MAX_INFO_WIDTH) break;

		_draw.width += MARGIN;
		for(int y = 0; y < CH_HEIGTH; y++) {
			for(int xx = 0; xx < width; xx++) {
				u8 bit = *pch++;
				unsigned bit5 = (unsigned)bit >> 3;
				unsigned bit1 = (unsigned)bit >> 7;
				_draw.data[y][_draw.width + xx] = (BitType)((bit5) | (bit5 << 5) | (bit5 << 10) | (bit1 << 15));
			}
		}
		_draw.width += width;
	}

	need_draw = 1;
	return true;
}

bool RemoveInfo(int type) {
	_draw.dead_time = 0;
	need_draw = 0;

	return true;
}


