// SPDX-License-Identifier: GPL-2.0
/* 5.10 stubs for interfaces GED 6.12 expects from other 6.12 modules */
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/soc/mediatek/mtk_tinysys_ipi.h>
#include <gpueb_ipi.h>

int gpufreq_get_cur_temperature(void)
{
	return 0;
}

unsigned int gpufreq_wrapper_ready(void)
{
	return 1;
}

void MTKGPUQoS_mode_ratio(int mode)
{
	(void)mode;
}

/* 6.12 wraps this in gpueb.ko together with a GHPM power sequence. 5.10's
 * gpueb.ko does not export the wrapper and has no GHPM control, but the GPUEB
 * coprocessor is already up (its watchdog is being kicked) and the generic
 * tinysys IPI transport is available, so send on the GPUEB channel directly.
 */
int mtk_ipi_send_compl_to_gpueb(int ipi_id, int opt, void *data, int len,
				unsigned long timeout)
{
	void *ipidev = get_gpueb_ipidev();
	int ret;

	if (!ipidev)
		return -ENODEV;

	ret = mtk_ipi_send_compl((struct mtk_ipi_device *)ipidev, ipi_id,
				 opt, data, len, timeout);
	if (ret != IPI_ACTION_DONE)
		return ret;

	return 0;
}

void set_gpu_pre_throttle(int value, int level)
{
	(void)value; (void)level;
}

void set_gpu_pre_throttle_opp(int opp, int level)
{
	(void)opp; (void)level;
}

/* 6.12 exports these as pointers filled in by GED itself */
void (*mtk_dvfs_workload_mode_fp)(int i32WorkloadMode);
int (*mtk_get_dvfs_workload_mode_fp)(void);
