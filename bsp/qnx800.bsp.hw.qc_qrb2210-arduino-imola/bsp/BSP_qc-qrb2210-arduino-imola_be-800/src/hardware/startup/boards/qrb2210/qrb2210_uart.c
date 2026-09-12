/*
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
 * Polled serial operations for the QRB2210 GENI (QUP) UART.
 *
 * Modeled after the Linux GENI serial driver, in particular the polled
 * poll_put_char()/earlycon_setup() paths, which never touch clocks or load
 * serial engine firmware and assume the boot loader already did so:
 *
 *   https://github.com/torvalds/linux/blob/master/drivers/tty/serial/qcom_geni_serial.c
 */

#include <startup.h>

#include "board.h"
#include <soc/st/qrb2210/include/qrb2210_uart.h>

/**
 * QRB2210 startup source file.
 *
 * @file       qrb2210_uart.c
 * @addtogroup startup
 * @{
 */

/* Bounded spin count for GENI bit polling, so a stuck/unexpected hardware
 * state can never wedge the boot forever (and trip the SoC watchdog with
 * no console output at all). */
#define GENI_POLL_LOOPS 1000000U

/**
 * Poll a GENI register until (value & mask) == mask, or give up after a
 * bounded number of iterations.
 *
 * @return 1 if the bit(s) became set, 0 on timeout.
 */
static int wait_geni_bits(uint32_t addr, uint32_t mask)
{
    unsigned i;

    for (i = 0; i < GENI_POLL_LOOPS; i++) {
        if ((in32(addr) & mask) == mask) {
            return 1;
        }
    }
    return 0;
}

/**
 * Parse UART initialization options (base address, register shift, baud
 * rate and input clock).
 *
 * @param channel Debug device index (in debug_devices structure)
 * @param line    String line to parse.
 * @param baud    Pointer to baudrate variable.
 * @param clk     Pointer to input peripheral clock variable.
 */
static void parse_line(unsigned channel, const char *line, unsigned *baud, unsigned *clk)
{
    /* Get device base address and register stride */
    if ((*line != '.') && (*line != '\0')) {
        dbg_device[channel].base = strtoul(line, (char **)&line, 16);
        if (*line == '^') {
            dbg_device[channel].shift = strtoul(line + 1, (char **)&line, 0);
        }
    }

    /* Get baud rate value */
    if (*line == '.') {
        ++line;
    }
    if ((*line != '.') && (*line != '\0')) {
        *baud = strtoul(line, (char **)&line, 0);
    }

    /* Get input device clock rate value */
    if (*line == '.') {
        ++line;
    }
    if (*line != '.' && *line != '\0') {
        *clk = strtoul(line, (char **)&line, 0);
    }
}

/**
 * Initialise the GENI based UART debug/console serial port.
 *
 * The serial engine firmware, protocol selection and byte clock are expected
 * to already be configured by the primary/secondary boot loader (mirroring
 * the "qcom,geni-debug-uart" earlycon in Linux). This routine only verifies
 * that the serial engine is indeed running the UART protocol, cancels any
 * primary sequencer command left active by the boot loader and (re)programs
 * the FIFO packing/transfer registers for 8N1 framing.
 *
 * @param channel   Debug device index (in debug_devices structure)
 * @param init      String line with configuration parameters.
 * @param defaults  String line with configuration parameters.
 */
