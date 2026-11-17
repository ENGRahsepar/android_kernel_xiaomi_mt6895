/*
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <linux/version.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <mali_kbase.h>

/* mali_kbase_mmap.c
 *
 * This file contains Linux specific implementation of
 * kbase_context_get_unmapped_area() interface.
 */

/**
 * kbase_unmapped_area_topdown() - allocates new areas top-down using vm_unmapped_area
 * @info:              Information about the memory area to allocate.
 * @is_shader_code:    Boolean which denotes whether the allocated area is
 *                     intended for the use by shader core in which case
 *                     special alignment requirements apply.
 * @is_same_4gb_page:  Boolean which indicates whether the allocated area needs
 *                     to reside completely within a 4GB chunk.
 *
 * Return: address of the found gap end (high limit) if area is found;
 *         -ENOMEM if search is unsuccessful
 */
static unsigned long kbase_unmapped_area_topdown(struct vm_unmapped_area_info *info,
		bool is_shader_code, bool is_same_4gb_page)
{
	unsigned long addr;

	info->flags |= VM_UNMAPPED_AREA_TOPDOWN;

	while (1) {
		addr = vm_unmapped_area(info);
		if (IS_ERR_VALUE(addr))
			return addr;

		if (is_shader_code) {
			/* Check for 4GB boundary */
			if (((addr & BASE_MEM_MASK_4GB) == 0) ||
			    (((addr + info->length) & BASE_MEM_MASK_4GB) == 0)) {
				if (addr < info->low_limit + info->length)
					return -ENOMEM;
				info->high_limit = addr;
				continue;
			}
		} else if (is_same_4gb_page) {
			unsigned long start = addr;
			unsigned long end = addr + info->length;
			unsigned long mask = ~((unsigned long)U32_MAX);

			if ((start & mask) != ((end - 1) & mask)) {
				unsigned long boundary = end & mask;
				if (boundary < info->low_limit + info->length)
					return -ENOMEM;
				info->high_limit = boundary;
				continue;
			}
		}

		return addr;
	}
}


#include <linux/sched/mm.h>

static inline unsigned long kbase_get_unmapped_area(struct mm_struct *mm,
		struct file *filp, unsigned long addr, unsigned long len,
		unsigned long pgoff, unsigned long flags)
{
#if (KERNEL_VERSION(6, 11, 0) <= LINUX_VERSION_CODE)
	return mm_get_unmapped_area(mm, filp, addr, len, pgoff, flags);
#else
	return mm->get_unmapped_area(filp, addr, len, pgoff, flags);
#endif
}

/* This function is based on Linux kernel's arch_get_unmapped_area, but
 * simplified slightly. Modifications come from the fact that some values
 * about the memory area are known in advance.
 */
