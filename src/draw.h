#pragma once

#include "basic_type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern int need_draw;

#define INFOTIME_INFINITY 0

typedef struct Info {
	unsigned time;
	const char* text;
} Info;

bool InitDraw();

int Draw();

bool AddInfo(const Info *info);
bool RemoveInfo();

#ifdef __cplusplus
}
#endif
