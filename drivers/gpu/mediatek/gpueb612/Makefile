#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# GED 6.12 backport: same source tree, built for the 5.10 kernel.
#
# Notes:
#  - module name stays "gpueb" (ged / mtk_gpufreq_wrapper / fhctl depend
#    on that name; 6.12 renamed it to mtk_gpueb)
#  - the GHPM sources are deliberately not built: they only implement
#    mt6991/mt6993 hardware, MT6895 has no GHPM
#

ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/gpu/mediatek/gpueb612/include/
ccflags-y += -Wno-unused-variable
ccflags-y += -Wno-error=macro-redefined
ccflags-y += -Wno-error=implicit-function-declaration
ccflags-y += -Wno-error=incompatible-function-pointer-types
ccflags-y += -Wno-error=incompatible-pointer-types
ccflags-y += -Wno-error=int-conversion
ccflags-y += -Wno-error=declaration-after-statement

obj-$(CONFIG_MTK_GPU_SUPPORT) += gpueb.o

gpueb-y += \
    gpueb_init.o \
    gpueb_ipi.o \
    gpueb_logger.o \
    gpueb_plat_service.o \
    gpueb_reserved_mem.o \
    gpueb_gpumpu.o \
    gpueb_hwvoter_dbg.o \
    gpueb_debug.o \
    gpueb_timesync.o \
    ghpm_compat_510.o

# GHPM (mt6991/mt6993 only) is intentionally excluded
