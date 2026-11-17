/* SPDX-License-Identifier: GPL-2.0 */
/* GED-local view of the 5.10 gpu_bm interface (struct + prototypes) */
#ifndef __GPU_BM_H__
#define __GPU_BM_H__

#include <linux/types.h>

struct v1_data {
	unsigned int version;
	unsigned int ctx;
	unsigned int frame;
	unsigned int job;
	unsigned int freq;
};

void MTKGPUQoS_mode(int seg_flag);
void MTKGPUQoS_setup(struct v1_data *v1, phys_addr_t phyaddr, size_t size);
int MTKGPUQoS_is_inited(void);

#endif
