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

static inline int _readNextbyte(IoHandle ioh) {
	int rst = 0;
	if(sceIoRead((SceUID)ParseHandle(ioh), &rst, 1) == 0) {
		return -1;
	} else {
		return rst;
	}
}

int IoFReadStr(IoHandle ioh, char* buff, int max_ch, const char* endMarkList, int cnt) {
	int len = 0;
	while(len < max_ch) {
		int byte = _readNextbyte(ioh);
		if(byte < 0) break;

		int end = 0;
		for(int i = 0; i < cnt; i++) {
			if((char)byte == endMarkList[i]) {
				end = 1;
				break;
			}
		}
		if(end) {
			if(len == 0) continue;
			else break;
		} else {
			buff[len++] = (char)byte;
		}
	}
	buff[len] = '\0';
	return len;
}

int IoFReadUInt(IoHandle ioh, unsigned int* out) {
	int len = 0;
	*out = 0;
	for(;;) {
		int byte = _readNextbyte(ioh);
		if(byte < 0) break;

		if(byte < '0' || byte > '9') {
			if(len == 0) continue;
			else break;
		}
		*out *= 10;
		*out += byte - '0';
		len++;
	}
	return len;
}

int IoFWriteStr(IoHandle ioh, const char* str) {
	int len = 0;
	while(str[len]) len++;
	sceIoWrite((SceUID)ParseHandle(ioh), str, len);
	return len;
}
int IoFWriteUInt(IoHandle ioh, unsigned int num) {
	int len = 0;
	char buff[12];
	while(num) {
		buff[sizeof(buff) - 1 - len] = num % 10 + '0';
		num /= 10;
		len++;
	}
	if(len == 0) { buff[sizeof(buff) - 1] = '0'; len = 1; }
	sceIoWrite((SceUID)ParseHandle(ioh), buff + sizeof(buff) - len, len);
	return len;
}

bool IoDirExists(const char* dir) {
	SceUID did = sceIoDopen(dir);
	if(dir < 0) return false;
	else return sceIoDclose(did) >= 0;
}
