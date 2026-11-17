/* SPDX-License-Identifier: GPL-2.0 */
/* Linux 5.10 compatibility additions for the backported GED 6.12 driver. */
#ifndef _GED_COMPAT_510_H_
#define _GED_COMPAT_510_H_

#include <linux/types.h>
#include <gpufreq_v2.h>

/* 6.12 gpufreq additions: approximate with the current frequency. */
static inline unsigned int gpufreq_get_cur_out_freq(enum gpufreq_target target)
{
	return gpufreq_get_cur_freq(target);
}

static inline unsigned int gpufreq_get_cur_avg_freq(enum gpufreq_target target)
{
	return gpufreq_get_cur_freq(target);
}

/* 6.12 workload-mode hints are not exposed by the 5.10 gpufreq API. */
static inline bool mtk_dvfs_workload_mode(int workload_mode)
{
	(void)workload_mode;
	return false;
}

static inline bool mtk_get_dvfs_workload_mode(unsigned int *workload_mode)
{
	if (workload_mode)
		*workload_mode = 0;
	return false;
}

/* 6.12 gpufreq additions. */
#ifndef DVFS_FIX_OPP
#define DVFS_FIX_OPP       (1u << 2)
#define DVFS_FIX_FREQ_VOLT (1u << 3)
#endif

/* Fold 6.12 GED commit types onto the closest 5.10 values. */
#ifndef MTK_GPU_DVFS_TYPE_IDLE
#define MTK_GPU_DVFS_TYPE_IDLE         MTK_GPU_DVFS_TYPE_FALLBACK
#endif
#ifndef MTK_GPU_DVFS_TYPE_SKIPFALLBACK
#define MTK_GPU_DVFS_TYPE_SKIPFALLBACK MTK_GPU_DVFS_TYPE_FALLBACK
#endif

/* 6.12 mfgsys config API: map to the 5.10 no-op implementation. */
#ifndef FEAT_ENABLE
#define FEAT_ENABLE 1
#endif
#ifndef CONFIG_AXI_MON
#define CONFIG_AXI_MON 32
#endif
static inline int gpufreq_set_mfgsys_config(int config, int feature)
{
	(void)config;
	(void)feature;
	return 0;
}

/* Map the 6.12 POWERHAL limiter onto the closest 5.10 limiter. */
#ifndef LIMIT_POWERHAL
#define LIMIT_POWERHAL LIMIT_PBM
#endif

/* Implementations supplied by ged_compat_510.c or gpueb612. */
unsigned int gpufreq_wrapper_ready(void);
int gpufreq_get_cur_temperature(void);
void MTKGPUQoS_mode_ratio(int mode);
void set_gpu_pre_throttle(int value, int level);
void set_gpu_pre_throttle_opp(int opp, int level);
int mtk_ipi_send_compl_to_gpueb(int ipi_id, int opt, void *data, int len,
				unsigned long timeout);

#endif /* _GED_COMPAT_510_H_ */
