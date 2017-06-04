#include "hook2.h"
#include "asm_offset_define.h"
#include "global.h"
#include "player.h"

#define BREAK "\n"

#define MAX_VOICEID_LEN	10
#define VOICE_FILE_PREFIX "/v"
static char _buff_voicefile[64];
static int _len_prefix;

#define _S(V) #V
#define S(V) _S(V)

static Play _play;

void InitHook2() {
	for(_len_prefix = 0; g.path[_len_prefix]; _len_prefix++) {
			_buff_voicefile[_len_prefix] = g.path[_len_prefix];
		}
	for(int i = 0; g.voice_ext[i]; i++, _len_prefix++) {
		_buff_voicefile[_len_prefix] = g.voice_ext[i];
	}
	for(unsigned i = 0; i < sizeof(VOICE_FILE_PREFIX); i++) {
		_buff_voicefile[_len_prefix + i] = VOICE_FILE_PREFIX[i];
	}
	_len_prefix += sizeof(VOICE_FILE_PREFIX) - 1;

	_play.filename = _buff_voicefile;
	_play.volume = g.config.Volume;
	_play.initSf = g.initSf;
}

void H_voice(const char* p) {
	if(*p != 'v') return;

	const char* q = p - 1;
	while(q >= p - MAX_VOICEID_LEN - 1 && *q != '#') q--;
	if(*q != '#' || p - q <= 1) return;

	q++;
	char* t = _buff_voicefile + _len_prefix;
	while(q < p) *t++ = *q++;
	*t++ = '.';
	for(unsigned i = 0; i < sizeof(g.voice_ext); i++) *t++ = g.voice_ext[i];

	PlaySound(&_play);
}

HookAddr ha;

__asm__(
	".global h_voice"									BREAK
	"h_voice:"											BREAK
	"    li   $a0, 'v'"									BREAK
	"    bne  $a0, $a1, voice_return"					BREAK
	"    add  $a0, $s0, $zero"							BREAK
	"    j    H_voice"									BREAK
	"voice_return:"										BREAK
	"    jr   $ra"										BREAK
);

__asm__(
	".global h_dududu"											BREAK
	"h_dududu:"													BREAK
	"    lui     $t4, %hi(g)"									BREAK
	"    addiu   $t4, %lo(g)"									BREAK
	"    lw      $t5, "  S(OFF_cfg_disdu)  "($t4)"				BREAK
	"    beq     $t5, $zero, dududu_not"						BREAK
	"    lw      $t5, "  S(OFF_od_disdu)  "($t4)"				BREAK
	"    beq     $t5, $zero, dududu_not"						BREAK

	"    li      $t0, 0"										BREAK
	"    jr      $ra"											BREAK

	"dududu_not:"
	"    li      $t0, 0x64"										BREAK
	"    jr      $ra"											BREAK
);

__asm__(
	".global h_dlgse"											BREAK
	"h_dlgse:"													BREAK
	"    addiu   $sp, $sp, -4"					       			BREAK
	"    sw      $ra, 0($sp)"					        		BREAK

	"    li      $t0, 0x64"										BREAK
	"    lui     $t4, %hi(g)"									BREAK
	"    addiu   $t4, %lo(g)"									BREAK
	"    lw      $t5, "  S(OFF_cfg_disdlg)  "($t4)"				BREAK
	"    beq     $t5, $zero, call_se"							BREAK
	"    lw      $t5, "  S(OFF_od_disdlg)  "($t4)"				BREAK
	"    beq     $t5, $zero, call_se"							BREAK
	"    li      $t0, 0"										BREAK

	"call_se:"													BREAK
	"    lw      $t5, "  S(OFF_sub_se)  "($t4)"					BREAK
	"    jalr    $t5"											BREAK

	"    jal     StopSound"										BREAK

	"    lw      $ra, 0($sp)"									BREAK
	"    addiu   $sp, $sp, 4"									BREAK
	"    jr      $ra"											BREAK
);

