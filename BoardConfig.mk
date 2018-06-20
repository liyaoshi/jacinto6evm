#
# Copyright (C) 2011 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# These two variables are set first, so they can be overridden
# by BoardConfigVendor.mk
#BOARD_USES_GENERIC_AUDIO := true
#USE_CAMERA_STUB := true
OMAP_ENHANCEMENT := true

ifeq ($(OMAP_ENHANCEMENT),true)
#COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT
endif

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a15

ENABLE_CPUSETS := true

#BOARD_HAVE_BLUETOOTH := true
#BOARD_HAVE_BLUETOOTH_TI := true
#BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/ti/jacinto6evm/bluetooth
TARGET_NO_BOOTLOADER := true

BOARD_KERNEL_BASE := 0x80000000
#BOARD_KERNEL_CMDLINE := console=ttyO2,115200n8 mem=1024M androidboot.console=ttyO2 androidboot.hardware=jacinto6evmboard vram=20M omapfb.vram=0:16M
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x03000000

TARGET_NO_RADIOIMAGE := true
TARGET_BOARD_PLATFORM := jacinto6
TARGET_BOOTLOADER_BOARD_NAME := jacinto6evm

BOARD_EGL_CFG := device/ti/jacinto6evm/egl.cfg

USE_OPENGL_RENDERER := true

# Use mke2fs to create ext4 images
TARGET_USES_MKE2FS := true

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 805306368
BOARD_USERDATAIMAGE_PARTITION_SIZE := 2147483648
BOARD_CACHEIMAGE_PARTITION_SIZE := 268435456
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 4096

BOARD_VENDORIMAGE_PARTITION_SIZE := 268435456
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_COPY_OUT_VENDOR := vendor

TARGET_RECOVERY_FSTAB = device/ti/jacinto6evm/fstab.jacinto6evmboard
TARGET_RECOVERY_PIXEL_FORMAT := "BGRA_8888"
TARGET_RELEASETOOLS_EXTENSIONS := device/ti/jacinto6evm

# Connectivity - Wi-Fi
USES_TI_MAC80211 := true
ifeq ($(USES_TI_MAC80211),true)
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
WPA_SUPPLICANT_VERSION      := VER_0_8_X
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB  := lib_driver_cmd_wl12xx
BOARD_HOSTAPD_PRIVATE_LIB         := lib_driver_cmd_wl12xx
BOARD_WLAN_DEVICE           := wl12xx_mac80211
BOARD_SOFTAP_DEVICE         := wl12xx_mac80211
#COMMON_GLOBAL_CFLAGS += -DUSES_TI_MAC80211
#COMMON_GLOBAL_CFLAGS += -DANDROID_LIB_STUB
endif

BOARD_SEPOLICY_DIRS += \
	device/ti/jacinto6evm/sepolicy \
	packages/services/Car/car_product/sepolicy

BOARD_PROPERTY_OVERRIDES_SPLIT_ENABLED := true

# lidbrm driver
BOARD_GPU_DRIVERS := omapdrm

# DispSync vsync offsets in nanoseconds
VSYNC_EVENT_PHASE_OFFSET_NS := 7500000
SF_VSYNC_EVENT_PHASE_OFFSET_NS := 5000000