unsigned long kbase_context_get_unmapped_area(struct kbase_context *const kctx,
		const unsigned long addr, const unsigned long len,
		const unsigned long pgoff, const unsigned long flags)
{
	struct mm_struct *mm = current->mm;
	struct vm_unmapped_area_info info;
	unsigned long align_offset = 0;
	unsigned long align_mask = 0;
	unsigned long high_limit = mm->mmap_base;
	unsigned long low_limit = PAGE_SIZE;
	int cpu_va_bits = BITS_PER_LONG;
	int gpu_pc_bits =
	      kctx->kbdev->gpu_props.props.core_props.log2_program_counter_size;
	bool is_shader_code = false;
	bool is_same_4gb_page = false;
	unsigned long ret;

	/* the 'nolock' form is used here:
	 * - the base_pfn of the SAME_VA zone does not change
	 * - in normal use, va_size_pages is constant once the first allocation
	 *   begins
	 *
	 * However, in abnormal use this function could be processing whilst
	 * another new zone is being setup in a different thread (e.g. to
	 * borrow part of the SAME_VA zone). In the worst case, this path may
	 * witness a higher SAME_VA end_pfn than the code setting up the new
	 * zone.
	 *
	 * This is safe because once we reach the main allocation functions,
	 * we'll see the updated SAME_VA end_pfn and will determine that there
	 * is no free region at the address found originally by too large a
	 * same_va_end_addr here, and will fail the allocation gracefully.
	 */
	struct kbase_reg_zone *zone =
		kbase_ctx_reg_zone_get_nolock(kctx, KBASE_REG_ZONE_SAME_VA);
	u64 same_va_end_addr = kbase_reg_zone_end_pfn(zone) << PAGE_SHIFT;

	/* err on fixed address */
	if ((flags & MAP_FIXED) || addr)
		return -EINVAL;

	if (!same_va_end_addr)
		same_va_end_addr = mm->mmap_base;

#if IS_ENABLED(CONFIG_64BIT)
	/* too big? */
	if (len > TASK_SIZE - SZ_2M)
		return -ENOMEM;

	if (!kbase_ctx_flag(kctx, KCTX_COMPAT)) {
		high_limit =
			min_t(unsigned long, mm->mmap_base, same_va_end_addr);

		/* If there's enough (> 33 bits) of GPU VA space, align
		 * to 2MB boundaries.
		 */
		if (kctx->kbdev->gpu_props.mmu.va_bits > 33) {
			if (len >= SZ_2M) {
				align_offset = SZ_2M;
				align_mask = SZ_2M - 1;
			}
		}

		low_limit = SZ_2M;
	} else {
		cpu_va_bits = 32;
	}
#endif /* CONFIG_64BIT */
	if ((PFN_DOWN(BASE_MEM_COOKIE_BASE) <= pgoff) &&
		(PFN_DOWN(BASE_MEM_FIRST_FREE_ADDRESS) > pgoff)) {
			int cookie = pgoff - PFN_DOWN(BASE_MEM_COOKIE_BASE);
			struct kbase_va_region *reg;

			/* Need to hold gpu vm lock when using reg */
			kbase_gpu_vm_lock(kctx);
			reg = kctx->pending_regions[cookie];
			if (!reg) {
				kbase_gpu_vm_unlock(kctx);
				return -EINVAL;
			}
			if (!(reg->flags & KBASE_REG_GPU_NX)) {
				if (cpu_va_bits > gpu_pc_bits) {
					align_offset = 1ULL << gpu_pc_bits;
					align_mask = align_offset - 1;
					is_shader_code = true;
				}
#if !MALI_USE_CSF
			} else if (reg->flags & KBASE_REG_TILER_ALIGN_TOP) {
				unsigned long extension_bytes =
					(unsigned long)(reg->extension
							<< PAGE_SHIFT);
				/* kbase_check_alloc_sizes() already satisfies
				 * these checks, but they're here to avoid
				 * maintenance hazards due to the assumptions
				 * involved
				 */
				WARN_ON(reg->extension >
					(ULONG_MAX >> PAGE_SHIFT));
				WARN_ON(reg->initial_commit > (ULONG_MAX >> PAGE_SHIFT));
				WARN_ON(!is_power_of_2(extension_bytes));
				align_mask = extension_bytes - 1;
				align_offset =
					extension_bytes -
					(reg->initial_commit << PAGE_SHIFT);
#endif /* !MALI_USE_CSF */
			} else if (reg->flags & KBASE_REG_GPU_VA_SAME_4GB_PAGE) {
				is_same_4gb_page = true;
			}
			kbase_gpu_vm_unlock(kctx);
#ifndef CONFIG_64BIT
	} else {
		return kbase_get_unmapped_area(current->mm,
			kctx->filp, addr, len, pgoff, flags);
#endif
	}

	memset(&info, 0, sizeof(info));
	info.flags = VM_UNMAPPED_AREA_TOPDOWN;
	info.length = len;
	info.low_limit = low_limit;
	info.high_limit = high_limit;
	info.align_offset = align_offset;
	info.align_mask = align_mask;
	info.start_gap = 0;

	ret = kbase_unmapped_area_topdown(&info, is_shader_code,
			is_same_4gb_page);

	if (IS_ERR_VALUE(ret) && high_limit == mm->mmap_base &&
	    high_limit < same_va_end_addr) {
		/* Retry above mmap_base */
		memset(&info, 0, sizeof(info));
		info.flags = VM_UNMAPPED_AREA_TOPDOWN;
		info.length = len;
		info.low_limit = mm->mmap_base;
		info.high_limit = min_t(u64, TASK_SIZE, same_va_end_addr);
		info.align_offset = align_offset;
		info.align_mask = align_mask;
		info.start_gap = 0;

		ret = kbase_unmapped_area_topdown(&info, is_shader_code,
				is_same_4gb_page);
	}

	/* Fallback: bottom-up within [low_limit, same_va_end_addr] if topdown exhausted */
	if (IS_ERR_VALUE(ret)) {
		memset(&info, 0, sizeof(info));
		info.flags = 0;
		info.length = len;
		info.low_limit = low_limit;
		info.high_limit = same_va_end_addr;
		info.align_offset = align_offset;
		info.align_mask = align_mask;
		info.start_gap = 0;

		ret = vm_unmapped_area(&info);
	}

	/* Clamping fallback around mm_get_unmapped_area while mmap_lock is held */
	if (IS_ERR_VALUE(ret)) {
		unsigned long saved_base = mm->mmap_base;

		mm->mmap_base = min_t(unsigned long, saved_base, same_va_end_addr);
		ret = kbase_get_unmapped_area(mm, kctx->filp, addr, len, pgoff, flags);
		mm->mmap_base = saved_base;
	}

	pr_info_once("[CSF MMAP] Topdown allocator active: same_va_end=0x%llx, mmap_base=0x%lx, first_alloc=0x%lx\n",
		     same_va_end_addr, mm->mmap_base, ret);

	if (IS_ERR_VALUE(ret)) {
		dev_err(kctx->kbdev->dev,
			"[CSF MMAP] get_unmapped_area failed: len=0x%lx same_va_end=0x%llx mmap_base=0x%lx err=%ld\n",
			len, same_va_end_addr, mm->mmap_base, (long)ret);
	}

	return ret;
}
