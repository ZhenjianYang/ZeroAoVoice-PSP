#include "event.h"

#include <pspthreadman.h>

EventHandle EventCreate(bool waitedByMultiThread, EventFlags initialBits) {
	SceUID evid = sceKernelCreateEventFlag("Event", waitedByMultiThread ? PSP_EVENT_WAITMULTIPLE : 0, initialBits, NULL);
	return evid < 0 ? NULL : (EventHandle)ParseHandle(evid);
}

bool EventDelete(EventHandle evh) {
	return sceKernelDeleteEventFlag((SceUID)ParseHandle(evh)) >= 0;
}

EventFlags EventWait(EventHandle evh, EventFlags bits, int types) {
	u32 res = 0;
	if(sceKernelWaitEventFlag((SceUID)ParseHandle(evh), (u32)bits, (u32)types, &res, NULL) < 0) return 0;

	return (EventFlags)res;
}

bool EventSet(EventHandle evh, EventFlags bits) {
	return sceKernelSetEventFlag((SceUID)ParseHandle(evh), bits) >= 0;
}

bool EventClear(EventHandle evh, EventFlags bits) {
	return sceKernelClearEventFlag((SceUID)ParseHandle(evh), ~bits) >= 0;
}

