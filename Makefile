## =============================================================================
# QNX 8 BSP Top-Level Makefile — Arduino UNO Q (Qualcomm QRB 2210)
# =============================================================================
# Usage:
#   make all
# =============================================================================

CURRENT_DIR := $(CURDIR)
SHELL := /bin/bash

QNX_VERSION := 800
QNX_INSTALL_DIR := $(HOME)/qnx$(QNX_VERSION)

ARDUINO_CLI_INSTALL_DIR := $(CURRENT_DIR)/tools/bin

SOC_VENDOR := qc
BOARD := arduino-uno-q
BSP_ROOT_DIR := $(CURRENT_DIR)/bsp/qnx$(QNX_VERSION).bsp.hw.$(SOC_VENDOR)_$(BOARD)/bsp/BSP_$(SOC_VENDOR)-$(BOARD)_be-$(QNX_VERSION)


.PHONY: all
all: bsp_all

.PHONY: install_dependencies
install_dependencies:
	@sudo apt update
	@sudo apt install -y curl gawk curl wget git git-lfs diffstat unzip texinfo tftpd-hpa libncurses-dev build-essential \
		libfdt-dev libxml2-dev libusb-1.0-0-dev libzip-dev pv android-tools-fastboot \
		meson ninja-build help2man libcmocka-dev cmake
	@sudo apt upgrade -y
	@sudo apt autoremove -y
	@sudo apt clean

.PHONY: git_submodules_configure
git_submodules_configure:
	@declare -A modules_map=( \
		["qdl"]="https://github.com/linux-msm/qdl.git,master" \
	); \
	for key in "$${!modules_map[@]}"; do \
		value="$${modules_map[$$key]}"; \
		url="$${value%,*}"; \
		branch="$${value#*,}"; \
		pushd $(CURRENT_DIR); \
		git submodule add --force --name $$key --branch $$branch $$url $$key; \
		popd; \
	done; \
	git submodule update --init --recursive

.PHONY: setup_host_tftp_server
setup_host_tftp_server:
	@sudo systemctl enable tftpd-hpa
	@sudo systemctl start tftpd-hpa
	@TFTP_DIRECTORY=$$(sed -n 's/^TFTP_DIRECTORY[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' /etc/default/tftpd-hpa); \
		echo "TFTP directory: $$TFTP_DIRECTORY"; \
		sudo chown -R $(shell id -u):$(shell id -g) $$TFTP_DIRECTORY;
	@echo "TFTP server setup completed. Please ensure the TFTP root directory is configured correctly."

.PHONY: tftp_server_transfer
tftp_server_transfer:
	@TFTP_DIRECTORY=$$(sed -n 's/^TFTP_DIRECTORY[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' /etc/default/tftpd-hpa); \
		echo "Transferring files to TFTP directory: $$TFTP_DIRECTORY"; \
		rm -rf $$TFTP_DIRECTORY/$(FILE); \
		cp -r $(FILE) $$TFTP_DIRECTORY;

# please refer to: https://www.qnx.com/developers/docs/BSP8.0/com.qnx.doc.bsp_raspberrypi.bcm2712.rpi5_8.0/topic/common/build_commandline.html
# for make file flags refer to: https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_prog/make_convent.html#PARTIAL
.PHONY: bsp_all
EXCLUDE_FROM_BUILD_LIST := i2c spi
MAKE_LIST_EXCLUDE := LIST=CONTROL "EXCLUDE_CONTROLLIST=$(EXCLUDE_FROM_BUILD_LIST)"
bsp_all:
	@mkdir -p $(BSP_ROOT_DIR)/install
	@source $(QNX_INSTALL_DIR)/qnxsdp-env.sh \
		&& $(MAKE) JLEVEL=$$(nproc) -C$(BSP_ROOT_DIR)/images clean \
		&& $(MAKE) JLEVEL=$$(nproc) -C$(BSP_ROOT_DIR) LIST=CONTROL $(MAKE_LIST_EXCLUDE) all \
		&& $(MAKE) JLEVEL=$$(nproc) -C$(BSP_ROOT_DIR)/images $(MAKE_LIST_EXCLUDE) ifs-$(BOARD).raw
	@$(MAKE) bsp_prebuilt
	@$(MAKE) tftp_server_transfer FILE=$(BSP_ROOT_DIR)/images/ifs-$(BOARD).raw

