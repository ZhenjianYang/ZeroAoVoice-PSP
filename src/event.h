#pragma once

#include <type.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef Handle EventHandle;
typedef int EventFlags;

enum {
	EVENT_WAITAND = 0, //wait all bits
	EVENT_WAITOR = 1, //wait one of any bits
	EVENT_WAITCLEAR = 0x20 //clear wait bits when matched
};

EventHandle EventCreate(bool waitForMultiThread, EventFlags initialBits);
bool EventDelete(EventHandle evh);

EventFlags EventWait(EventHandle evh, EventFlags bits, int types);
bool EventSet(EventHandle evh, EventFlags bits);
bool EventClear(EventHandle evh, EventFlags bits);

#ifdef __cplusplus
}
#endif // __cplusplus

