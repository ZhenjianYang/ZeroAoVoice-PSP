#include "sf_wav.h"

#define FillZero(ptr, size) { for(unsigned __i = 0; __i < size; __i++) *((char*)ptr + __i) = 0; }

static bool _Open(const char* filename);
static int _Read(SampleType(*buff)[NUM_CHANNELS_BUF], int count);
static void _Close();
static SoundFile* _sf;

bool InitWAV(SoundFile* soundFile) {
	FillZero(soundFile, sizeof(*soundFile));
	soundFile->Open = &_Open;
	soundFile->Read = &_Read;
	soundFile->Close = &_Close;
	_sf = soundFile;
	return true;
}

#include "io.h"

typedef struct WAVEFORMAT {
	u16 wFormatTag;        /* format type */
	u16 nChannels;         /* number of channels (i.e. mono, stereo, etc.) */
	u32 nSamplesPerSec;    /* sample rate */
	u32 nAvgBytesPerSec;   /* for buffer estimation */
	u16 nBlockAlign;       /* block size of data */
	u16 wBitsPerSample;    /* number of bits per sample of mono data */
} WAVEFORMAT;

typedef struct WAVHead
{
	u32 tag_RIFF;
	u32 size;
	u32 tag_WAVE;
	u32 tag_fmt;
	u32 size_fmt;
	WAVEFORMAT fmt;
	u32 tag_data;
	s32 size_data;
} WAVHead;

static IoHandle _file = NULL;

bool _Open(const char* filename) {
#define FAILED_IF(condition) if(condition) { IoFClose(_file); _file = NULL; return false; }

	WAVHead head;

	_file = IoFOpen(filename, IO_O_RDONLY);
	if (_file == 0) {
		return false;
	}
	FAILED_IF(!IoFRead(&head, sizeof(head), 1, _file));

	FAILED_IF(head.tag_RIFF != Tag_RIFF || head.tag_WAVE != Tag_WAVE || head.tag_fmt != Tag_fmt || head.tag_data != Tag_data
		|| head.size_fmt != sizeof(WAVEFORMAT));
	FAILED_IF(head.fmt.nChannels != 1 && head.fmt.nChannels != 2);
	
	_sf->formatTag = head.fmt.wFormatTag;
	_sf->channels = head.fmt.nChannels;
	_sf->samplesPerSec = head.fmt.nSamplesPerSec;
	_sf->avgBytesPerSec = head.fmt.nAvgBytesPerSec;
	_sf->blockAlign = head.fmt.nBlockAlign;
	
	_sf->samplesTotal = head.size_data / head.fmt.nBlockAlign;
	_sf->samplesRead = 0;

	return true;
}

int _Read(SampleType (*buff)[NUM_CHANNELS_BUF], int count) {

	int request = _sf->samplesTotal - _sf->samplesRead;
	if (request > count) request = count;
	
	SampleType* wBuff = (SampleType*)buff;
	if (_sf->channels == 1) wBuff += count;

	int read = IoFRead(wBuff, _sf->blockAlign, count, _file);

	if (_sf->channels == 1) {
		for(int i = 0; i < read; i++) {
			buff[i][0] = buff[i][1] = wBuff[i];
		}
	}
	for (int i = read; i < count; i++) buff[i][0] = buff[i][1] = 0;

	_sf->samplesRead += read;
	return read;
}

void _Close() {
	if (_file) {
		IoFClose(_file);
		_file = NULL;
	}
}

