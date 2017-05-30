#include <pspmoduleinfo.h>
#include <pspkerneltypes.h>

#include "za_voice.h"

#ifndef __cplusplus
#define za_voice "za_voice"
#endif
PSP_MODULE_INFO(za_voice, PSP_MODULE_USER, 1, 0);
PSP_HEAP_SIZE_KB(256);

#ifdef __cplusplus
extern "C"
#endif
int module_start(SceSize args, void* argp) {
	return InitZaVoice(args, argp);
}

#ifdef __cplusplus
extern "C"
#endif
int module_stop(SceSize args, void *argp) {
	return EndZaVoice(args, argp);
}
