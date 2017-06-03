#pragma once


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct InitPlayerParam {
	int* p_h_dududu_volume;
	int* p_h_dlgse_volume;
} InitPlayerParam;

typedef struct Player {
	const char* filename;
	int volume;
	void* initSf;

	unsigned* pfm_cnt;
} Player;

#ifdef __cplusplus
}
#endif // __cplusplus


