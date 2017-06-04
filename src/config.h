#pragma once

#include "basic_type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define Max_Volume 100

#define AutoPlay_VoiceOnly			1
#define AutoPlay_All				2

typedef struct Config {
	int Volume;
	int AutoPlay;
	int WaitTimePerChar;
	int WaitTimeDialog;
	int WaitTimeDialogWithVoice;
	int SkipVoice;
	int DisableDialogTextSE;
	int DisableDialogSwitchSE;
	int DisableOriginalVoice;
} Config;

bool LoadConfig(struct Config* config, const char* cfg_file);
bool SaveConfig(const struct Config* config, const char* cfg_file);

#ifdef __cplusplus
}
#endif // __cplusplus
