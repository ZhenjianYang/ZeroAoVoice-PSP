#pragma once

#include "basic_type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern int need_draw;

enum InfoType {
	InfoType_Volume = 0,
	InfoType_AutoPlay,

	InfoType_TotalCount
};

#define INFOTIME_INFINITY 0

typedef struct Info {
	int type;
	unsigned time;

	const char* text;
} Info;

int Draw();

bool AddInfo(const Info *info);
bool RemoveInfo(int type);

#ifdef __cplusplus
}
#endif