__asm__(
	".global h_scode"											BREAK
	"h_scode:"													BREAK
	"    addiu   $sp, $sp, -8"									BREAK
	"    sw      $t4, 4($sp)"									BREAK
	"    sw      $v0, 0($sp)"									BREAK

	"    lui     $t4, %hi(g)"									BREAK
	"    addiu   $t4, %lo(g)"									BREAK

	"    li      $v0, "  S(scode_TEXT)							BREAK
	"    beq     $v0, $v1, scode_record"						BREAK
	"    li      $v0, "  S(scode_SAY)							BREAK
	"    beq     $v0, $v1, scode_record"						BREAK
	"    li      $v0, "  S(scode_TALK)							BREAK
	"    beq     $v0, $v1, scode_record"						BREAK

	"    li      $v0, "  S(scode_MENU)							BREAK
	"    beq     $v0, $v1, scode_clear"							BREAK
	"    li      $v0, "  S(scode_MENUEND)						BREAK
	"    beq     $v0, $v1, scode_clear"							BREAK

	"scode_ret:"												BREAK
	"    lw      $t4, 4($sp)"									BREAK
	"    lw      $v0, 0($sp)"									BREAK
	"    sll     $v1, 3"										BREAK
	"    addu    $v1, $v1, $v0"									BREAK
	"    addiu   $sp, $sp, 8"									BREAK
	"    jr      $ra"											BREAK

	"scode_clear:"												BREAK
	"    sw      $zero, " S(OFF_ap_scod)  "($t4)"				BREAK
	"    j       scode_ret"										BREAK

	"scode_record:"												BREAK
	"    sw      $v1, " S(OFF_ap_scod)  "($t4)"					BREAK
	"    sw      $zero, " S(OFF_od_disdu)  "($t4)"				BREAK
	"    sw      $zero, " S(OFF_od_disdlg)  "($t4)"				BREAK
	"    sw      $zero, " S(OFF_ap_tend)  "($t4)"				BREAK
	"    sw      $zero, " S(OFF_ap_tcnt)  "($t4)"				BREAK
	"    sw      $zero, " S(OFF_ap_fmvs)  "($t4)"				BREAK
	"    sw      $zero, " S(OFF_ap_fms)  "($t4)"				BREAK
	"    j       scode_ret"										BREAK
);

__asm__  (
	".global h_count"											BREAK
	"h_count:"													BREAK
	"    addiu   $sp, $sp, -8"					       			BREAK
	"    sw      $t0, 4($sp)"					        		BREAK
	"    sw      $t4, 0($sp)"					        		BREAK

	"    lui     $t4, %hi(g)"									BREAK
	"    addiu   $t4, %lo(g)"									BREAK

	"    beq     $v0, $zero, code_ge20"							BREAK

	"    lw      $v0, "  S(OFF_ap_scod)  "($t4)"				BREAK
	"    beq     $v0, $zero, count_ret"							BREAK

	"    li      $v0, 2"						        		BREAK
	"    bne     $v0, $v1, count_ret"	     			   		BREAK

	"    li      $v0, 1"										BREAK
	"    sw      $v0, "  S(OFF_ap_tend)  "($t4)"				BREAK

	"    lw      $v0, "  S(OFF_cfg_wtpch)  "($t4)"				BREAK
	"    lw      $t0, "  S(OFF_ap_tcnt)  "($t4)"				BREAK
	"    multu   $v0, $t0"					        			BREAK
	"    mflo    $v0"					        				BREAK
	"    lw      $t0, "  S(OFF_cfg_wtd)  "($t4)"				BREAK
	"    addu    $v0, $v0, $t0"									BREAK
	"    li      $t0, "  S(FPS)									BREAK
	"    multu   $v0, $t0"					        			BREAK
	"    mflo    $v0"					        				BREAK
	"    li      $t0, "  S(TIME_UNITS_PER_SEC)					BREAK
	"    divu    $v0, $t0"					        			BREAK
	"    mflo    $v0"					        				BREAK
	"    lw      $t0, "  S(OFF_ap_fms)  "($t4)"					BREAK
	"    addu    $v0, $v0, $t0"									BREAK
	"    sw      $v0, "  S(OFF_ap_fma)  "($t4)"					BREAK

	"count_ret:"												BREAK
	"    lw      $t0, 4($sp)"					        		BREAK
	"    lw      $t4, 0($sp)"					        		BREAK
	"    li      $v0, 0x23"						        		BREAK
	"    addiu   $sp, $sp, 8"					       			BREAK
	"    jr      $ra"											BREAK

	"code_ge20:"												BREAK
	"    lw      $v0, "  S(OFF_ap_scod)  "($t4)"				BREAK
	"    beq     $v0, $zero, count_ret_ge20"					BREAK

	"    li      $v0, '#'"                           		    BREAK
	"    beq     $v1, $v0, count_ret_ge20"						BREAK

	"    lw      $v0, "  S(OFF_ap_tcnt)  "($t4)"				BREAK
	"    bne     $v0, $zero, code_notfirst"                     BREAK

	"    lw      $t0, "  S(OFF_pfm_cnt)  "($t4)"				BREAK
	"    lw      $t0, 0($t0)"									BREAK
	"    sw      $t0, "  S(OFF_ap_fms)  "($t4)"					BREAK

	"code_notfirst:"											BREAK
	"    addiu   $v0, $v0, 1"					       			BREAK
	"    sw      $v0, "  S(OFF_ap_tcnt)  "($t4)"				BREAK

	"count_ret_ge20:"											BREAK
	"    lw      $ra, "  S(OFF_sub_ge)  "($t4)"					BREAK
	"    j       count_ret"										BREAK
);

