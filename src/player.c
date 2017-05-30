#include <pspaudio.h>
#include <pspthreadman.h>

#include "player.h"
#include "event.h"
#include "mutex.h"
#include "log.h"

#define SAMNUM_ONETIME 1600
#define SMPNUM_HALFBUFF (SAMNUM_ONETIME * 15)
#define SMPNUM_BUFF (2 * SMPNUM_HALFBUFF)
#define CHANNELS NUM_CHANNELS_BUF

static EventHandle _evh_sound;
static EventHandle _evh_read;
enum PlayEvents {
	None = 0,

	InitPlayRequest = 1 << 0,
	StopPlayRequest = 1 << 1,
	PlayRequest = 1 << 2,

	EndSoundRequest = 1 << 9,


	InitPlayDone = 1 << 10,
	StopPlayDone = 1 << 11,
	PlayDone = 1 << 12,

	NewFileRequest = 1 << 20,
	CloseFileRequest = 1 << 21,
	ReadFirstHalfRequest = 1 << 22,
	ReadSecondHalfRequest = 1 << 23,

	EndReadRequest = 1 << 29,
};

static int _audioId = -1;
static unsigned _rate = 0;
static int _volume;

static SoundFile _sf;

static InitSfCall _initSfCall;
static char _fileName[64];
static MutexHandle _mt;

static SceUID _th_sound = -1;
static SceUID _th_read = -1;

static int _pos = 0;
static SampleType _soundbuff[SMPNUM_BUFF][CHANNELS];

static int soundThread(SceSize args, void *argp) {
	for(;;) {
		EventFlags req = EventWait(_evh_sound,
						InitPlayRequest | StopPlayRequest | PlayRequest | EndSoundRequest,
						EVENT_WAITOR);

		if(req & EndSoundRequest) {
			EventClear(_evh_sound, EndSoundRequest);
			break;
		} else if (req & StopPlayRequest) {
			EventClear(_evh_sound, StopPlayRequest);
			EventClear(_evh_sound, PlayRequest);

			EventSet(_evh_read, StopPlayDone);
		} else if (req & InitPlayRequest) {
			EventClear(_evh_sound, InitPlayRequest);

			if(_audioId >= 0) {
				_pos = 0;
				EventSet(_evh_sound, PlayRequest);
			}
			EventSet(_evh_read, InitPlayDone);
		} else if (req & PlayRequest) {
			if(_audioId >= 0) {
				if(_pos == 0) EventSet(_evh_read, ReadSecondHalfRequest);
				else if(_pos == SMPNUM_HALFBUFF) EventSet(_evh_read, ReadFirstHalfRequest);

				sceAudioSRCOutputBlocking(_volume, _soundbuff + _pos);

				_pos += SAMNUM_ONETIME;
				_pos %= SMPNUM_BUFF;
			}
			else {
				EventClear(_evh_sound, PlayRequest);
				continue;
			}
		}
	}
	return sceKernelExitDeleteThread(0);
}

