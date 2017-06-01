#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void h_voice();
void H_voice(const char* p);

extern void h_dududu();
extern int h_dududu_volume;

extern void h_dlgse();
extern int h_dlgse_volume;
extern unsigned h_sub_se;

void H_stop_voice();

#ifdef __cplusplus
}
#endif // __cplusplus
