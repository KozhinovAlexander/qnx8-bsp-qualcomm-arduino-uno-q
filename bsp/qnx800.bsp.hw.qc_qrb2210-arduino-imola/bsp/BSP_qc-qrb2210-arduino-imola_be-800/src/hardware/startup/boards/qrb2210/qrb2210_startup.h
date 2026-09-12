/*
 * $QNXLicenseC:
 * Copyright 2026 Alexander Kozhinov <ak.alexander.kozhinov@gmail.com>
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


#ifndef QRB2210_STARTUP_H_
#define QRB2210_STARTUP_H_

#include <soc/st/qrb2210/include/qrb2210_wdt.h>

/**
 * QRB2210 startup source file.
 *
 * @file       qrb2210_startup.h
 * @addtogroup startup
 * @{
 */

extern void qrb2210_init_uart(unsigned channel, const char *init, const char *defaults);
extern void qrb2210_uart_put_char(int);

extern void qrb2210_init_pcie_ext_msi_controller(void);

extern struct callout_rtn qrb2210_uart_display_char;
extern struct callout_rtn qrb2210_uart_poll_key;
extern struct callout_rtn qrb2210_uart_break_detect;

void qrb2210_init_raminfo(void);
void qrb2210_init_hwinfo(void);
void qrb2210_board_mmu_enable(unsigned base, unsigned size);

#endif /* QRB2210_STARTUP_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/hardware/branches/release/hardware/startup/boards/qrb2210/qrb2210_startup.h $ $Rev: 979659 $")
#endif
