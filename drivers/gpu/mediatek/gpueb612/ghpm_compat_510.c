// SPDX-License-Identifier: GPL-2.0
/*
 * Stubs for the GHPM (GPU Hardware Power Manager) interface.
 *
 * The upstream tree implements GHPM only for mt6991/mt6993.  MT6895 predates
 * that hardware and uses the legacy power sequence, so the whole GHPM layer is
 * left out of this build.  gpueb_init() still calls ghpm_wrapper_init(), hence
 * these no-ops.
 */
#include <linux/types.h>
#include <linux/platform_device.h>
#include "include/ghpm_wrapper.h"

unsigned int g_ghpm_support;
EXPORT_SYMBOL(g_ghpm_support);

void ghpm_wrapper_init(struct platform_device *pdev)
{
	/* no GHPM on this platform */
	(void)pdev;
	g_ghpm_support = 0;
}

int ghpm_ctrl(enum ghpm_state power, enum mfg0_off_state off_state)
{
	(void)power;
	(void)off_state;
	return GHPM_ERR;
}

/*
 * With no GHPM the upstream implementation returns success immediately, so
 * the IPI send path behaves exactly as on a GPUEB-always-on platform: there is
 * no power sequence to run, just the IPI itself.
 */
int gpueb_ctrl(enum ghpm_state power,
	enum mfg0_off_state off_state, enum gpueb_low_power_event event)
{
	(void)power;
	(void)off_state;
	(void)event;
	return 0;
}
EXPORT_SYMBOL(gpueb_ctrl);
