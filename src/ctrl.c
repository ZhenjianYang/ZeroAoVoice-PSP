#include "ctrl.h"

#include "global.h"
#include "draw.h"
#include "player.h"
#include "message.h"

static const char* Msg_AutoPlay_Switches[] = { Msg_AutoPlay_Off, Msg_AutoPlay_VoiceOnly, Msg_AutoPlay_All};

void SwitchAutoPlay() {
	g.config.AutoPlay++;
	if(g.config.AutoPlay > Max_AutoPlay) g.config.AutoPlay = 0;

	char buff[48];
	int i;
	for(i = 0; Msg_AutoPlay[i]; i++) {
		buff[i] = Msg_AutoPlay[i];
	}
	const char* sw = Msg_AutoPlay_Switches[g.config.AutoPlay];
	for(int j = 0; sw[j]; j++, i++) {
		buff[i] = sw[j];
	}

	buff[i] = '\0';
	Info info = { Msg_Time_Normal, buff };
	AddInfo(&info);

	SaveConfig(&g.config, PATH_CONFIG);
}

void AddVolume(int add) {
	g.config.Volume += add;
	if(g.config.Volume > Max_Volume) g.config.Volume = Max_Volume;
	else if(g.config.Volume < 0) g.config.Volume = 0;

	char buff[48];
	int i;
	for(i = 0; Msg_Volume[i]; i++) {
		buff[i] = Msg_Volume[i];
	}
	if(g.config.Volume >= 100) buff[i++] = (g.config.Volume / 100) + '0';
	if(g.config.Volume >= 10) buff[i++] = (g.config.Volume % 100 / 10) + '0';
	buff[i++] = (g.config.Volume % 10) + '0';
	buff[i] = '\0';

	Info info = { Msg_Time_Normal, buff };
	AddInfo(&info);

	SetVolume(g.config.Volume);

	SaveConfig(&g.config, PATH_CONFIG);
}