BOARD_VENDOR_KERNEL_MODULES := \
	${KERNELDIR}/kernel/net/8021q/8021q.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtables.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtable_broute.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtable_nat.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_mark_m.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtable_filter.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_log.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_mark.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_vlan.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_ip.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_pkttype.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_arp.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_nflog.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_limit.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_arpreply.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_redirect.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_stp.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_ip6.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_802_3.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_among.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_snat.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_dnat.ko \
	${KERNELDIR}/kernel/net/bridge/bridge.ko \
	${KERNELDIR}/kernel/net/bridge/br_netfilter.ko \
	${KERNELDIR}/kernel/net/ipv4/xfrm4_mode_beet.ko \
	${KERNELDIR}/kernel/net/ipv4/ah4.ko \
	${KERNELDIR}/kernel/net/ipv4/xfrm4_tunnel.ko \
	${KERNELDIR}/kernel/net/ipv4/xfrm4_mode_transport.ko \
	${KERNELDIR}/kernel/net/ipv4/ipcomp.ko \
	${KERNELDIR}/kernel/net/netfilter/xt_cpu.ko \
	${KERNELDIR}/kernel/net/netfilter/xt_sctp.ko \
	${KERNELDIR}/kernel/net/netfilter/xt_multiport.ko \
	${KERNELDIR}/kernel/net/sctp/sctp.ko \
	${KERNELDIR}/kernel/net/ipv6/ip6_tunnel.ko \
	${KERNELDIR}/kernel/net/dns_resolver/dns_resolver.ko \
	${KERNELDIR}/kernel/net/sched/sch_choke.ko \
	${KERNELDIR}/kernel/net/sched/sch_qfq.ko \
	${KERNELDIR}/kernel/net/sched/em_cmp.ko \
	${KERNELDIR}/kernel/net/sched/sch_ingress.ko \
	${KERNELDIR}/kernel/net/sched/sch_dsmark.ko \
	${KERNELDIR}/kernel/net/sched/cls_rsvp.ko \
	${KERNELDIR}/kernel/net/sched/sch_drr.ko \
	${KERNELDIR}/kernel/net/sched/act_simple.ko \
	${KERNELDIR}/kernel/net/sched/cls_flow.ko \
	${KERNELDIR}/kernel/net/sched/cls_tcindex.ko \
	${KERNELDIR}/kernel/net/sched/em_nbyte.ko \
	${KERNELDIR}/kernel/net/sched/act_gact.ko \
	${KERNELDIR}/kernel/net/sched/cls_rsvp6.ko \
	${KERNELDIR}/kernel/net/sched/sch_multiq.ko \
	${KERNELDIR}/kernel/net/sched/act_ipt.ko \
	${KERNELDIR}/kernel/net/sched/sch_gred.ko \
	${KERNELDIR}/kernel/net/sched/sch_fq_codel.ko \
	${KERNELDIR}/kernel/net/sched/act_mirred.ko \
	${KERNELDIR}/kernel/net/sched/act_csum.ko \
	${KERNELDIR}/kernel/net/sched/em_text.ko \
	${KERNELDIR}/kernel/net/sched/sch_netem.ko \
	${KERNELDIR}/kernel/net/sched/cls_basic.ko \
	${KERNELDIR}/kernel/net/sched/sch_prio.ko \
	${KERNELDIR}/kernel/net/sched/sch_codel.ko \
	${KERNELDIR}/kernel/net/sched/act_police.ko \
	${KERNELDIR}/kernel/net/sched/sch_mqprio.ko \
	${KERNELDIR}/kernel/net/sched/sch_tbf.ko \
	${KERNELDIR}/kernel/net/sched/sch_cbq.ko \
	${KERNELDIR}/kernel/net/sched/sch_hfsc.ko \
	${KERNELDIR}/kernel/net/sched/em_meta.ko \
	${KERNELDIR}/kernel/net/sched/sch_red.ko \
	${KERNELDIR}/kernel/net/sched/act_nat.ko \
	${KERNELDIR}/kernel/net/sched/act_pedit.ko \
	${KERNELDIR}/kernel/net/sched/cls_fw.ko \
	${KERNELDIR}/kernel/net/sched/sch_sfq.ko \
	${KERNELDIR}/kernel/net/sched/act_skbedit.ko \
	${KERNELDIR}/kernel/net/sched/cls_route.ko \
	${KERNELDIR}/kernel/net/sched/sch_teql.ko \
	${KERNELDIR}/kernel/net/sched/sch_sfb.ko \
	${KERNELDIR}/kernel/net/llc/llc.ko \
	${KERNELDIR}/kernel/net/802/stp.ko \
	${KERNELDIR}/kernel/net/802/psnap.ko \
	${KERNELDIR}/kernel/net/802/p8022.ko \
	${KERNELDIR}/kernel/fs/cifs/cifs.ko \
	${KERNELDIR}/kernel/fs/fscache/fscache.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/aes-arm-bs.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha2-arm-ce.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha512-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha1-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/aes-arm-ce.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/ghash-arm-ce.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/aes-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha1-arm-neon.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha256-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha1-arm-ce.ko \
	${KERNELDIR}/kernel/crypto/ablk_helper.ko \
	${KERNELDIR}/kernel/crypto/algif_hash.ko \
	${KERNELDIR}/kernel/crypto/cryptd.ko \
	${KERNELDIR}/kernel/crypto/tcrypt.ko \
	${KERNELDIR}/kernel/crypto/echainiv.ko \
	${KERNELDIR}/kernel/crypto/md4.ko \
	${KERNELDIR}/kernel/crypto/af_alg.ko \
	${KERNELDIR}/kernel/crypto/algif_skcipher.ko \
	${KERNELDIR}/kernel/crypto/sha512_generic.ko \
	${KERNELDIR}/kernel/drivers/net/wireless/ti/wl18xx/wl18xx.ko \
	${KERNELDIR}/kernel/drivers/net/wireless/ti/wlcore/wlcore.ko \
	${KERNELDIR}/kernel/drivers/net/wireless/ti/wlcore/wlcore_sdio.ko \
	${KERNELDIR}/kernel/drivers/net/usb/usbnet.ko \
	${KERNELDIR}/kernel/drivers/net/usb/cdc_ncm.ko \
	${KERNELDIR}/kernel/drivers/net/usb/r8152.ko \
	${KERNELDIR}/kernel/drivers/net/usb/pegasus.ko \
	${KERNELDIR}/kernel/drivers/net/usb/smsc95xx.ko \
	${KERNELDIR}/kernel/drivers/net/usb/cdc_ether.ko \
	${KERNELDIR}/kernel/drivers/net/mii.ko \
	${KERNELDIR}/kernel/drivers/leds/leds-tlc591xx.ko \
	${KERNELDIR}/kernel/drivers/gpio/gpio-pisosr.ko \
	${KERNELDIR}/kernel/drivers/watchdog/omap_wdt.ko \
	${KERNELDIR}/kernel/drivers/scsi/sr_mod.ko \
	${KERNELDIR}/kernel/drivers/scsi/sd_mod.ko \
	${KERNELDIR}/kernel/drivers/usb/musb/musb_hdrc.ko \
	${KERNELDIR}/kernel/drivers/usb/misc/usbtest.ko \
	${KERNELDIR}/kernel/drivers/usb/host/ehci-omap.ko \
	${KERNELDIR}/kernel/drivers/usb/class/cdc-acm.ko \
	${KERNELDIR}/kernel/drivers/usb/storage/usb-storage.ko \
	${KERNELDIR}/kernel/drivers/usb/serial/usbserial.ko \
	${KERNELDIR}/kernel/drivers/crypto/omap-aes-driver.ko \
	${KERNELDIR}/kernel/drivers/crypto/omap-des.ko \
	${KERNELDIR}/kernel/drivers/crypto/omap-sham.ko \
	${KERNELDIR}/kernel/drivers/bluetooth/btwilink.ko \
	${KERNELDIR}/kernel/drivers/ata/ahci_platform.ko \
	${KERNELDIR}/kernel/drivers/ata/libahci_platform.ko \
	${KERNELDIR}/kernel/drivers/ata/sata_mv.ko \
	${KERNELDIR}/kernel/drivers/ata/libahci.ko \
	${KERNELDIR}/kernel/drivers/cdrom/cdrom.ko \
	${KERNELDIR}/kernel/drivers/media/platform/soc_camera/soc_camera.ko \
	${KERNELDIR}/kernel/drivers/media/platform/soc_camera/soc_camera_platform.ko \
	${KERNELDIR}/kernel/drivers/media/platform/soc_camera/soc_mediabus.ko \
	${KERNELDIR}/kernel/drivers/media/usb/uvc/uvcvideo.ko \
	${KERNELDIR}/kernel/drivers/media/v4l2-core/videobuf2-vmalloc.ko \
	${KERNELDIR}/kernel/drivers/media/v4l2-core/videobuf-core.ko \
	${KERNELDIR}/kernel/drivers/input/misc/adxl34x-i2c.ko \
	${KERNELDIR}/kernel/drivers/input/misc/adxl34x-spi.ko \
	${KERNELDIR}/kernel/drivers/input/misc/adxl34x.ko \
	${KERNELDIR}/kernel/drivers/input/misc/rotary_encoder.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/ldc3001_ts.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/pixcir_i2c_ts.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/goodix.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/atmel_mxt_ts.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/st1232.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/edt-ft5x06.ko \
	${KERNELDIR}/kernel/drivers/input/matrix-keymap.ko \
	${KERNELDIR}/kernel/drivers/input/input-polldev.ko \
	${KERNELDIR}/kernel/drivers/input/keyboard/qt1070.ko \
	${KERNELDIR}/kernel/drivers/input/keyboard/matrix_keypad.ko \
	${KERNELDIR}/kernel/drivers/char/hw_random/rng-core.ko \
	${KERNELDIR}/kernel/drivers/char/hw_random/omap-rng.ko \
	${KERNELDIR}/kernel/lib/crc-itu-t.ko \
	${KERNELDIR}/kernel/lib/crc-ccitt.ko \
	${KERNELDIR}/kernel/lib/crc7.ko \


