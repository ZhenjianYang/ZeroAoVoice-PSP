#include "io.h"

#include <pspiofilemgr.h>

IoHandle IoFOpen(const char* fileName, int mode) {
	SceUID f = sceIoOpen(fileName, mode, 0777);
	return f < 0 ? NULL : (IoHandle)ParseHandle(f);
}

unsigned IoFRead(void * buf, unsigned sz, unsigned cnt, IoHandle ioh) {
	return sceIoRead((SceUID)ParseHandle(ioh), buf, sz * cnt) / sz;
}

unsigned IoFWrite(const void * buf, unsigned sz, unsigned cnt, IoHandle ioh) {
	return sceIoWrite((SceUID)ParseHandle(ioh), buf, sz * cnt) / sz;
}

int IoFSeek(IoHandle ioh, long long off, int whence) {
	sceIoLseek((SceUID)ParseHandle(ioh), off, whence);
	return 0;
}

long IoFTell(IoHandle ioh) {
	return sceIoLseek((SceUID)ParseHandle(ioh), 0, IO_SEEK_CUR);
}

int IoFClose(IoHandle ioh) {
	return sceIoClose((SceUID)ParseHandle(ioh));
}

bool IoDirExists(const char* dir) {
	SceUID did = sceIoDopen(dir);
	if(dir < 0) return false;
	else return sceIoDclose(did) >= 0;
}
