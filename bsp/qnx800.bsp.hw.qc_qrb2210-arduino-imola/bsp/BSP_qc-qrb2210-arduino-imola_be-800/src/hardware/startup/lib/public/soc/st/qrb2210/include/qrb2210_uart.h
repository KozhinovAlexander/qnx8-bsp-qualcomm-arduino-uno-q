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

#ifndef QRB2210_UART_H
#define QRB2210_UART_H

#include <soc/st/qrb2210/include/qrb2210_uart_regs.h>

/*
 * source: https://github.com/torvalds/linux/blob/5225b8eec4c9bb21aecff6295fab6346a3c3738e/arch/arm64/boot/dts/qcom/agatti.dtsi#L1493
 * uart4: serial@4a90000 {
 *   compatible = "qcom,geni-uart";
 *   reg = <0x0 0x04a90000 0x0 0x4000>;
 *   interrupts = <GIC_SPI 331 IRQ_TYPE_LEVEL_HIGH>;
 *   clocks = <&gcc GCC_QUPV3_WRAP0_S4_CLK>;
 *   clock-names = "se";
 *   pinctrl-0 = <&qup_uart4_default>;
 *   pinctrl-names = "default";
 *   interconnects = <&qup_virt MASTER_QUP_CORE_0 RPM_ALWAYS_TAG
 *         &qup_virt SLAVE_QUP_CORE_0 RPM_ALWAYS_TAG>,
 *       <&bimc MASTER_APPSS_PROC RPM_ALWAYS_TAG
 *         &config_noc SLAVE_QUP_0 RPM_ALWAYS_TAG>;
 *   interconnect-names = "qup-core",
 *             "qup-config";
 *   status = "disabled";
 * };
 *
 * The GENI serial engine firmware, protocol selection and byte clock are
 * expected to already be configured by the primary/secondary boot loader
 * before QNX startup runs, mirroring the assumptions made by the Linux
 * "qcom,geni-debug-uart" earlycon implementation (which never touches
 * clocks nor loads firmware either).
 */

#define QRB2210_UART4_SE_BASE       0x4a90000UL
#define QRB2210_UART4_SE_SIZE       0x4000UL

#ifndef __ASSEMBLER__
/**
 * Initialise the GENI based UART debug/console serial port.
 *
 * @param channel   Debug device index (in debug_devices structure)
 * @param init      String line with configuration parameters.
 * @param defaults  String line with configuration parameters.
 */
void qrb2210_init_uart(unsigned channel, const char *init, const char *defaults);

/**
 * Send a character (blocking / polled).
 *
 * @param data Character to send.
 */
void qrb2210_uart_put_char(int data);
#endif /* __ASSEMBLER__ */

#endif /* QRB2210_UART_H */
