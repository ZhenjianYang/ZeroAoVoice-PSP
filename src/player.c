#include <pspaudio.h>
#include <pspthreadman.h>

#include "player.h"
#include "voice_pack.h"
#include "global.h"
#include "sf.h"
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

	NewSoundRequest = 1 << 20,
	StopSoundRequest = 1 << 21,
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
static unsigned _voice_id;
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

				sceAudioSRCOutputBlocking(_volume * PSP_AUDIO_VOLUME_MAX / Max_Volume, _soundbuff + _pos);

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
	char fileName[64];
	unsigned voice_id;
	InitSfCall initSfCall;

	int read_left = SMPNUM_HALFBUFF, read_right = SMPNUM_HALFBUFF;
	for (;;) {
		EventFlags req = EventWait(_evh_read,
				NewSoundRequest | StopSoundRequest | ReadFirstHalfRequest | ReadSecondHalfRequest | EndReadRequest,
				EVENT_WAITOR | EVENT_WAITCLEAR);
		if (req & EndReadRequest) {
			if(_sf.Close) _sf.Close();
			if(_audioId >= 0) {
				sceAudioSRCChRelease();
			}
			break;
		} else if(req & StopSoundRequest) {
			EventSet(_evh_sound, StopPlayRequest);
			if(_sf.Close) _sf.Close();
			EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);
		} else if(req & NewSoundRequest) {
			EventSet(_evh_sound, StopPlayRequest);
			if(_sf.Close) _sf.Close();
			EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);

			MutexLock(_mt);
			initSfCall = _initSfCall;
			voice_id = _voice_id;
			if(!voice_id) {
				for(unsigned i = 0; i < sizeof(fileName) && _fileName[i]; i++) {
					fileName[i] = _fileName[i];
				}
			}
			MutexUnlock(_mt);

			bool ok = initSfCall(&_sf);

			if(ok) {
				if(voice_id) {
					const VoiceInfo *vi;
					if((vi = VP_Find(&g.vp, voice_id)) != NULL) {
						LOG("Voice Found. ioh = 0x%08X, offset = 0x%08X, size = 0x%08X", (unsigned)g.vp.ioh, vi->offset, vi->size);

						Sf_Ioh_Param param = { g.vp.ioh, vi->offset, vi->size };
						IoFSeek(g.vp.ioh, vi->offset, IO_SEEK_SET);

						ok = _sf.Open(&param, Sf_Open_Mode_IoHandle);
					} else {
						LOG("Voice Not Found.");
						ok = false;
					}
				} else {
					ok = _sf.Open(_fileName, Sf_Open_Mode_FileName);
				}
			}

			if(!ok) {
				LOG("Open Failed.");
				g.order.disableDududu = 0; g.order.disableDlgSe = 0;
				g.autoPlay.fm_voice_start = g.autoPlay.fm_voice_auto = 0;
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

			if(g.autoPlay.scode) {
				g.autoPlay.fm_voice_start = *g.pfm_cnt;
				g.autoPlay.fm_voice_auto = g.autoPlay.fm_voice_start +
					_sf.samplesTotal * FPS / _sf.samplesPerSec + g.config.WaitTimeDialogWithVoice * FPS / TIME_UNITS_PER_SEC;
			}

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
			if(read_left == 0 && read_right == 0) {
				EventSet(_evh_sound, StopPlayRequest);
				_sf.Close();
				EventWait(_evh_read, StopPlayDone, EVENT_WAITAND | EVENT_WAITCLEAR);
			}
		} else if(req & ReadSecondHalfRequest) {
			read_right = _sf.Read(_soundbuff + SMPNUM_HALFBUFF, SMPNUM_HALFBUFF);
			if(read_right == 0 && read_left == 0) {
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

	_th_sound = sceKernelCreateThread("soundThread", soundThread, 0x10, 0x1000, 0, 0);
	_th_read = sceKernelCreateThread("readThread", readThread, 0x10, 0x4000, 0, 0);

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

bool PlaySound(struct Play* play) {
	LOG("File: %s, voice_id = %09d, volume = 0x%04X, init = 0x%08X",
			play->filename ? play->filename : "NULL",
			play->voice_id,
			play->volume,
			(unsigned)play->initSf);

	if((!play->filename && !play->voice_id) || !play->initSf) return false;

	MutexLock(_mt);
	_volume = play->volume;
	_initSfCall = (InitSfCall)play->initSf;
	if(play->filename) {
		for(unsigned i = 0; i < sizeof(_fileName) && play->filename[i]; i++) {
			_fileName[i] = play->filename[i];
		}
		_fileName[sizeof(_fileName) - 1] = '\0';
		_voice_id = 0;
	} else {
		_voice_id = play->voice_id;
	}
	MutexUnlock(_mt);

	g.order.disableDududu = g.order.disableDlgSe = 1;
	if(g.autoPlay.scode) {
		g.autoPlay.fm_voice_start = g.autoPlay.fm_voice_auto = 0xFFFFFFFF;
	}

	EventClear(_evh_read, StopSoundRequest);
	EventSet(_evh_read, NewSoundRequest);
	return true;
}

bool StopSound() {
	g.order.disableDududu = g.order.disableDlgSe = 0;

	if(g.config.SkipVoice) {
		EventSet(_evh_read, StopSoundRequest);
	}
	LOG("Stop is called.");
	return true;
}

bool SetVolume(int volume) {
	_volume = volume;
	return true;
}
