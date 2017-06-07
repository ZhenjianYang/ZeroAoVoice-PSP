#include "voice_pack.h"
#include "log.h"

#include <pspsysmem.h>

bool VP_Init(struct VoicePack *voice_pack, const char* pack_file) {
	if(!voice_pack) return false;

	voice_pack->count = 0;
	voice_pack->ioh = IoFOpen(pack_file, IO_O_RDONLY);
	if(!voice_pack->ioh) {
		LOG("Open file failed : %s", pack_file);
		return false;
	}

	SceUID memID;
	unsigned count;
	if(!IoFRead(&count, sizeof(count), 1, voice_pack->ioh)
			|| count > VP_MAX_VOICE_NUM || count == 0
			|| !IoFRead(voice_pack->ext, sizeof(voice_pack->ext), 1, voice_pack->ioh)
			|| (memID = sceKernelAllocPartitionMemory(2, "VoicePack", 0, count * sizeof(VoiceInfo) + sizeof(SceUID), NULL)) < 0
			) {
		LOG("Read info failed.");

		IoFClose(voice_pack->ioh);
		voice_pack->ioh = NULL;
		return false;
	}

	SceUID *p = (SceUID*)sceKernelGetBlockHeadAddr(memID);
	*p = memID;
	voice_pack->voice_info_list = (VoiceInfo*)(p + 1);
	voice_pack->count = IoFRead(voice_pack->voice_info_list, sizeof(VoiceInfo), count, voice_pack->ioh);

	LOG("Voice pack:\n"
		"    count(record) = %d\n"
		"    count = %d\n"
		"    ext = %s\n",
		count, voice_pack->count, voice_pack->ext);

	return true;
}
bool VP_Finish(struct VoicePack *voice_pack) {
	if(!voice_pack) return false;

	if(voice_pack->ioh) { IoFClose(voice_pack->ioh); voice_pack->ioh = NULL; }
	if(voice_pack->voice_info_list) {
		SceUID memID = *((SceUID*)voice_pack->voice_info_list - 1);
		sceKernelFreePartitionMemory(memID);
		voice_pack->voice_info_list = NULL;
	}

	voice_pack->count = 0;
	return true;
}

const struct VoiceInfo* VP_Find(struct VoicePack *voice_pack, unsigned voice_id) {
	VoiceInfo* vil = voice_pack->voice_info_list;
	unsigned count = voice_pack->count;

	VoiceInfo* first = vil;
	VoiceInfo* middle;
	int half, len = count;

	while (len > 0) {
		half = len >> 1;
		middle = first + half;
		if (middle->voice_id < voice_id) {
			first = middle + 1;
			len = len - half - 1;
		} else
			len = half;
	}

	return first < vil + count && first->voice_id == voice_id ?
			first
			: NULL;
}

bool VP_SetOffset(struct VoicePack *voice_pack, unsigned voice_id) {
	const VoiceInfo* vi = VP_Find(voice_pack, voice_id);
	if(!vi) return false;

	IoFSeek(voice_pack->ioh, vi->offset, IO_SEEK_SET);
	return true;
}


