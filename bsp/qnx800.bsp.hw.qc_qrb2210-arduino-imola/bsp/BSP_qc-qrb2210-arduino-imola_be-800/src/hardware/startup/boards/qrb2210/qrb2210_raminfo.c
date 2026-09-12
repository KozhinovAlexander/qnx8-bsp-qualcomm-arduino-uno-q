/*
 * $QNXLicenseC:
 * Copyright 2016, 2022 BlackBerry Limited.
 * Copyright 2022 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */



/*
 * init_raminfo.c
 * Tell syspage about our RAM configuration
 */
#include <startup.h>
#include "board.h"
#include "qrb2210_startup.h"

/**
 * QRB2210 startup source file.
 *
 * @file       qrb2210_init_raminfo.c
 * @addtogroup startup
 * @{
 */

/*
 * Firmware/hypervisor reserved regions, taken from the upstream QCM2290
 * device tree "reserved-memory" node (dts/upstream/src/arm64/qcom/qcm2290.dtsi):
 *
 *   hyp@45700000       0x600000   \
 *   xbl-aop@45e00000   0x140000    | inside the QRB2210_DRAM0_TFA_SIZE carve-out
 *   sec-apps@45fff000  0x1000      |
 *   smem@46000000      0x200000   /
 *   modem@4ab00000     0x6900000  \
 *   video@51400000     0x500000    | contiguous, straddles the TFA carve-out
 *   wlan-msa@51900000  0x100000    | boundary, ends at QRB2210_RSVD_LOWBLOCK_END
 *   adsp@51a00000      0x1c00000   |
 *   ipa-fw@53600000    0x10000     |
 *   ipa-gsi@53610000   0x5000      |
 *   zap@53615000       0x2000     /
 *   framebuffer@5c000000  0xf00000  \ contiguous splash/dfps block
 *   dpfs-data@5cf00000    0x100000  /
 *   reserved@60000000     0x3900000  (removed_mem)
 *   memory@89b01000       0x200000   (rmtfs_mem)
 */
#define QRB2210_RSVD_LOWBLOCK_END  0x53617000UL
#define QRB2210_RSVD_SPLASH_BASE   0x5C000000UL
#define QRB2210_RSVD_SPLASH_END    0x5D000000UL
#define QRB2210_RSVD_REMOVED_BASE  0x60000000UL
#define QRB2210_RSVD_REMOVED_END   0x63900000UL
#define QRB2210_RSVD_RMTFS_BASE    0x89B01000UL
#define QRB2210_RSVD_RMTFS_END     0x89D01000UL

/**
 * Add RAM area information to the system page.
 */
void qrb2210_init_raminfo(void)
{
    /*
     * The first QRB2210_DRAM0_TFA_SIZE bytes are used by TF-A/XBL and
     * overlap the start of the modem/video/wlan/adsp/ipa/gpu reserved
     * block, so the first free span starts only after that block ends.
     */
    add_ram(QRB2210_RSVD_LOWBLOCK_END, QRB2210_RSVD_SPLASH_BASE - QRB2210_RSVD_LOWBLOCK_END);
    add_ram(QRB2210_RSVD_SPLASH_END, QRB2210_RSVD_REMOVED_BASE - QRB2210_RSVD_SPLASH_END);
    add_ram(QRB2210_RSVD_REMOVED_END, QRB2210_RSVD_RMTFS_BASE - QRB2210_RSVD_REMOVED_END);
    add_ram(QRB2210_RSVD_RMTFS_END,
            (QRB2210_DRAM0_BASE + QRB2210_DRAM0_SIZE) - QRB2210_RSVD_RMTFS_END);
    /* Add 4 KB /memory/dma region. This region is not used by QNX and is dedicated to DMA. */
    // as_add(QRB2210_DRAM0_BASE, QRB2210_DRAM0_BASE + KILO(4) - 1, AS_ATTR_NONE, "dma", as_default());
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/hardware/branches/release/hardware/startup/boards/qrb2210/qrb2210_init_raminfo.c $ $Rev: 984580 $")
#endif