BOARD_RECOVERY_KERNEL_MODULES := \
	${KERNELDIR}/kernel/net/8021q/8021q.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtables.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtable_broute.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtable_nat.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_mark_m.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebtable_filter.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_log.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_mark.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_vlan.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_ip.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_pkttype.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_arp.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_nflog.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_limit.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_arpreply.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_redirect.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_stp.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_ip6.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_802_3.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_among.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_snat.ko \
	${KERNELDIR}/kernel/net/bridge/netfilter/ebt_dnat.ko \
	${KERNELDIR}/kernel/net/bridge/bridge.ko \
	${KERNELDIR}/kernel/net/bridge/br_netfilter.ko \
	${KERNELDIR}/kernel/net/ipv4/xfrm4_mode_beet.ko \
	${KERNELDIR}/kernel/net/ipv4/ah4.ko \
	${KERNELDIR}/kernel/net/ipv4/xfrm4_tunnel.ko \
	${KERNELDIR}/kernel/net/ipv4/xfrm4_mode_transport.ko \
	${KERNELDIR}/kernel/net/ipv4/ipcomp.ko \
	${KERNELDIR}/kernel/net/netfilter/xt_cpu.ko \
	${KERNELDIR}/kernel/net/netfilter/xt_sctp.ko \
	${KERNELDIR}/kernel/net/netfilter/xt_multiport.ko \
	${KERNELDIR}/kernel/net/sctp/sctp.ko \
	${KERNELDIR}/kernel/net/ipv6/ip6_tunnel.ko \
	${KERNELDIR}/kernel/net/dns_resolver/dns_resolver.ko \
	${KERNELDIR}/kernel/net/sched/sch_choke.ko \
	${KERNELDIR}/kernel/net/sched/sch_qfq.ko \
	${KERNELDIR}/kernel/net/sched/em_cmp.ko \
	${KERNELDIR}/kernel/net/sched/sch_ingress.ko \
	${KERNELDIR}/kernel/net/sched/sch_dsmark.ko \
	${KERNELDIR}/kernel/net/sched/cls_rsvp.ko \
	${KERNELDIR}/kernel/net/sched/sch_drr.ko \
	${KERNELDIR}/kernel/net/sched/act_simple.ko \
	${KERNELDIR}/kernel/net/sched/cls_flow.ko \
	${KERNELDIR}/kernel/net/sched/cls_tcindex.ko \
	${KERNELDIR}/kernel/net/sched/em_nbyte.ko \
	${KERNELDIR}/kernel/net/sched/act_gact.ko \
	${KERNELDIR}/kernel/net/sched/cls_rsvp6.ko \
	${KERNELDIR}/kernel/net/sched/sch_multiq.ko \
	${KERNELDIR}/kernel/net/sched/act_ipt.ko \
	${KERNELDIR}/kernel/net/sched/sch_gred.ko \
	${KERNELDIR}/kernel/net/sched/sch_fq_codel.ko \
	${KERNELDIR}/kernel/net/sched/act_mirred.ko \
	${KERNELDIR}/kernel/net/sched/act_csum.ko \
	${KERNELDIR}/kernel/net/sched/em_text.ko \
	${KERNELDIR}/kernel/net/sched/sch_netem.ko \
	${KERNELDIR}/kernel/net/sched/cls_basic.ko \
	${KERNELDIR}/kernel/net/sched/sch_prio.ko \
	${KERNELDIR}/kernel/net/sched/sch_codel.ko \
	${KERNELDIR}/kernel/net/sched/act_police.ko \
	${KERNELDIR}/kernel/net/sched/sch_mqprio.ko \
	${KERNELDIR}/kernel/net/sched/sch_tbf.ko \
	${KERNELDIR}/kernel/net/sched/sch_cbq.ko \
	${KERNELDIR}/kernel/net/sched/sch_hfsc.ko \
	${KERNELDIR}/kernel/net/sched/em_meta.ko \
	${KERNELDIR}/kernel/net/sched/sch_red.ko \
	${KERNELDIR}/kernel/net/sched/act_nat.ko \
	${KERNELDIR}/kernel/net/sched/act_pedit.ko \
	${KERNELDIR}/kernel/net/sched/cls_fw.ko \
	${KERNELDIR}/kernel/net/sched/sch_sfq.ko \
	${KERNELDIR}/kernel/net/sched/act_skbedit.ko \
	${KERNELDIR}/kernel/net/sched/cls_route.ko \
	${KERNELDIR}/kernel/net/sched/sch_teql.ko \
	${KERNELDIR}/kernel/net/sched/sch_sfb.ko \
	${KERNELDIR}/kernel/net/llc/llc.ko \
	${KERNELDIR}/kernel/net/802/stp.ko \
	${KERNELDIR}/kernel/net/802/psnap.ko \
	${KERNELDIR}/kernel/net/802/p8022.ko \
	${KERNELDIR}/kernel/fs/cifs/cifs.ko \
	${KERNELDIR}/kernel/fs/fscache/fscache.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/aes-arm-bs.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha2-arm-ce.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha512-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha1-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/aes-arm-ce.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/ghash-arm-ce.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/aes-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha1-arm-neon.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha256-arm.ko \
	${KERNELDIR}/kernel/arch/arm/crypto/sha1-arm-ce.ko \
	${KERNELDIR}/kernel/crypto/ablk_helper.ko \
	${KERNELDIR}/kernel/crypto/algif_hash.ko \
	${KERNELDIR}/kernel/crypto/cryptd.ko \
	${KERNELDIR}/kernel/crypto/tcrypt.ko \
	${KERNELDIR}/kernel/crypto/echainiv.ko \
	${KERNELDIR}/kernel/crypto/md4.ko \
	${KERNELDIR}/kernel/crypto/af_alg.ko \
	${KERNELDIR}/kernel/crypto/algif_skcipher.ko \
	${KERNELDIR}/kernel/crypto/sha512_generic.ko \
	${KERNELDIR}/kernel/drivers/net/wireless/ti/wl18xx/wl18xx.ko \
	${KERNELDIR}/kernel/drivers/net/wireless/ti/wlcore/wlcore.ko \
	${KERNELDIR}/kernel/drivers/net/wireless/ti/wlcore/wlcore_sdio.ko \
	${KERNELDIR}/kernel/drivers/net/usb/usbnet.ko \
	${KERNELDIR}/kernel/drivers/net/usb/cdc_ncm.ko \
	${KERNELDIR}/kernel/drivers/net/usb/r8152.ko \
	${KERNELDIR}/kernel/drivers/net/usb/pegasus.ko \
	${KERNELDIR}/kernel/drivers/net/usb/smsc95xx.ko \
	${KERNELDIR}/kernel/drivers/net/usb/cdc_ether.ko \
	${KERNELDIR}/kernel/drivers/net/mii.ko \
	${KERNELDIR}/kernel/drivers/leds/leds-tlc591xx.ko \
	${KERNELDIR}/kernel/drivers/gpio/gpio-pisosr.ko \
	${KERNELDIR}/kernel/drivers/watchdog/omap_wdt.ko \
	${KERNELDIR}/kernel/drivers/scsi/sr_mod.ko \
	${KERNELDIR}/kernel/drivers/scsi/sd_mod.ko \
	${KERNELDIR}/kernel/drivers/usb/musb/musb_hdrc.ko \
	${KERNELDIR}/kernel/drivers/usb/misc/usbtest.ko \
	${KERNELDIR}/kernel/drivers/usb/host/ehci-omap.ko \
	${KERNELDIR}/kernel/drivers/usb/class/cdc-acm.ko \
	${KERNELDIR}/kernel/drivers/usb/storage/usb-storage.ko \
	${KERNELDIR}/kernel/drivers/usb/serial/usbserial.ko \
	${KERNELDIR}/kernel/drivers/crypto/omap-aes-driver.ko \
	${KERNELDIR}/kernel/drivers/crypto/omap-des.ko \
	${KERNELDIR}/kernel/drivers/crypto/omap-sham.ko \
	${KERNELDIR}/kernel/drivers/bluetooth/btwilink.ko \
	${KERNELDIR}/kernel/drivers/ata/ahci_platform.ko \
	${KERNELDIR}/kernel/drivers/ata/libahci_platform.ko \
	${KERNELDIR}/kernel/drivers/ata/sata_mv.ko \
	${KERNELDIR}/kernel/drivers/ata/libahci.ko \
	${KERNELDIR}/kernel/drivers/cdrom/cdrom.ko \
	${KERNELDIR}/kernel/drivers/media/platform/soc_camera/soc_camera.ko \
	${KERNELDIR}/kernel/drivers/media/platform/soc_camera/soc_camera_platform.ko \
	${KERNELDIR}/kernel/drivers/media/platform/soc_camera/soc_mediabus.ko \
	${KERNELDIR}/kernel/drivers/media/usb/uvc/uvcvideo.ko \
	${KERNELDIR}/kernel/drivers/media/v4l2-core/videobuf2-vmalloc.ko \
	${KERNELDIR}/kernel/drivers/media/v4l2-core/videobuf-core.ko \
	${KERNELDIR}/kernel/drivers/input/misc/adxl34x-i2c.ko \
	${KERNELDIR}/kernel/drivers/input/misc/adxl34x-spi.ko \
	${KERNELDIR}/kernel/drivers/input/misc/adxl34x.ko \
	${KERNELDIR}/kernel/drivers/input/misc/rotary_encoder.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/ldc3001_ts.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/pixcir_i2c_ts.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/goodix.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/atmel_mxt_ts.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/st1232.ko \
	${KERNELDIR}/kernel/drivers/input/touchscreen/edt-ft5x06.ko \
	${KERNELDIR}/kernel/drivers/input/matrix-keymap.ko \
	${KERNELDIR}/kernel/drivers/input/input-polldev.ko \
	${KERNELDIR}/kernel/drivers/input/keyboard/qt1070.ko \
	${KERNELDIR}/kernel/drivers/input/keyboard/matrix_keypad.ko \
	${KERNELDIR}/kernel/drivers/char/hw_random/rng-core.ko \
	${KERNELDIR}/kernel/drivers/char/hw_random/omap-rng.ko \
	${KERNELDIR}/kernel/lib/crc-itu-t.ko \
	${KERNELDIR}/kernel/lib/crc-ccitt.ko \
	${KERNELDIR}/kernel/lib/crc7.ko \

