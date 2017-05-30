
#ifndef __THREADMAN_MUTEX_H__
#define __THREADMAN_MUTEX_H__

#include <pspkerneltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_MUTEX_ATTR_FIFO 0
#define PSP_MUTEX_ATTR_PRIORITY 0x100
#define PSP_MUTEX_ATTR_ALLOW_RECURSIVE 0x200
#define PSP_MUTEX_ATTR_KNOWN (PSP_MUTEX_ATTR_PRIORITY | PSP_MUTEX_ATTR_ALLOW_RECURSIVE)

SceUID sceKernelCreateMutex(const char *name, SceUInt attr, int initCount, void* opt);
int sceKernelDeleteMutex(SceUID id);
int sceKernelLockMutex(SceUID id, int count, unsigned int *timeout);
int sceKernelUnlockMutex(SceUID id, int count);

#ifdef __cplusplus
}
#endif

#endif
