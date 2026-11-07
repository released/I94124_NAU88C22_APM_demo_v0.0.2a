#ifndef __COLSONIC_APM_H
#define __COLSONIC_APM_H

#include <stdint.h>

#define __COLSONIC_AEC_NS_ (1)
#define __COLSONIC_Model_ __COLSONIC_AEC_NS_
#define FRM_LEN 320

/* Shared legacy USB baseline: 320 signed mono samples per APM frame. */
uint32_t Colsonic_apm_init(void);
void Colsonic_apm_proc(int16_t mic_sig[], int16_t ls_sig[], int16_t proc_out[]);
void Colsonic_apm_release(void);
uint32_t colsonic_setpara_proc(uint32_t *ptr);

/* Legacy conversion APIs; neither application uses these in its active path. */
void Colsonic_up_16kto96k_proc(int16_t *in, int16_t *out);
void Colsonic_down_96kto16k_proc(int16_t *in, int16_t *out);

#endif