__asm__(
	".global h_ctrl"									BREAK
	"h_ctrl:"											BREAK
	"    lbu     $v1, 9($sp)"							BREAK

	"    lui     $t4, %hi(g)"							BREAK
	"    addiu   $t4, %lo(g)"							BREAK

	"    lw      $t0, "  S(OFF_ap_tend)  "($t4)"		BREAK
	"    beq     $t0, $zero, ctrl_ret"					BREAK

	"    lw      $t0, "  S(OFF_pfm_cnt)  "($t4)"		BREAK
	"    lw      $t0, 0($t0)"							BREAK

	"    lw      $t1, "  S(OFF_ap_fmva)  "($t4)"		BREAK
	"    blt	 $t0, $t1, ctrl_ret"					BREAK

	"    lw      $t1, "  S(OFF_ap_fma)  "($t4)"			BREAK
	"    blt	 $t0, $t1, ctrl_ret"					BREAK

	"    lw      $t1, "  S(OFF_ap_fmvs)  "($t4)"		BREAK
	"    lw      $t0, "  S(OFF_cfg_aup)  "($t4)"		BREAK

	"    sw      $zero, " S(OFF_ap_tend)  "($t4)"		BREAK
	"    sw      $zero, " S(OFF_ap_tcnt)  "($t4)"		BREAK
	"    sw      $zero, " S(OFF_ap_fmvs)  "($t4)"		BREAK
	"    sw      $zero, " S(OFF_ap_fms)  "($t4)"		BREAK

	"    beq	 $t1, $zero, no_voice" 					BREAK
	"    beq	 $t0, $zero, ctrl_ret" 					BREAK
	"    j       set_autoplay"							BREAK

	"no_voice:" 										BREAK
	"    li      $t1, "  S(AutoPlay_All)  				BREAK
	"    bne	 $t0, $t1, ctrl_ret" 					BREAK

	"set_autoplay:" 									BREAK
	"    ori     $a2, $a2, " S(CTRL_OK) 				BREAK
	"    sw      $a2, 4($sp)" 							BREAK

	"ctrl_ret:"
	"    jr   $ra"										BREAK
);

__asm__(
	".global h_pfm_cnt"									BREAK
	"h_pfm_cnt:"										BREAK
	"    lui     $t4, %hi(g)"							BREAK
	"    addiu   $t4, %lo(g)"							BREAK
	"    lw      $a1, "  S(OFF_off_pfm_cnt)  "($t4)"	BREAK
	"    addu    $a1, $a1, $a0"							BREAK
	"    sw      $a1, "  S(OFF_pfm_cnt)  "($t4)"		BREAK
	"    li      $a1, 0"								BREAK
	"    jr      $ra"									BREAK
);

__asm__(
	".global h_codeA"									BREAK
	"h_codeA:"											BREAK
	"    lui     $s0, %hi(g)"							BREAK
	"    addiu   $s0, %lo(g)"							BREAK

	"    sw      $zero, " S(OFF_ap_scod)  "($s0)"		BREAK
	"    sw      $zero, " S(OFF_ap_tend)  "($s0)"		BREAK
	"    sw      $zero, " S(OFF_ap_tcnt)  "($s0)"		BREAK
	"    sw      $zero, " S(OFF_ap_fmvs)  "($s0)"		BREAK
	"    sw      $zero, " S(OFF_ap_fms)  "($s0)"		BREAK

	"    addu    $v0, $v0, $v1"							BREAK
	"    jr      $ra"									BREAK
);
