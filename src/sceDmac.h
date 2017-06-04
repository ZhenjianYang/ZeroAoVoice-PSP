
#ifndef __SCE_DMAC_H__
#define __SCE_DMAC_H__

#include <pspkerneltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int sceDmacMemcpy(void* dst, const void* src, unsigned int size);
int sceDmacTryMemcpy(void* dst, const void* src, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif
