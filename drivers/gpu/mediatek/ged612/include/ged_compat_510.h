/* SPDX-License-Identifier: GPL-2.0 */
/* 5.10 compatibility shims for GED 6.12 */
#ifndef _GED_COMPAT_510_H_
#define _GED_COMPAT_510_H_

#include <linux/types.h>
#include <gpufreq_v2.h>

/* 6.12 gpufreq additions: approximate with the current frequency */
static inline unsigned int gpufreq_get_cur_out_freq(enum gpufreq_target target)
{
	return gpufreq_get_cur_freq(target);
}
static inline unsigned int gpufreq_get_cur_avg_freq(enum gpufreq_target target)
{
	return gpufreq_get_cur_freq(target);
}

/* 6.12 workload-mode hints: not supported on 5.10 */
static inline bool mtk_dvfs_workload_mode(int i32WorkloadMode)
{
	(void)i32WorkloadMode;
	return false;
}
static inline bool mtk_get_dvfs_workload_mode(unsigned int *pui32WorkloadMode)
{
	if (pui32WorkloadMode)
		*pui32WorkloadMode = 0;
	return false;
}

/* 6.12 gpufreq_v2 additions */
#ifndef DVFS_FIX_OPP
#define DVFS_FIX_OPP       (1u << 2)
#define DVFS_FIX_FREQ_VOLT (1u << 3)
#endif

/* 6.12 GED commit types: fold onto the closest 5.10 value */
#ifndef MTK_GPU_DVFS_TYPE_IDLE
#define MTK_GPU_DVFS_TYPE_IDLE         MTK_GPU_DVFS_TYPE_FALLBACK
#endif
#ifndef MTK_GPU_DVFS_TYPE_SKIPFALLBACK
#define MTK_GPU_DVFS_TYPE_SKIPFALLBACK MTK_GPU_DVFS_TYPE_FALLBACK
#endif

/* gpufreq asks GED for its commit info; nothing reads these on 5.10 */
static unsigned long (*ged_get_last_commit_idx_fp)(void);
static unsigned long (*ged_get_last_commit_top_idx_fp)(void);
static unsigned long (*ged_get_last_commit_stack_idx_fp)(void);
static void (*ged_notify_gpu_fix_opp_fp)(int);
static void (*ged_notify_gpu_fix_freq_volt_fp)(unsigned int, unsigned int);

/* 6.12 mfgsys config API: not present on 5.10 */
#ifndef FEAT_ENABLE
#define FEAT_ENABLE 1
#endif
#ifndef CONFIG_AXI_MON
#define CONFIG_AXI_MON 32
#endif
static inline int gpufreq_set_mfgsys_config(int config, int feature)
{
	(void)config; (void)feature;
	return 0;
}

/* 6.12 added a POWERHAL limiter; map it onto the closest 5.10 limiter */
#ifndef LIMIT_POWERHAL
#define LIMIT_POWERHAL LIMIT_PBM
#endif

/* Compatibility implementations supplied by ged_compat_510.c. */
unsigned int gpufreq_wrapper_ready(void);
void MTKGPUQoS_mode_ratio(int mode);
void set_gpu_pre_throttle(int value, int level);
void set_gpu_pre_throttle_opp(int opp, int level);
int mtk_ipi_send_compl_to_gpueb(int ipi_id, int opt, void *data, int len,
				unsigned long timeout);

#endif