.PHONY: bsp_prebuilt
bsp_prebuilt:
	@mkdir -p $(BSP_ROOT_DIR)/prebuilt
	@cp -r $(BSP_ROOT_DIR)/install/* $(BSP_ROOT_DIR)/prebuilt

.PHONY: bsp_clean
bsp_clean:
	@source $(QNX_INSTALL_DIR)/qnxsdp-env.sh \
		&& $(MAKE) JLEVEL=$$(nproc) -C $(BSP_ROOT_DIR)/images clean \
		&& $(MAKE) JLEVEL=$$(nproc) -C $(BSP_ROOT_DIR) clean

.PHONY: clean
clean: bsp_clean bootloader_clean qdl_clean
	@echo "Cleaned all build artifacts."

.PHONY: bootloader_pack
ARCH_NAME := deploy_images_orig
bootloader_pack:
	@mkdir -p $(CURRENT_DIR)/bootloader
	@rm -f $(CURRENT_DIR)/bootloader/$(ARCH_NAME).tar.gz
	@tar -cvzf $(CURRENT_DIR)/bootloader/$(ARCH_NAME).tar.gz \
		-C $(UNOQ_LINUX_DIR) .

.PHONY: bootloader_unpack
BOOTLOADER_OUT_DIR := $(CURRENT_DIR)/bootloader/output
bootloader_unpack:
	@rm -rf $(BOOTLOADER_OUT_DIR)
	@mkdir -p $(BOOTLOADER_OUT_DIR)
	@tar -xvzf $(CURRENT_DIR)/bootloader/$(ARCH_NAME).tar.gz \
		-C $(BOOTLOADER_OUT_DIR)

.PHONY: bootloader_clean
bootloader_clean:
	@rm -rf $(BOOTLOADER_OUT_DIR)

# Flash the device using EDL mode
# source: https://docs.qualcomm.com/doc/80-70022-254/topic/flash_images.html
.PHONY: bootloader_flash
FIREHOSE_ELF := $(BOOTLOADER_OUT_DIR)/flash/prog_firehose_ddr.elf
bootloader_flash: udev_rules bootloader_unpack
	@mkdir -p $(BOOTLOADER_OUT_DIR)
	@$(QDL_BIN) --storage emmc \
		$(FIREHOSE_ELF) \
		$(BOOTLOADER_OUT_DIR)/flash/rawprogram0.xml \
		$(BOOTLOADER_OUT_DIR)/flash/patch0.xml

.PHONY: udev_rules
udev_rules:
	@echo "Deploying QDL udev rules..."
	@sudo cp $(CURRENT_DIR)/tools/udev_rules/51-qcom-usb.rules /etc/udev/rules.d/
	@sudo groupadd -f plugdev
	@sudo usermod -aG plugdev $$USER
	@sudo usermod -a -G dialout $$USER
	@sudo udevadm control --reload-rules
	@sudo udevadm trigger

.PHONY: qdl_app_build
qdl_app_build:
	@meson setup $(CURRENT_DIR)/qdl/build $(CURRENT_DIR)/qdl --reconfigure
	@meson compile -C $(CURRENT_DIR)/qdl/build

.PHONY: qdl_app_install
QDL_INSTALL_DIR := $(CURRENT_DIR)/tools/apps/qdl
QDL_BIN := $(QDL_INSTALL_DIR)/usr/local/bin/qdl
qdl_app_install:
	@meson install -C $(CURRENT_DIR)/qdl/build --destdir $(QDL_INSTALL_DIR)

.PHONY: qdl_app_clean
qdl_app_clean:
	@meson compile -C $(CURRENT_DIR)/qdl/build --clean
	@rm -rf $(CURRENT_DIR)/qdl/build
	@rm -rf $(QDL_INSTALL_DIR)

.PHONY: qdl
qdl: qdl_app_build qdl_app_install

.PHONY: unoq_linux_download
UNOQ_LINUX_URL := https://downloads.arduino.cc/debian-im/Stable/20260528-558/arduino-unoq-debian-image-20260528-558.tar.zst
unoq_linux_download:
	@mkdir -p $(CURRENT_DIR)/tmp
	@rm -rf $(CURRENT_DIR)/tmp/arduino-unoq-debian-image-tar.zst
	@wget -O $(CURRENT_DIR)/tmp/arduino-unoq-debian-image-tar.zst $(UNOQ_LINUX_URL)

.PHONY: unoq_linux_unpack
UNOQ_LINUX_DIR := $(CURRENT_DIR)/tmp/arduino-unoq-debian-image-????????-???
ESP_FILES := \
	$(UNOQ_LINUX_DIR)/disk-sdcard.img.root \
	$(UNOQ_LINUX_DIR)/disk-sdcard.img.home
unoq_linux_unpack:
	@rm -rf $(UNOQ_LINUX_DIR)
	@sudo umount $(MOUNT_DISK_DIR) || true
	@rm -rf $(MOUNT_DISK_DIR) || true
	@tar -xvf $(CURRENT_DIR)/tmp/arduino-unoq-debian-image-tar.zst -C $(CURRENT_DIR)/tmp
	@for file in $(ESP_FILES); do \
		rm -f $$file; \
		truncate -s 64M $$file; \
		mkfs.fat -F 32 $$file; \
	done
	@$(MAKE) unoq_linux_prepare_esp_mount

.PHONY: unoq_linux_prepare_esp_mount
MOUNT_DISK_DIR := $(CURRENT_DIR)/tmp/mount-disk-sdcard.img.esp
unoq_linux_prepare_esp_mount:
	@mkdir -p $(MOUNT_DISK_DIR)
	@sudo umount $(UNOQ_LINUX_DIR)/disk-sdcard.img.esp ||true
	@sudo mount -o loop,rw $(UNOQ_LINUX_DIR)/disk-sdcard.img.esp $(MOUNT_DISK_DIR)
	@sudo rm -rf $$(find $(MOUNT_DISK_DIR)/dtb -mindepth 1 -maxdepth 1 -type f,d ! -name 'qcom')
	@sudo rm -rf $(MOUNT_DISK_DIR)/EFI $(MOUNT_DISK_DIR)/loader
	@sudo find $(MOUNT_DISK_DIR) \
		-mindepth 1 -maxdepth 1 -type d \
		-regextype posix-extended -regex '.*/[0-9a-f]{32}' \
		-exec rm -rf -- {} +
	@sudo mv $(MOUNT_DISK_DIR)/dtb/qcom $(MOUNT_DISK_DIR) || true
	@sudo rm -rf $(MOUNT_DISK_DIR)/dtb || true
	@sudo umount $(MOUNT_DISK_DIR)
	@rm -rf $(MOUNT_DISK_DIR)
