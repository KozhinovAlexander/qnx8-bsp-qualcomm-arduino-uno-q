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

#ifndef QRB2210_UART_REGS_H
#define QRB2210_UART_REGS_H

#include <soc/st/qrb2210/include/utils_def.h>

/*
 * Qualcomm GENI (Generic Interface) Serial Engine register definitions for
 * the QRB2210 QUP based UART. Offsets and field masks are modeled after the
 * Linux GENI SE / serial drivers:
 *
 *   https://github.com/torvalds/linux/blob/master/include/linux/soc/qcom/geni-se.h
 *   https://github.com/torvalds/linux/blob/master/drivers/tty/serial/qcom_geni_serial.c
 */

/* Common GENI SE registers */
#define GENI_FW_REVISION_RO		U(0x0068)
#define SE_GENI_STATUS			U(0x0040)
#define SE_GENI_DMA_MODE_EN		U(0x0258)
#define SE_GENI_TX_PACKING_CFG0	U(0x0260)
#define SE_GENI_TX_PACKING_CFG1	U(0x0264)
#define SE_GENI_RX_PACKING_CFG0	U(0x0284)
#define SE_GENI_RX_PACKING_CFG1	U(0x0288)
#define SE_GENI_M_CMD0			U(0x0600)
#define SE_GENI_M_CMD_CTRL_REG		U(0x0604)
#define SE_GENI_M_IRQ_STATUS		U(0x0610)
#define SE_GENI_M_IRQ_CLEAR		U(0x0618)
#define SE_GENI_S_IRQ_STATUS		U(0x0640)
#define SE_GENI_S_IRQ_CLEAR		U(0x0648)
#define SE_GENI_TX_FIFOn		U(0x0700)
#define SE_GENI_RX_FIFOn		U(0x0780)
#define SE_GENI_RX_FIFO_STATUS		U(0x0804)

/* UART specific GENI registers */
#define SE_UART_TX_TRANS_CFG		U(0x025C)
#define SE_UART_TX_WORD_LEN		U(0x0268)
#define SE_UART_TX_STOP_BIT_LEN	U(0x026C)
#define SE_UART_TX_TRANS_LEN		U(0x0270)
#define SE_UART_RX_TRANS_CFG		U(0x0280)
#define SE_UART_RX_WORD_LEN		U(0x028C)
#define SE_UART_TX_PARITY_CFG		U(0x02A4)
#define SE_UART_RX_PARITY_CFG		U(0x02A8)

/* SE_GENI_STATUS fields */
#define M_GENI_CMD_ACTIVE		BIT(0)

/* GENI_FW_REVISION_RO fields */
#define FW_REV_PROTOCOL_MSK		GENMASK(15, 8)
#define FW_REV_PROTOCOL_SHFT		8

/* Protocol value programmed by the SE firmware loader for UART mode */
#define GENI_SE_PROTOCOL_UART		U(0x2)

/* SE_GENI_DMA_MODE_EN fields */
#define GENI_DMA_MODE_EN		BIT(0)

/* SE_GENI_M_CMD0 fields */
#define M_OPCODE_SHFT			27
#define UART_START_TX			U(0x1)

/* SE_GENI_M_CMD_CTRL_REG fields */
#define M_GENI_CMD_CANCEL		BIT(2)

/* SE_GENI_M_IRQ_STATUS/CLEAR fields */
#define M_CMD_DONE_EN			BIT(0)
#define M_CMD_CANCEL_EN			BIT(4)

/* SE_GENI_S_IRQ_STATUS/CLEAR fields (break/parity indications) */
#define S_GP_IRQ_2_EN			BIT(11)
#define S_GP_IRQ_3_EN			BIT(12)
#define S_BREAK_IRQS			(S_GP_IRQ_2_EN | S_GP_IRQ_3_EN)

/* SE_GENI_RX_FIFO_STATUS fields */
#define RX_FIFO_WC_MSK			GENMASK(24, 0)

/* SE_UART_TX_TRANS_CFG / SE_UART_RX_TRANS_CFG fields */
#define UART_CTS_MASK			BIT(1)

/* SE_UART_TX_STOP_BIT_LEN fields */
#define TX_STOP_BIT_LEN_1		U(0x0)

/* Number of data bits per UART character (8N1) */
#define UART_BITS_PER_CHAR		U(8)

/*
 * TX/RX FIFO packing configuration for byte-oriented (8 bit, 4 bytes per
 * 32-bit FIFO word, LSB first) transfers. Equivalent to calling the Linux
 * geni_se_config_packing(se, 8, 4, false, true, true) helper.
 */
#define GENI_UART_PACKING_CFG0		U(0x0004380E)
#define GENI_UART_PACKING_CFG1		U(0x000C3E0E)

#endif /* QRB2210_UART_REGS_H */
