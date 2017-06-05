#pragma once


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void* _binary_src_font_dat_start;
extern void* _binary_src_font_dat_end;
extern int _binary_src_font_dat_size;

#define font_dat_start _binary_src_font_dat_start
#define font_dat_end _binary_src_font_dat_end
#define font_dat_size _binary_src_font_dat_size

#ifdef __cplusplus
}
#endif