void qrb2210_init_uart(unsigned channel, const char *init, const char *defaults)
{
    uint32_t base;
    uint32_t proto;

    /*
     * Default UART settings:
     * Baud rate: 115200
     * Data: 8 bit
     * Parity: none
     * Stop: 1 bit
     * Flow control: none
     */
    unsigned int baud_rate = 115200;
    unsigned int clockfreq = 7372800; /* default GENI SE core clock rate (unused) */

    parse_line(channel, defaults, &baud_rate, &clockfreq);
    parse_line(channel, init, &baud_rate, &clockfreq);
    base = dbg_device[channel].base;

    if (baud_rate == 0) {
        crash("qrb2210_uart: invalid baud rate %d\n", baud_rate);
        return;
    }

    proto = (in32(base + GENI_FW_REVISION_RO) & FW_REV_PROTOCOL_MSK) >> FW_REV_PROTOCOL_SHFT;
    if (proto != GENI_SE_PROTOCOL_UART) {
        /* Boot loader didn't leave the SE in UART mode; leave it untouched
         * rather than reprogramming a serial engine of unknown protocol. */
        return;
    }

    /* Cancel any primary sequencer command left running by the boot loader */
    if (in32(base + SE_GENI_STATUS) & M_GENI_CMD_ACTIVE) {
        out32(base + SE_GENI_M_CMD_CTRL_REG, M_GENI_CMD_CANCEL);
        if (!wait_geni_bits(base + SE_GENI_M_IRQ_STATUS, M_CMD_CANCEL_EN)) {
            out32(base + SE_GENI_M_CMD_CTRL_REG, M_GENI_CMD_ABORT);
            wait_geni_bits(base + SE_GENI_M_IRQ_STATUS, M_CMD_ABORT_EN);
            out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_ABORT_EN);
        }
        out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_CANCEL_EN);
    }

    /* Byte-oriented (4 bytes per FIFO word) TX/RX packing, LSB first */
    out32(base + SE_GENI_TX_PACKING_CFG0, GENI_UART_PACKING_CFG0);
    out32(base + SE_GENI_TX_PACKING_CFG1, GENI_UART_PACKING_CFG1);
    out32(base + SE_GENI_RX_PACKING_CFG0, GENI_UART_PACKING_CFG0);
    out32(base + SE_GENI_RX_PACKING_CFG1, GENI_UART_PACKING_CFG1);

    /* Ignore flow control, 8 data bits, 1 stop bit, no parity */
    out32(base + SE_UART_TX_TRANS_CFG, UART_CTS_MASK);
    out32(base + SE_UART_TX_PARITY_CFG, 0);
    out32(base + SE_UART_RX_TRANS_CFG, 0);
    out32(base + SE_UART_RX_PARITY_CFG, 0);
    out32(base + SE_UART_TX_WORD_LEN, UART_BITS_PER_CHAR);
    out32(base + SE_UART_RX_WORD_LEN, UART_BITS_PER_CHAR);
    out32(base + SE_UART_TX_STOP_BIT_LEN, TX_STOP_BIT_LEN_1);

    /* Use FIFO mode (not DMA) for the primary/secondary sequencers */
    out32(base + SE_GENI_DMA_MODE_EN, in32(base + SE_GENI_DMA_MODE_EN) & ~GENI_DMA_MODE_EN);
}

/**
 * Send a character (blocking / polled), mirroring
 * qcom_geni_serial_poll_put_char() in the Linux GENI serial driver.
 *
 * @param data Character to send.
 */
void qrb2210_uart_put_char(int data)
{
    uint32_t base = dbg_device[0].base;

    /* Wait for (or give up on) any command still running from a previous call */
    if (in32(base + SE_GENI_STATUS) & M_GENI_CMD_ACTIVE) {
        if (!wait_geni_bits(base + SE_GENI_M_IRQ_STATUS, M_CMD_DONE_EN)) {
            out32(base + SE_GENI_M_CMD_CTRL_REG, M_GENI_CMD_ABORT);
            wait_geni_bits(base + SE_GENI_M_IRQ_STATUS, M_CMD_ABORT_EN);
            out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_ABORT_EN);
        }
        out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_DONE_EN);
    }

    out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_DONE_EN);
    out32(base + SE_UART_TX_TRANS_LEN, 1);
    out32(base + SE_GENI_M_CMD0, UART_START_TX << M_OPCODE_SHFT);
    out32(base + SE_GENI_TX_FIFOn, (uint32_t)data & 0xff);

    if (!wait_geni_bits(base + SE_GENI_M_IRQ_STATUS, M_CMD_DONE_EN)) {
        out32(base + SE_GENI_M_CMD_CTRL_REG, M_GENI_CMD_ABORT);
        wait_geni_bits(base + SE_GENI_M_IRQ_STATUS, M_CMD_ABORT_EN);
        out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_ABORT_EN);
    }
    out32(base + SE_GENI_M_IRQ_CLEAR, M_CMD_DONE_EN);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/hardware/branches/release/hardware/startup/boards/qrb2210/qrb2210_init_uart.c $ $Rev: 984580 $")
#endif
