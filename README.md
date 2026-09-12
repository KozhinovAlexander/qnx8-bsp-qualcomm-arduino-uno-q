# qnx8-bsp-qualcomm-arduino-uno-q

![Arduino UNO Q Pinout](./docs/imgs/Simple-pinout-ABX00162.png)

## Prerequesties

- Connect `1v8 TTL` level `USB to UART` adapter to `JCTL` `SOC_SE4_RX` and `SOC_SE4_TX` pins.
- Connect Arduino UNO Q over USB-C cable to yout Linux hosz machine.
- Use minicom to connect to `SOC_SE4` `UART`: `minicom -b 115200 -D /dev/ttyACM0`.
    **NOTE:** The `/dev/ttyACM0` device may appera different on your machine depending on the UART adapter used.
- Execute following make targets:

```bash
make git_submodules_configure install_dependencies qdl
```

## Flashing

This section flashes bootloader partitions from [arduino-unoq-debian](https://downloads.arduino.cc/debian-im/Stable/20260528-558/arduino-unoq-debian-image-20260528-558.tar.zst) linux image.

Put the board in `EDL` mode by shorting the pins `USB_BOOT` and `GND` - see pinout-diagram above.

Flash the board by executing:

```bash
make bootloader_flash
```

Leave `USB_BOOT` and `GND` pins free now to boot the board normal way.

After flashing the target the bootable partition with linux image is empty. This is intended.
The board shall boot into U-Boot now. Following U-Boot environment settings shall be done now:

## Loading Image to RAM and booting it

Configure U-Boot environment first:

```bash
# setenv ifs_file ifs-qrb2210-arduino-imola.bin
setenv ifs_part 43
# fdt file is located at partition 0x43 (efi)
# setenv fdtfile qcom/qrb2210-arduino-imola.dtb
setenv loaddtb 'fatload mmc 0:${ifs_part} ${fdt_addr_r} ${fdtfile}'
# setenv loadifs 'fatload mmc 0:${ifs_part} ${kernel_addr_r} ${ifs_file}'
# setenv bootifs 'run loaddtb; run loadifs; go ${kernel_addr_r} ${fdt_addr_r}'
# setenv loadtftp 'tftp ${kernel_addr_r} ${ifs_file}'
# setenv boottftp 'run loaddtb; run loadtftp; go ${kernel_addr_r} ${fdt_addr_r}'
setenv fastboot_bootcmd 'run loaddtb; fdt addr ${fdt_addr_r}; bootm ${fastboot_addr_r} ${fastboot_addr_r} ${fdt_addr_r}'
```

after that run fastboot in uboot

```bash
run fastboot
```

or all in one line:

```bash
setenv doall ' \
setenv ifs_part 43; \
setenv loaddtb "fatload mmc 0:${ifs_part} ${fdt_addr_r} ${fdtfile}"; \
setenv fastboot_bootcmd "run loaddtb; fdt addr ${fdt_addr_r}; bootm ${fastboot_addr_r} ${fastboot_addr_r} ${fdt_addr_r}"; run fastboot'
```

***NOTE:** Ensure now `Google usb download gadget` appears in your system

Run on your host machine:

```bash
fastboot devices
```

Use fastboot command on the host to load image into RAM:

```bash
fastboot boot /srv/tftp/ifs-qrb2210-arduino-imola.raw
```

## Creating latest Bootloader

Createing latest bootloader can be done by executing following commands:

```bash
make unoq_linux_download unoq_linux_unpack bootloader_pack
```
