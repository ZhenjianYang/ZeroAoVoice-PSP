#pragma once

#include <pspctrl.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define KEY_AUTO_PLAY (PSP_CTRL_SQUARE | PSP_CTRL_RIGHT)

#define KEY_VOLUME_UP (PSP_CTRL_SQUARE | PSP_CTRL_UP)
#define KEY_VOLUME_DOWN (PSP_CTRL_SQUARE | PSP_CTRL_DOWN)

#define KEY_VOLUME_BIG_STEP PSP_CTRL_TRIANGLE

#define VOLUME_STEP 1
#define VOLUME_STEP_BIG 5

void SwitchAutoPlay();
void AddVolume(int add);

#ifdef __cplusplus
}
#endif // __cplusplus

