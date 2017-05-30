#pragma once

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define IO_O_RDONLY	0x0001
#define IO_O_WRONLY	0x0002
#define IO_O_RDWR	(IO_O_RDONLY | IO_O_WRONLY)
#define IO_O_NBLOCK	0x0004
#define IO_O_DIROPEN	0x0008	// Internal use for dopen
#define IO_O_APPEND	0x0100
#define IO_O_CREAT	0x0200
#define IO_O_TRUNC	0x0400
#define	IO_O_EXCL	0x0800
#define IO_O_NOWAIT	0x8000

#define IO_SEEK_SET	0
#define IO_SEEK_CUR	1
#define IO_SEEK_END	2

typedef Handle IoHandle;

IoHandle IoFOpen(const char* fileName, int mode);

unsigned IoFRead(void * buf, unsigned sz, unsigned cnt, IoHandle ioh);

unsigned IoFWrite(const void * buf, unsigned sz, unsigned cnt, IoHandle ioh);

int IoFSeek(IoHandle ioh, long long off, int whence);

long IoFTell(IoHandle ioh);

int IoFClose(IoHandle ioh);

bool IoDirExists(const char* dir);

#ifdef __cplusplus
}
#endif // __cplusplus

