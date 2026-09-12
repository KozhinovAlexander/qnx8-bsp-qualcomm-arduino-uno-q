/*
 * Copyright (c) 2016, 2022-2023, BlackBerry Limited.
 * Copyright 2022-2023 NXP
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
 */


#ifndef BOARD_H_
#define BOARD_H_

#ifndef __ASM__
#include "qrb2210_startup.h"
#endif


/** QRB2210 SoC cores number */
#define QRB2210_CPU_CORES_NUMBER  4

#define QRB2210_RCC_BASE_ADDR    (0x44200000UL)

/** Core counter input clock (in MHz) */
#define QRB2210_HSE_CLOCK_FREQ   40'000'000

/** QRB2210 USART2 base address and size */
#define QRB2210_UART4_BASE_ADDR   (0x400e0000UL)
#define QRB2210_UART_SIZE         (0x400UL)

/*!
 * @name QNX SDRAM memory configuration
 */
/*@{*/
/** DRAM0 base address (QCM2290/QRB2210 DDR always starts at 0x40000000,
 *  see upstream "memory@40000000" node in qcm2290.dtsi) */
#define QRB2210_DRAM0_BASE       (0x40000000UL)
/** DRAM0 size in bytes */
#define QRB2210_DRAM0_SIZE       GIG(4UL)
/** DRAM0 TF-A stage size in bytes (BL2/BL31 etc.) */
#define QRB2210_DRAM0_TFA_SIZE   MEG(256UL)
/** DRAM0 base address available for OS */
#define QRB2210_DRAM0_OS_BASE    (QRB2210_DRAM0_BASE + QRB2210_DRAM0_TFA_SIZE)
/** DRAM0 base size available for OS */
#define QRB2210_DRAM0_OS_SIZE    (QRB2210_DRAM0_SIZE - QRB2210_DRAM0_TFA_SIZE)
/*@}*/

/** QRB2210 GIC addresses.
 *  QCM2290/QRB2210 uses an ARM GICv3 (no memory-mapped GICC/GICH/GICV),
 *  see "intc: interrupt-controller@f200000" in the upstream sm6115.dtsi:
 *  reg = <0x0 0x0f200000 0x0 0x10000>,   / * GICD * /
 *        <0x0 0x0f300000 0x0 0x100000>; / * GICR * /
 */
#define GICD_PADDR             (0x0F200000UL)
#define GICR_PADDR             (0x0F300000UL)

#endif  /* BOARD_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/hardware/branches/release/hardware/startup/boards/qrb2210/dk/board.h $ $Rev: 985114 $")
#endif
