#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspthreadman.h>

#include <stdio.h>
#include "player.h"
#include "log.h"
#include "mutex.h"
#include "config.h"
#include "global.h"

#include "sf_wav.h"
#define TEST_WAV "ms0:/za_voice/test.wav"

#include "sf_ogg.h"
#define TEST_OGG "ms0:/za_voice/test.ogg"

#include "sf_at3.h"
#define TEST_AT3 "ms0:/za_voice/test.at3"

Global g;

typedef struct Test {
	InitSfCall initCall;
	const char fileName[64];
} Test;
static Test tests[] = {
	{ &InitAt3, TEST_AT3 },
	{ &InitOgg, TEST_OGG },
	{ &InitWAV, TEST_WAV },
};
#define Num_tests (sizeof(tests) / sizeof(*tests))

PSP_MODULE_INFO("KPTEST", 0, 1, 1);

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();

	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

#define printf	pspDebugScreenPrintf

typedef struct WAVEFORMAT {
	u16 wFormatTag;        /* format type */
	u16 nChannels;         /* number of channels (i.e. mono, stereo, etc.) */
	u32 nSamplesPerSec;    /* sample rate */
	u32 nAvgBytesPerSec;   /* for buffer estimation */
	u16 nBlockAlign;       /* block size of data */
	u16 wBitsPerSample;    /* number of bits per sample of mono data */
} WAVEFORMAT;

typedef struct WAVHeadOut
{
	u32 tag_RIFF;
	u32 size;
	u32 tag_WAVE;
	u32 tag_fmt;
	u32 size_WAVEFORMAT;
	WAVEFORMAT waveFormat;
	u32 tag_data;
	s32 size_data;
} WAVHeadOut;
static const u32 tag_RIFF = 0x46464952;
static const u32 tag_WAVE = 0x45564157;
static const u32 tag_fmt  = 0x20746D66;
static const u32 tag_data = 0x61746164;

MutexHandle mt;

int BThread(SceSize args, void *argp) {
	MutexLock(mt);
	LOG("BThread start.");
	for(int i = 0; i < 10; i ++) {
		LOG("[BThread] : %d", i);
		sceKernelDelayThread(200 * 1000);
	}
	LOG("BThread end.");
	MutexUnlock(mt);
	return sceKernelExitDeleteThread(0);
}

int AThread(SceSize args, void *argp) {
	MutexLock(mt);
	LOG("AThread start.");
	for(int i = 0; i < 10; i ++) {
		LOG("[AThread] : %d", i);
		sceKernelDelayThread(200 * 1000);
	}
	LOG("AThread end.");
	MutexUnlock(mt);
	return sceKernelExitDeleteThread(0);
}

static unsigned count = 0;
Play play;
void _call() {
	for(unsigned i = 0; i < Num_tests; i++) {
		if(tests[count % Num_tests].initCall) {
			play.filename = tests[count % Num_tests].fileName;
			play.volume = 100;
			play.initSf = tests[count % Num_tests].initCall;
			PlaySound(&play);
			count++;
			break;
		}
	}
}

