#include "sf_at3.h"

#define FillZero(ptr, size) { for(unsigned __i = 0; __i < size; __i++) *((u8*)(ptr) + __i) = 0; }

static bool _Open(void* source, Sf_Open_Mode mode);
static int _Read(SampleType(*buff)[NUM_CHANNELS_BUF], int count);
static void _Close();
static SoundFile* _sf;

bool InitAt3(SoundFile* soundFile) {
	FillZero(soundFile, sizeof(*soundFile));
	soundFile->Open = &_Open;
	soundFile->Read = &_Read;
	soundFile->Close = &_Close;
	_sf = soundFile;
	return true;
}

#include "io.h"

#include <sys/types.h>
#include <pspkerneltypes.h>
#include <pspatrac3.h>

#define Tag_fact 0x74636166

#define SAMPLES_PER_FRAME_AT3		1024
#define SAMPLES_PER_FRAME_AT3PLUS	2048

#define MAX_FRAME_SIZE   2048

static int _buff_src_len;
static u8 _buff_src[MAX_FRAME_SIZE * 4];

static int _buff_dec_len;
static int _buff_dec_pos;
static int _buff_dec_avl;
static SampleType _buff_dec[SAMPLES_PER_FRAME_AT3PLUS][2];

static int _remain_frames;
static int _flagDecEnd;

static IoHandle _file = NULL;
static Sf_Open_Mode _mode;
static int _at3Id = -1;

bool _Open(void* source, Sf_Open_Mode mode) {
#define FAILED_IF(condition) if(condition) { if(_mode == Sf_Open_Mode_FileName) IoFClose(_file); _file = NULL; return false; }
	_file = _mode == Sf_Open_Mode_FileName ?
				IoFOpen((const char*)source, IO_O_RDONLY)
				: (IoHandle)source;

	if (!_file) return false;

	_buff_src_len = IoFRead(_buff_src, 1, sizeof(_buff_src), _file);
	FAILED_IF(_buff_src_len < 44);
	unsigned offset = 0;

	u32* head = (u32*)(_buff_src + offset);
	FAILED_IF(head[0] != Tag_RIFF || head[2] != Tag_WAVE);
	unsigned file_size = head[1];
	offset += 12;
	
	typedef struct {
		u16 wFormatTag;
		u16 nChannels;
		u32 nSamplesPerSec;
		u32 nAvgBytesPerSec;
		u16 nBlockAlign;
	} FMT;
	u32* tag_size = (u32*)(_buff_src + offset);
	FAILED_IF(tag_size[0] != Tag_fmt || tag_size[1] < sizeof(FMT));
	offset += 8;

	FMT *fmt = (FMT*)(_buff_src + offset);
	_sf->formatTag = fmt->wFormatTag;
	_sf->channels = fmt->nChannels;
	_sf->samplesPerSec = fmt->nSamplesPerSec;
	_sf->avgBytesPerSec = fmt->nAvgBytesPerSec;
	_sf->blockAlign = fmt->nBlockAlign;
	
	FAILED_IF((_sf->formatTag != FORMAT_TAG_ATRAC3 && _sf->formatTag != FORMAT_TAG_ATRAC3PLUS)
		|| _sf->channels > 2 || _sf->blockAlign > MAX_FRAME_SIZE);

	offset += tag_size[1];
	FAILED_IF(offset > file_size);

	_sf->samplesTotal = -1;
	for(;;) {
		if (offset + 8 > file_size) break;
		tag_size = (u32*)(_buff_src + offset); offset += 8;
		if (tag_size[0] == Tag_data) break;

		if (tag_size[0] == Tag_fact && tag_size[1] >= 4 && offset + 4 <= file_size) {
			_sf->samplesTotal = *(s32*)(_buff_src + offset);
		}
		offset += tag_size[1];
	}
	FAILED_IF(_sf->samplesTotal < 0 || tag_size[0] != Tag_data);
	FAILED_IF(tag_size[1] + offset - 8 > file_size);

	_buff_dec_len = _sf->formatTag == FORMAT_TAG_ATRAC3 ? SAMPLES_PER_FRAME_AT3 : SAMPLES_PER_FRAME_AT3PLUS;
	_buff_dec_pos = _buff_dec_len;
	_buff_dec_avl = 0;

	_sf->samplesRead = 0;
	_remain_frames = 1;

	_at3Id = sceAtracSetDataAndGetID(_buff_src, _buff_src_len);
	FAILED_IF(_at3Id < 0);

	_flagDecEnd = 0;
	return true;
}


int _Read(SampleType(*buff)[NUM_CHANNELS_BUF], int count) {
	int read = 0;

	u8* writePointer;
	u32 availableBytes;
	u32 readOffset;

	while (read < count) {
		if (_buff_dec_avl >= count - read) {
			for (int i = 0; i < count - read; i++, _buff_dec_pos++, buff++) {
				(*buff)[0] = _buff_dec[_buff_dec_pos][0];
				(*buff)[1] = _buff_dec[_buff_dec_pos][1];
			}
			_buff_dec_avl -= count - read;
			read = count;
		}
		else {
			for (int i = 0; i < _buff_dec_avl; i++, _buff_dec_pos++, buff++) {
				(*buff)[0] = _buff_dec[_buff_dec_pos][0];
				(*buff)[1] = _buff_dec[_buff_dec_pos][1];
			}
			read += _buff_dec_avl;
			_buff_dec_avl = 0;
			_buff_dec_pos = 0;

			if (!_flagDecEnd && sceAtracGetStreamDataInfo(_at3Id, &writePointer, &availableBytes, &readOffset) >= 0) {
				if (availableBytes > 0 && _remain_frames >= 0) {
					//only need to seek if the at3 has loops
					//io_fseek(file, readOffset, IO_SEEK_SET);
					u32 read_bytes = IoFRead(writePointer, 1, availableBytes, _file);
					if (read_bytes < availableBytes) {
						FillZero(writePointer + read_bytes, availableBytes - read_bytes);
					}
					sceAtracAddStreamData(_at3Id, availableBytes);
				}
				if (sceAtracDecodeData(_at3Id, (u16*)_buff_dec, &_buff_dec_avl, &_flagDecEnd, &_remain_frames) < 0)
					break;
			}
			else {
				break;
			}
		}
	}
	FillZero(buff, (count - read) * sizeof(*buff));

	_sf->samplesRead += read;
	return read;
}

void _Close() {
	if(_file) {
		IoFClose(_file);
		_file = NULL;
	}
	if(_at3Id >= 0) {
		sceAtracReleaseAtracID(_at3Id);
		_at3Id = -1;
	}
}
