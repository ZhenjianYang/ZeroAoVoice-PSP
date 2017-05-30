#include "hook_asm.h"

#define BREAK "\n"

__asm__(
	".global h_voice"									BREAK
	"h_voice:"											BREAK
	"    li   $a0, 'v' "								BREAK
	"    bne  $a0, $a1, voice_return"					BREAK
	"    add  $a0, $s0, $zero"							BREAK
	"    j    H_voice"									BREAK
	"voice_return:"										BREAK
	"    jr   $ra"										BREAK
);
