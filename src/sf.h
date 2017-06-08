#pragma once

#include "basic_type.h"

#define Tag_RIFF 0x46464952
#define Tag_WAVE 0x45564157
#define Tag_fmt  0x20746D66
#define Tag_data 0x61746164

#define NUM_CHANNELS_BUF 2

typedef enum Sf_Open_Mode {
	Sf_Open_Mode_FileName,
	Sf_Open_Mode_IoHandle,
} Sf_Open_Mode;

typedef u16 SampleType;

typedef struct Sf_Ioh_Param {
	Handle ioh;
	unsigned offset;
	unsigned size;
} Sf_Ioh_Param;

typedef struct SoundFile {
	bool (*Open)(void* source, Sf_Open_Mode mode);
	int (*Read)(SampleType (*buff_dec)[NUM_CHANNELS_BUF], int count);
	void (*Close)();

	int samplesRead;
	int samplesTotal;
	
	u16 formatTag;        /* format type */
	u16 channels;         /* number of channels (i.e. mono, stereo, etc.) */
	u32 samplesPerSec;    /* sample rate */
	u32 avgBytesPerSec;   /* for buffer estimation */
	u16 blockAlign;       /* block size of data */
} SoundFile;

typedef bool (*InitSfCall)(SoundFile* );

