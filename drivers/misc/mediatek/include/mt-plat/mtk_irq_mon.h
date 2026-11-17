/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 */

#ifndef __MTK_IRQ_MON__
#define __MTK_IRQ_MON__

#include <linux/version.h>

#if IS_ENABLED(CONFIG_MTK_IRQ_MONITOR)
extern void mt_aee_dump_irq_info(void);
#else
#define mt_aee_dump_irq_info() do {} while (0)
/* The R54 CSF driver uses the 6.12 IRQ-monitor AEE hook. Linux 5.10's
 * MediaTek IRQ monitor does not expose it, and the hook is optional.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0))
static inline void irq_mon_aee_period_set(unsigned int irq, unsigned int period)
{
	(void)irq;
	(void)period;
}
#endif
#endif

#endif

