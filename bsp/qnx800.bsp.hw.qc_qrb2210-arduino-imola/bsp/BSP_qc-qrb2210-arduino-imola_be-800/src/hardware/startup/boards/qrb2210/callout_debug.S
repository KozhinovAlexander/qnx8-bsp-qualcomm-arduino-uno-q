/*
 * Copyright (c) 2016,2022-2023, BlackBerry Limited.
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
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

/*
 * QRB2210 GENI (QUP) UART specific polled serial I/O.
 *
 * Modeled after the Linux qcom_geni_serial driver polled TX/RX paths:
 * https://github.com/torvalds/linux/blob/master/drivers/tty/serial/qcom_geni_serial.c
 */

#include <aarch64/callout.ah>
#include "board.h"
#include <soc/st/qrb2210/include/qrb2210_uart.h>

/*
 * Patch interrupt callouts to access rw data.
 * The first call will also map the uart.
 *
 * Patcher routine takes the following arguments:
 *    x0 - syspage paddr
 *    x1 - vaddr of callout
 *    x2 - offset from start of syspage to start of callout routine
 *    x3 - offset from start of syspage to rw storage
 *    x4 - patch data
 *    x5 - callout_rtn
 */

patch_debug:
    sub        sp, sp, #16
    stp        x19, x30, [sp]

    add        x19, x0, x2        // x19 = address of callout routine

    /* Map the whole GENI serial engine register block using patch_data */
    mov        x0, #QRB2210_UART4_SE_SIZE
    ldr        x1, [x4]
    bl         callout_io_map

    /* Patch callout with mapped virtual address in x0 */
    CALLOUT_PATCH    x19, w6, w7

    ldp        x19, x30, [sp]
    add        sp, sp, #16
    ret

/*
 * -----------------------------------------------------------------------
 * void    qrb2210_uart_display_char(struct sypage_entry *, char)
 *
 * x0: syspage pointer
 * x1: character
 * -----------------------------------------------------------------------
 */
CALLOUT_START(qrb2210_uart_display_char, 0, patch_debug)
    mov        x7, #0xabcd                // GENI SE base address (patched)
    movk       x7, #0xabcd, lsl #16
    movk       x7, #0xabcd, lsl #32
    movk       x7, #0xabcd, lsl #48

    /* Clear a stale M_CMD_DONE and kick off a 1 byte TX command */
    mov        w2, #M_CMD_DONE_EN
    str        w2, [x7, #SE_GENI_M_IRQ_CLEAR]
    mov        w2, #1
    str        w2, [x7, #SE_UART_TX_TRANS_LEN]
    mov        w2, #(UART_START_TX << M_OPCODE_SHFT)
    str        w2, [x7, #SE_GENI_M_CMD0]

    and        w0, w1, #0xff
    str        w0, [x7, #SE_GENI_TX_FIFOn]

    /* Wait for the primary sequencer to report command completion */
0:
    ldr        w2, [x7, #SE_GENI_M_IRQ_STATUS]
    tst        w2, #M_CMD_DONE_EN
    beq        0b

    mov        w2, #M_CMD_DONE_EN
    str        w2, [x7, #SE_GENI_M_IRQ_CLEAR]
    ret
CALLOUT_END(qrb2210_uart_display_char)


/*
 * -----------------------------------------------------------------------
 * char    qrb2210_uart_poll_key(struct sypage_entry *)
 *
 * x0: syspage pointer
 * -----------------------------------------------------------------------
 */
CALLOUT_START(qrb2210_uart_poll_key, 0, patch_debug)
    mov     x7, #0xabcd                // GENI SE base address (patched)
    movk    x7, #0xabcd, lsl #16
    movk    x7, #0xabcd, lsl #32
    movk    x7, #0xabcd, lsl #48

    ldr     w1, [x7, #SE_GENI_RX_FIFO_STATUS]
    and     w1, w1, #RX_FIFO_WC_MSK
    cbz     w1, 0f
    ldr     w0, [x7, #SE_GENI_RX_FIFOn]
    and     w0, w0, #0xff
    ret
0:
    mov     w0, #-1
    ret
CALLOUT_END(qrb2210_uart_poll_key)


/*
 * -----------------------------------------------------------------------
 * int qrb2210_uart_break_detect(struct syspage_entry *)
 *
 * x0: syspage pointer
 *
 * Return 1 if break detected, 0 otherwise
 * -----------------------------------------------------------------------
 */
CALLOUT_START(qrb2210_uart_break_detect, 0, patch_debug)
    mov     x7, #0xabcd                // GENI SE base address (patched)
    movk    x7, #0xabcd, lsl #16
    movk    x7, #0xabcd, lsl #32
    movk    x7, #0xabcd, lsl #48

    ldr     w0, [x7, #SE_GENI_S_IRQ_STATUS]
    and     w1, w0, #S_BREAK_IRQS
    cbz     w1, 0f

    /* Acknowledge the break condition and report it */
    str     w1, [x7, #SE_GENI_S_IRQ_CLEAR]
    mov     w0, #1
    ret
0:
    mov     w0, #0
    ret
CALLOUT_END(qrb2210_uart_break_detect)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/hardware/branches/release/hardware/startup/boards/qrb2210/callout_debug.S $ $Rev: 984398 $";
.previous
#endif
#endif