static int readThread(SceSize args, void *argp) {
	int read_left = SMPNUM_HALFBUFF, read_right = SMPNUM_HALFBUFF;
	for (;;) {
		EventFlags req = EventWait(_evh_read,
				NewFileRequest | CloseFileRequest | ReadFirstHalfRequest | ReadSecondHalfRequest | EndReadRequest,
				EVENT_WAITOR | EVENT_WAITCLEAR);
		if (req & EndReadRequest) {
			if(_sf.Close) _sf.Close();
			if(_audioId >= 0) {
				sceAudioSRCChRelease();
			}
			break;
		} else if(req & CloseFileRequest) {
			EventSet(_evh_sound, StopPlayRequest);
			if(_sf.Close) _sf.Close();
			EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);
		} else if(req & NewFileRequest) {
			EventSet(_evh_sound, StopPlayRequest);
			if(_sf.Close) _sf.Close();
			EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);

			MutexLock(_mt);
			bool ok = _initSfCall && _initSfCall(&_sf) && _sf.Open(_fileName);
			MutexUnlock(_mt);

			if(!ok) {
				LOG("Open Failed.");
				continue;
			}

			LOG("Sound File Info:\n"
				"    FormatTag = 0x%04X\n"
				"    Channels  = %d\n"
				"    SamplesPerSec = %d\n"
				"    AvgBytesPerSec = %d\n"
				"    BlockAlign = %d\n"
				"    SamplesTotal =  %d",
				_sf.formatTag,
				_sf.channels,
				_sf.samplesPerSec,
				_sf.avgBytesPerSec,
				_sf.blockAlign,
				_sf.samplesTotal
			);

			read_left = _sf.Read(_soundbuff, SMPNUM_HALFBUFF);
			read_right = SMPNUM_HALFBUFF;

			if(_audioId >= 0 && _rate != _sf.samplesPerSec) {
				if(sceAudioSRCChRelease() >= 0) {
					_audioId = -1;
					LOG("Released AudioSRCCh.");
				} else {
					LOG("Release AudioSRCCh Failed.");
				}
			}

			_rate = _sf.samplesPerSec;
			if(_audioId < 0) {
				_audioId = sceAudioSRCChReserve(SAMNUM_ONETIME, _rate, CHANNELS);
				LOG("Reserve AudioSRCCh : 0x%08X", _audioId);
			}

			if(_audioId >= 0) {
				LOG("Play start.");
				EventSet(_evh_sound, InitPlayRequest);
				EventWait(_evh_read, InitPlayDone, EVENT_WAITOR | EVENT_WAITCLEAR);
			} else {
				_sf.Close();
				LOG("Failed to start playing.");
			}
		} else if(req & ReadFirstHalfRequest) {
			read_left = _sf.Read(_soundbuff, SMPNUM_HALFBUFF);
			if(read_left == 0 && read_right < SMPNUM_HALFBUFF) {
				EventSet(_evh_sound, StopPlayRequest);
				_sf.Close();
				EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);
			}
		} else if(req & ReadSecondHalfRequest) {
			read_right = _sf.Read(_soundbuff + SMPNUM_HALFBUFF, SMPNUM_HALFBUFF);
			if(read_right == 0 && read_left < SMPNUM_HALFBUFF) {
				EventSet(_evh_sound, StopPlayRequest);
				_sf.Close();
				EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);
			}
		}
	}

	return sceKernelExitDeleteThread(0);
}

bool InitPlayer() {
	_evh_sound = EventCreate(false, 0);
	_evh_read = EventCreate(false, 0);
	_mt = MutexCreate();

	_th_sound = sceKernelCreateThread("soundThread", soundThread, 0x30, 0x1000, 0, 0);
	_th_read = sceKernelCreateThread("readThread", readThread, 0x30, 0x4000, 0, 0);

	if (_th_sound >= 0 && _th_read >= 0 && _evh_sound && _evh_read && _mt) {
		sceKernelStartThread(_th_sound, 0, 0);
		LOG("soundThread created");

		sceKernelStartThread(_th_read, 0, 0);
		LOG("readThread created");
	} else {
		LOG("Creat Thread Failed.");
		if(_th_sound >= 0) sceKernelDeleteThread(_th_sound);
		if(_th_read >= 0) sceKernelDeleteThread(_th_read);

		EventDelete(_evh_sound); _evh_sound = NULL;
		EventDelete(_evh_read); _evh_read = NULL;
		MutexDelete(_mt); _mt = NULL;
		_th_sound = _th_read = -1;

		return false;
	}

	_volume = PSP_AUDIO_VOLUME_MAX;
	return true;
}

bool EndPlayer() {
	EventSet(_evh_sound, EndSoundRequest);
	EventSet(_evh_read, EndReadRequest);
	if(_th_sound >= 0) sceKernelWaitThreadEnd(_th_sound, NULL);
	if(_th_read >= 0) sceKernelWaitThreadEnd(_th_read, NULL);
	EventDelete(_evh_sound); _evh_sound = NULL;
	EventDelete(_evh_read); _evh_read = NULL;
	MutexDelete(_mt); _mt = NULL;
	_th_sound = _th_read = -1;
	return true;
}

bool PlaySound(const char* filename, int volume, InitSfCall initSf) {
	LOG("File: %s, volume = 0x%04X, init = 0x%08X", filename, volume, (unsigned)initSf);
	MutexLock(_mt);
	_volume = volume;
	_initSfCall = initSf;
	for(unsigned i = 0; i < sizeof(_fileName) && filename[i]; i++) {
		_fileName[i] = filename[i];
	}
	_fileName[sizeof(_fileName) - 1] = '\0';
	MutexUnlock(_mt);
	EventSet(_evh_read, NewFileRequest);
	return true;
}

bool StopSound() {
	EventSet(_evh_read, CloseFileRequest);
	return true;
}

