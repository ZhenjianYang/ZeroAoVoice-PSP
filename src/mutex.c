#include "mutex.h"

#include <sceKernelMutex.h>

MutexHandle MutexCreate() {
	SceUID mid = sceKernelCreateMutex("Mutex", 0, 0, NULL);
	return mid < 0 ? NULL : (MutexHandle)ParseHandle(mid);
}

bool MutexDelete(MutexHandle mth) {
	return sceKernelDeleteMutex((SceUID)ParseHandle(mth)) >= 0;
}

bool MutexLock(MutexHandle mth) {
	return sceKernelLockMutex((SceUID)ParseHandle(mth), 1, NULL) >= 0;
}

bool MutexUnlock(MutexHandle mth) {
	return sceKernelUnlockMutex((SceUID)ParseHandle(mth), 1) >= 0;
}

