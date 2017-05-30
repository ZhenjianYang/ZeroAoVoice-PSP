#include "type.h"

#include <pspctrl.h>
#include <pspthreadman.h>
#include <pspmodulemgr.h>

#ifdef DEBUG
#include <pspiofilemgr.h>
#define LOG_FILE "ms0:/za_voice/load_log.bin"
#define LOG(_id, value) { \
unsigned id = _id;\
SceUID f = sceIoOpen(LOG_FILE, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);\
if(f >= 0) { sceIoWrite(f, &id, sizeof(id)); sceIoWrite(f, &value, sizeof(value)); }\
sceIoClose(f);\
}
#else
#define LOG(...)
#endif

//typedef struct {
//    /* The number of entry thread parameters, typically 3. */
//    u32 numParams;
//    /* The initial priority of the entry thread. */
//    u32 initPriority;
//    /* The stack size of the entry thread. */
//    u32 stackSize;
//    /* The attributes of the entry thread. */
//    u32 attr;
//} SceModuleEntryThread;
//#ifdef __cplusplus
//extern "C"
//#endif
//const SceModuleEntryThread module_start_thread_parameter = {
//		3,
//		0x1F,
//		0x400,
//		0x1000
//};

#ifndef __cplusplus
#define EbootLoader "EbootLoader"
#endif
PSP_MODULE_INFO(EbootLoader, PSP_MODULE_USER, 1, 0);

#define PathOldBoot  "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"
#define PathPrx "ms0:/za_voice/za_voice.prx"

static int main_thread(SceSize args, void *argp) {
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(&pad, 1);

	char str_eboot[] = PathOldBoot;
	SceUID mid_eboot = sceKernelLoadModule(str_eboot, 0, NULL);
	LOG(0, mid_eboot);
	int stat_eboot;
	
	//load za_voice only if RTRIGGER is not held
	if (mid_eboot >= 0 && !(pad.Buttons & PSP_CTRL_RTRIGGER)) {
		SceUID mid_prx = sceKernelLoadModule(PathPrx, 0, NULL);
		int stat_za;
		if (mid_prx >= 0) {
			sceKernelStartModule(mid_prx, sizeof(mid_eboot), &mid_eboot, &stat_za, NULL);
		}
	}

	if (mid_eboot >= 0) {
		sceKernelStartModule(mid_eboot, sizeof(str_eboot), str_eboot, &stat_eboot, NULL);
	}

	return sceKernelExitDeleteThread(0);
}

#ifdef __cplusplus
extern "C"
#endif
int module_start (SceSize args, void* argp) {
	int th = sceKernelCreateThread("loader", main_thread, 0x1F, 0x1000, 0, 0);
	if (th >= 0) {
		sceKernelStartThread(th, args, argp);
	}
	return 0;
}

#ifdef __cplusplus
extern "C"
#endif
int module_stop (SceSize args, void *argp) {
    return 0;
}
