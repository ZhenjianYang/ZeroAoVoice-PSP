#pragma once

#ifndef DEBUG
#define LOG(...)
#else
#include <stdio.h>
#include <psprtc.h>
#include <pspdebug.h>

#define LOG_FILENAME "ms0:/PSP/za_voice/log.txt"

#define LOG(format, ...) { \
pspTime time;\
sceRtcGetCurrentClockLocalTime(&time);\
FILE* _flog = fopen(LOG_FILENAME, "a"); \
fprintf(_flog, "[%04d-%02d-%02d %02d:%02d:%02d.%03d]" format, time.year, time.month, time.day, \
	time.hour, time.minutes, time.seconds, time.microseconds / 1000, ##__VA_ARGS__);\
fprintf(_flog, "\n"); \
fclose(_flog); \
pspDebugScreenPrintf("[%02d:%02d.%03d]" format "\n", \
	time.minutes, time.seconds, time.microseconds / 1000, ##__VA_ARGS__);\
}
#endif // !LOG



