#include "sf_ogg.h"

#define FillZero(ptr, size) { for(unsigned __i = 0; __i < size; __i++) *((char*)ptr + __i) = 0; }

static bool _Open(const char* filename);
static int _Read(SampleType(*buff)[NUM_CHANNELS_BUF], int count);
static void _Close();
static SoundFile* _sf;

bool InitOgg(SoundFile* soundFile) {
	FillZero(soundFile, sizeof(*soundFile));
	soundFile->Open = &_Open;
	soundFile->Read = &_Read;
	soundFile->Close = &_Close;
	_sf = soundFile;
	return true;
}

#include "io.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
static ov_callbacks ov_callbacks_Io = {
	IoFRead,
	IoFSeek,
	IoFClose,
	IoFTell
};

static OggVorbis_File _ovFile;

bool _Open(const char* filename) {
#define FAILED_IF(condition) if(condition) { IoFClose(file); file = NULL; return false; }

	void* file= IoFOpen(filename, IO_O_RDONLY);
	if (file == NULL) {
		return false;
	}

	FAILED_IF(ov_open_callbacks(file, &_ovFile, 0, 0, ov_callbacks_Io));
	vorbis_info* info = ov_info(&_ovFile, -1);

	_sf->formatTag = FORMAT_TAG_OGG;
	_sf->channels = info->channels;
	_sf->samplesPerSec = info->rate;
	_sf->blockAlign = info->channels * 2;
	_sf->avgBytesPerSec = _sf->blockAlign * _sf->samplesPerSec;

	FAILED_IF(_sf->channels > 2);

	_sf->samplesRead = 0;
	_sf->samplesTotal = ov_pcm_total(&_ovFile, -1);

	return true;
}

int _Read(SampleType(*buff)[NUM_CHANNELS_BUF], int count) {
#define block 4096

	int bitstream = 0;
	int read = 0;
	
	SampleType* wBuff = (SampleType*)buff;
	if (_sf->channels == 1) wBuff += count;

	while (read < count)
	{
		int request = count - read;
		if(request > block) request = block;
		int tread = ov_read(&_ovFile, (char*)wBuff + _sf->blockAlign * read,
				_sf->blockAlign, 0, 2, 1, &bitstream) / _sf->blockAlign;
		if (tread <= 0) break;

		read += tread;
	}

	if (_sf->channels == 1) {
		for (int i = 0; i < read; i++) {
			buff[i][0] = buff[i][1] = wBuff[i];
		}
	}
	for (int i = read; i < count; i++) buff[i][0] = buff[i][1] = 0;

	_sf->samplesRead += read;
	return read;
}

void _Close() {
	ov_clear(&_ovFile);
}
