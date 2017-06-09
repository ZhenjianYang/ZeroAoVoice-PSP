#include "sf_ogg.h"

#define FillZero(ptr, size) { for(unsigned __i = 0; __i < size; __i++) *((char*)ptr + __i) = 0; }

static bool _Open(void* source, Sf_Open_Mode mode);
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

typedef struct VPV {
	Handle ioh;
	unsigned offset;
	unsigned size;
	unsigned pos;
} VPV;
static unsigned VPVoice_Read(void * buf, unsigned sz, unsigned cnt, void* vpv) {
	VPV* pvpv = (VPV*)vpv;
	if(pvpv->pos >= pvpv->size) return 0;

	unsigned request = (pvpv->size - pvpv->pos) / sz;
	if(request > cnt) request = cnt;

	unsigned read = IoFRead(buf, sz, request, pvpv->ioh);
	pvpv->pos += read * sz;

	if(read < cnt) {
		IoFSeek(pvpv->ioh, pvpv->offset + pvpv->pos, IO_SEEK_SET);
	}
	return read;
}
static int VPVoice_Seek(void* vpv, long long off, int whence) {
	VPV* pvpv = (VPV*)vpv;
	switch(whence) {
	case IO_SEEK_SET:
		off += pvpv->offset;
		break;
	case IO_SEEK_CUR:
		off += pvpv->offset + pvpv->pos;
		break;
	case IO_SEEK_END:
		off += pvpv->offset + pvpv->size;
		break;
	}

	if(off < (long long)pvpv->offset || off > (long long)pvpv->offset + pvpv->size) return -1;
	if(IoFSeek(pvpv->ioh, off, IO_SEEK_SET) < 0) return -1;
	pvpv->pos = (unsigned)(off - pvpv->offset);
	return 0;
}

static long VPVoice_Tell(void* vpv) {
	return ((VPV*)vpv)->pos;
}

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
static ov_callbacks ov_callbacks_Io = {
	IoFRead,
	IoFSeek,
	IoFClose,
	IoFTell
};
static ov_callbacks ov_callbacks_forPack = {
	VPVoice_Read,
	VPVoice_Seek,
	NULL,
	VPVoice_Tell
};

static OggVorbis_File _ovFile;
static Sf_Open_Mode _mode;
static VPV _vpv;

bool _Open(void* source, Sf_Open_Mode mode) {
	_mode = mode;
#define FAILED_IF(condition) if(condition) { if(_mode == Sf_Open_Mode_FileName) IoFClose(_file); _file = NULL; return false; }

	void* _file = NULL;
	if(_mode == Sf_Open_Mode_FileName) {
		_file = IoFOpen((const char*)source, IO_O_RDONLY);
	} else {
		_file = &_vpv;
		_vpv.ioh = ((Sf_Ioh_Param*)source)->ioh;
		_vpv.size = ((Sf_Ioh_Param*)source)->size;
		_vpv.offset = ((Sf_Ioh_Param*)source)->offset;
		_vpv.pos = 0;
	}

	if (_file == NULL) {
		return false;
	}

	FAILED_IF(ov_open_callbacks(_file, &_ovFile, 0, 0,
			_mode == Sf_Open_Mode_FileName ? ov_callbacks_Io : ov_callbacks_forPack));

	vorbis_info* info = ov_info(&_ovFile, -1);

	_sf->formatTag = FORMAT_TAG_OGG;
	_sf->channels = info->channels;
	_sf->samplesPerSec = info->rate;
	_sf->blockAlign = info->channels * 2;
	_sf->avgBytesPerSec = _sf->blockAlign * _sf->samplesPerSec;

	if(_sf->channels > 2) {
		ov_clear(&_ovFile);
		return false;
	}

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