#define BUFF_SIZE 24000
SampleType buff[24000][2];
Config sconfig;
Config* config = &sconfig;
int main(void)
{
	pspDebugScreenInit();
	SetupCallbacks();
	
	pspDebugScreenPrintf("Start.\n");
	LOG("Start.");
	
	LOG("Config test.");
	LoadConfig(config, "ms0:/PSP/za_voice/config.ini");
	LOG("Config info:\n"
		"    Volume                   = %d\n"
		"    AutoPlay                 = %d\n"
		"    WaitTimePerChar          = %d\n"
		"    WaitTimeDialog           = %d\n"
		"    WaitTimeDialogWithVoice  = %d\n"
		"    SkipVoice                = %d\n"
		"    DisableDialogTextSE      = %d\n"
		"    DisableDialogSwitchSE    = %d",
		config->Volume,
		config->AutoPlay,
		config->WaitTimePerChar,
		config->WaitTimeDialog,
		config->WaitTimeDialogWithVoice,
		config->SkipVoice,
		config->DisableDialogTextSE,
		config->DisableDialogSwitchSE
	);
	SaveConfig(config, "ms0:/PSP/za_voice/config_new.ini");
	LOG("Config test end.");
	sceKernelDelayThread(5 * 1000 * 1000);


	LOG("Mutex test...");
	mt = MutexCreate();
	LOG("Mutex Creted.");

	int thida = sceKernelCreateThread("update_thread", AThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if (thida >= 0) sceKernelStartThread(thida, 0, 0);
	int thidb = sceKernelCreateThread("update_thread", BThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if (thidb >= 0) sceKernelStartThread(thidb, 0, 0);

	sceKernelDelayThread(5 * 1000 * 1000);

	MutexDelete(mt);
	mt = NULL;
	LOG("Mutex Deleted.");
	LOG("Mutex test end.");

	int rst = 0;
	SoundFile sf;

	Test* test = tests + 2;

	LOG("InitSoundFile = 0x%08X.", (unsigned)test->initCall);
	test->initCall(&sf);

	LOG("open %s...", test->fileName);
	rst = sf.Open(test->fileName);
	LOG("Finished. %d", rst);
	LOG("Sound File Info:\n"
		"    FormatTag = 0x%04X\n"
		"    Channels  = %d\n"
		"    SamplesPerSec = %d\n"
		"    AvgBytesPerSec = %d\n"
		"    BlockAlign = %d\n"
		"    SamplesTotal =  %d",
		sf.formatTag,
		sf.channels,
		sf.samplesPerSec,
		sf.avgBytesPerSec,
		sf.blockAlign,
		sf.samplesTotal
	);
	WAVEFORMAT waveFormat;
	waveFormat.wFormatTag = 1;
	waveFormat.nChannels = 2;
	waveFormat.nSamplesPerSec = sf.samplesPerSec;
	waveFormat.nBlockAlign = 2 * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.wBitsPerSample = 16;
	WAVHeadOut head = {
		tag_RIFF,
		sizeof(WAVHeadOut) + sf.samplesTotal * waveFormat.nBlockAlign - 8,
		tag_WAVE,
		tag_fmt,
		sizeof(WAVEFORMAT),
		waveFormat,
		tag_data,
		sf.samplesTotal * waveFormat.nBlockAlign
	};
	LOG("read...");
	int remain = sf.samplesTotal;
	FILE* fout = fopen("ms0:/za_voice/test_out.wav", "wb");
	fwrite(&head, sizeof(head), 1, fout);
	while(remain > 0) {
		int request = remain > BUFF_SIZE ? BUFF_SIZE : remain;
		rst = sf.Read(buff, request);
		//LOG("rst : %d", rst);
		if(rst <= 0) break;
		fwrite(buff, sizeof(*buff), rst, fout);
		remain -= rst;
	}
	fclose(fout);
	LOG("finished: %d", rst);
	LOG("close...");
	sf.Close();
	LOG("finished.");

	SceCtrlData pad;
	int btn_old = 0;

	SceUID mid = sceKernelLoadModule("flash0:/kd/libatrac3plus.prx", 0, 0);
	if(mid >= 0) {
		int outp;
		mid = sceKernelStartModule(mid, 0, 0, &outp, 0);
		LOG("libatrac3plus loaded. 0x%08X", mid);
	} else {
		LOG("Load libatrac3plus failed.");
	}
	InitPlayer(NULL);
	LOG("InitSound Finished.");

	while(1) {
		sceCtrlPeekBufferPositive(&pad,1);
		if((pad.Buttons & PSP_CTRL_RTRIGGER) && !(btn_old & PSP_CTRL_RTRIGGER))
		{
			LOG("pressed.");
			_call();
		}
		btn_old = pad.Buttons;
		sceKernelDelayThread(100 * 1000);
	}
	
	return 0;
}

