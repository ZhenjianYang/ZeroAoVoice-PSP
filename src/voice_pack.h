#pragma once

#include "basic_type.h"
#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define VP_MAX_VOICE_NUM 50000

typedef struct VoiceInfo {
	unsigned voice_id;
	unsigned offset;
	unsigned size;
} VoiceInfo;

typedef struct VoicePack {
	unsigned count;
	char ext[4];

	IoHandle ioh;
	struct VoiceInfo* voice_info_list;
} VoicePack;

bool VP_Init(struct VoicePack *voice_pack, const char* pack_file);
bool VP_Finish(struct VoicePack *voice_pack);

const struct VoiceInfo* VP_Find(struct VoicePack *voice_pack, unsigned voice_id);
bool VP_SetOffset(struct VoicePack *voice_pack, unsigned voice_id);

#ifdef __cplusplus
}
#endif // __cplusplus

