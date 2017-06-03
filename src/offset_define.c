#include "offset_define.h"
#include "global.h"

DEFINE_OFFSET(Global, config);
DEFINE_OFFSET_WName(cfg_vol, Global, config.Volume);
DEFINE_OFFSET_WName(cfg_aup, Global, config.AutoPlay);
DEFINE_OFFSET_WName(cfg_wtpch, Global, config.WaitTimePerChar);
DEFINE_OFFSET_WName(cfg_wtd, Global, config.WaitTimeDialog);
DEFINE_OFFSET_WName(cfg_wtdv, Global, config.WaitTimeDialogWithVoice);
DEFINE_OFFSET_WName(cfg_sv, Global, config.SkipVoice);
DEFINE_OFFSET_WName(cfg_disdu, Global, config.DisableDialogTextSE);
DEFINE_OFFSET_WName(cfg_disdlg, Global, config.DisableDialogSwitchSE);
DEFINE_OFFSET_WName(cfg_disori, Global, config.DisableOriginalVoice);

DEFINE_OFFSET(Global, order);
DEFINE_OFFSET_WName(od_disdu, Global, order.disableDududu);
DEFINE_OFFSET_WName(od_disdlg, Global, order.disableDlgSe);


DEFINE_OFFSET(Global, autoPlay);
DEFINE_OFFSET_WName(ap_scod, Global, autoPlay.scode);
DEFINE_OFFSET_WName(ap_tend, Global, autoPlay.text_end);
DEFINE_OFFSET_WName(ap_tcnt, Global, autoPlay.text_cnt);
DEFINE_OFFSET_WName(ap_fms, Global, autoPlay.fm_start);
DEFINE_OFFSET_WName(ap_fma, Global, autoPlay.fm_auto);
DEFINE_OFFSET_WName(ap_fmvs, Global, autoPlay.fm_voice_start);
DEFINE_OFFSET_WName(ap_fmva, Global, autoPlay.fm_voice_auto);


DEFINE_OFFSET(Global, ha);
DEFINE_OFFSET_WName(sub_se, Global, ha.addr_sub_se);
DEFINE_OFFSET_WName(sub_less, Global, ha.addr_less);

DEFINE_OFFSET(Global, pfm_cnt);
DEFINE_OFFSET(Global, off_pfm_cnt);


