#pragma once

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef Handle MutexHandle;

MutexHandle MutexCreate();
bool MutexDelete(MutexHandle mth);

bool MutexLock(MutexHandle mth);
bool MutexUnlock(MutexHandle mth);

#ifdef __cplusplus
}
#endif // __cplusplus

