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
	${KERNELDIR}/net/8021q/8021q.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtables.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtable_broute.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtable_nat.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_mark_m.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtable_filter.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_log.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_mark.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_vlan.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_ip.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_pkttype.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_arp.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_nflog.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_limit.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_arpreply.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_redirect.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_stp.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_ip6.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_802_3.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_among.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_snat.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_dnat.ko \
	${KERNELDIR}/net/bridge/bridge.ko \
	${KERNELDIR}/net/bridge/br_netfilter.ko \
	${KERNELDIR}/net/ipv4/xfrm4_mode_beet.ko \
	${KERNELDIR}/net/ipv4/ah4.ko \
	${KERNELDIR}/net/ipv4/xfrm4_tunnel.ko \
	${KERNELDIR}/net/ipv4/xfrm4_mode_transport.ko \
	${KERNELDIR}/net/ipv4/ipcomp.ko \
	${KERNELDIR}/net/netfilter/xt_cpu.ko \
	${KERNELDIR}/net/netfilter/xt_sctp.ko \
	${KERNELDIR}/net/netfilter/xt_multiport.ko \
	${KERNELDIR}/net/sctp/sctp.ko \
	${KERNELDIR}/net/ipv6/ip6_tunnel.ko \
	${KERNELDIR}/net/dns_resolver/dns_resolver.ko \
	${KERNELDIR}/net/sched/sch_choke.ko \
	${KERNELDIR}/net/sched/sch_qfq.ko \
	${KERNELDIR}/net/sched/em_cmp.ko \
	${KERNELDIR}/net/sched/sch_ingress.ko \
	${KERNELDIR}/net/sched/sch_dsmark.ko \
	${KERNELDIR}/net/sched/cls_rsvp.ko \
	${KERNELDIR}/net/sched/sch_drr.ko \
	${KERNELDIR}/net/sched/act_simple.ko \
	${KERNELDIR}/net/sched/cls_flow.ko \
	${KERNELDIR}/net/sched/cls_tcindex.ko \
	${KERNELDIR}/net/sched/em_nbyte.ko \
	${KERNELDIR}/net/sched/act_gact.ko \
	${KERNELDIR}/net/sched/cls_rsvp6.ko \
	${KERNELDIR}/net/sched/sch_multiq.ko \
	${KERNELDIR}/net/sched/act_ipt.ko \
	${KERNELDIR}/net/sched/sch_gred.ko \
	${KERNELDIR}/net/sched/sch_fq_codel.ko \
	${KERNELDIR}/net/sched/act_mirred.ko \
	${KERNELDIR}/net/sched/act_csum.ko \
	${KERNELDIR}/net/sched/em_text.ko \
	${KERNELDIR}/net/sched/sch_netem.ko \
	${KERNELDIR}/net/sched/cls_basic.ko \
	${KERNELDIR}/net/sched/sch_prio.ko \
	${KERNELDIR}/net/sched/sch_codel.ko \
	${KERNELDIR}/net/sched/act_police.ko \
	${KERNELDIR}/net/sched/sch_mqprio.ko \
	${KERNELDIR}/net/sched/sch_tbf.ko \
	${KERNELDIR}/net/sched/sch_cbq.ko \
	${KERNELDIR}/net/sched/sch_hfsc.ko \
	${KERNELDIR}/net/sched/em_meta.ko \
	${KERNELDIR}/net/sched/sch_red.ko \
	${KERNELDIR}/net/sched/act_nat.ko \
	${KERNELDIR}/net/sched/act_pedit.ko \
	${KERNELDIR}/net/sched/cls_fw.ko \
	${KERNELDIR}/net/sched/sch_sfq.ko \
	${KERNELDIR}/net/sched/act_skbedit.ko \
	${KERNELDIR}/net/sched/cls_route.ko \
	${KERNELDIR}/net/sched/sch_teql.ko \
	${KERNELDIR}/net/sched/sch_sfb.ko \
	${KERNELDIR}/net/llc/llc.ko \
	${KERNELDIR}/net/802/stp.ko \
	${KERNELDIR}/net/802/psnap.ko \
	${KERNELDIR}/net/802/p8022.ko \
	${KERNELDIR}/fs/cifs/cifs.ko \
	${KERNELDIR}/fs/fscache/fscache.ko \
	${KERNELDIR}/arch/arm/crypto/aes-arm-bs.ko \
	${KERNELDIR}/arch/arm/crypto/sha2-arm-ce.ko \
	${KERNELDIR}/arch/arm/crypto/sha512-arm.ko \
	${KERNELDIR}/arch/arm/crypto/sha1-arm.ko \
	${KERNELDIR}/arch/arm/crypto/aes-arm-ce.ko \
	${KERNELDIR}/arch/arm/crypto/ghash-arm-ce.ko \
	${KERNELDIR}/arch/arm/crypto/aes-arm.ko \
	${KERNELDIR}/arch/arm/crypto/sha1-arm-neon.ko \
	${KERNELDIR}/arch/arm/crypto/sha256-arm.ko \
	${KERNELDIR}/arch/arm/crypto/sha1-arm-ce.ko \
	${KERNELDIR}/crypto/ablk_helper.ko \
	${KERNELDIR}/crypto/algif_hash.ko \
	${KERNELDIR}/crypto/cryptd.ko \
	${KERNELDIR}/crypto/tcrypt.ko \
	${KERNELDIR}/crypto/echainiv.ko \
	${KERNELDIR}/crypto/md4.ko \
	${KERNELDIR}/crypto/af_alg.ko \
	${KERNELDIR}/crypto/algif_skcipher.ko \
	${KERNELDIR}/crypto/sha512_generic.ko \
	${KERNELDIR}/drivers/net/wireless/ti/wl18xx/wl18xx.ko \
	${KERNELDIR}/drivers/net/wireless/ti/wlcore/wlcore.ko \
	${KERNELDIR}/drivers/net/wireless/ti/wlcore/wlcore_sdio.ko \
	${KERNELDIR}/drivers/net/usb/usbnet.ko \
	${KERNELDIR}/drivers/net/usb/cdc_ncm.ko \
	${KERNELDIR}/drivers/net/usb/r8152.ko \
	${KERNELDIR}/drivers/net/usb/pegasus.ko \
	${KERNELDIR}/drivers/net/usb/smsc95xx.ko \
	${KERNELDIR}/drivers/net/usb/cdc_ether.ko \
	${KERNELDIR}/drivers/net/mii.ko \
	${KERNELDIR}/drivers/leds/leds-tlc591xx.ko \
	${KERNELDIR}/drivers/gpio/gpio-pisosr.ko \
	${KERNELDIR}/drivers/watchdog/omap_wdt.ko \
	${KERNELDIR}/drivers/scsi/sr_mod.ko \
	${KERNELDIR}/drivers/scsi/sd_mod.ko \
	${KERNELDIR}/drivers/usb/musb/musb_hdrc.ko \
	${KERNELDIR}/drivers/usb/misc/usbtest.ko \
	${KERNELDIR}/drivers/usb/host/ehci-omap.ko \
	${KERNELDIR}/drivers/usb/class/cdc-acm.ko \
	${KERNELDIR}/drivers/usb/storage/usb-storage.ko \
	${KERNELDIR}/drivers/usb/serial/usbserial.ko \
	${KERNELDIR}/drivers/crypto/omap-aes-driver.ko \
	${KERNELDIR}/drivers/crypto/omap-des.ko \
	${KERNELDIR}/drivers/crypto/omap-sham.ko \
	${KERNELDIR}/drivers/bluetooth/btwilink.ko \
	${KERNELDIR}/drivers/ata/ahci_platform.ko \
	${KERNELDIR}/drivers/ata/libahci_platform.ko \
	${KERNELDIR}/drivers/ata/sata_mv.ko \
	${KERNELDIR}/drivers/ata/libahci.ko \
	${KERNELDIR}/drivers/cdrom/cdrom.ko \
	${KERNELDIR}/drivers/media/platform/soc_camera/soc_camera.ko \
	${KERNELDIR}/drivers/media/platform/soc_camera/soc_camera_platform.ko \
	${KERNELDIR}/drivers/media/platform/soc_camera/soc_mediabus.ko \
	${KERNELDIR}/drivers/media/usb/uvc/uvcvideo.ko \
	${KERNELDIR}/drivers/media/v4l2-core/videobuf2-vmalloc.ko \
	${KERNELDIR}/drivers/media/v4l2-core/videobuf-core.ko \
	${KERNELDIR}/drivers/input/misc/adxl34x-i2c.ko \
	${KERNELDIR}/drivers/input/misc/adxl34x-spi.ko \
	${KERNELDIR}/drivers/input/misc/adxl34x.ko \
	${KERNELDIR}/drivers/input/misc/rotary_encoder.ko \
	${KERNELDIR}/drivers/input/touchscreen/ldc3001_ts.ko \
	${KERNELDIR}/drivers/input/touchscreen/pixcir_i2c_ts.ko \
	${KERNELDIR}/drivers/input/touchscreen/goodix.ko \
	${KERNELDIR}/drivers/input/touchscreen/atmel_mxt_ts.ko \
	${KERNELDIR}/drivers/input/touchscreen/st1232.ko \
	${KERNELDIR}/drivers/input/touchscreen/edt-ft5x06.ko \
	${KERNELDIR}/drivers/input/matrix-keymap.ko \
	${KERNELDIR}/drivers/input/input-polldev.ko \
	${KERNELDIR}/drivers/input/keyboard/qt1070.ko \
	${KERNELDIR}/drivers/input/keyboard/matrix_keypad.ko \
	${KERNELDIR}/drivers/char/hw_random/rng-core.ko \
	${KERNELDIR}/drivers/char/hw_random/omap-rng.ko \
	${KERNELDIR}/lib/crc-itu-t.ko \
	${KERNELDIR}/lib/crc-ccitt.ko \
	${KERNELDIR}/lib/crc7.ko \



BOARD_RECOVERY_KERNEL_MODULES := \
	${KERNELDIR}/net/8021q/8021q.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtables.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtable_broute.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtable_nat.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_mark_m.ko \
	${KERNELDIR}/net/bridge/netfilter/ebtable_filter.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_log.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_mark.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_vlan.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_ip.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_pkttype.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_arp.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_nflog.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_limit.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_arpreply.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_redirect.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_stp.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_ip6.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_802_3.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_among.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_snat.ko \
	${KERNELDIR}/net/bridge/netfilter/ebt_dnat.ko \
	${KERNELDIR}/net/bridge/bridge.ko \
	${KERNELDIR}/net/bridge/br_netfilter.ko \
	${KERNELDIR}/net/ipv4/xfrm4_mode_beet.ko \
	${KERNELDIR}/net/ipv4/ah4.ko \
	${KERNELDIR}/net/ipv4/xfrm4_tunnel.ko \
	${KERNELDIR}/net/ipv4/xfrm4_mode_transport.ko \
	${KERNELDIR}/net/ipv4/ipcomp.ko \
	${KERNELDIR}/net/netfilter/xt_cpu.ko \
	${KERNELDIR}/net/netfilter/xt_sctp.ko \
	${KERNELDIR}/net/netfilter/xt_multiport.ko \
	${KERNELDIR}/net/sctp/sctp.ko \
	${KERNELDIR}/net/ipv6/ip6_tunnel.ko \
	${KERNELDIR}/net/dns_resolver/dns_resolver.ko \
	${KERNELDIR}/net/sched/sch_choke.ko \
	${KERNELDIR}/net/sched/sch_qfq.ko \
	${KERNELDIR}/net/sched/em_cmp.ko \
	${KERNELDIR}/net/sched/sch_ingress.ko \
	${KERNELDIR}/net/sched/sch_dsmark.ko \
	${KERNELDIR}/net/sched/cls_rsvp.ko \
	${KERNELDIR}/net/sched/sch_drr.ko \
	${KERNELDIR}/net/sched/act_simple.ko \
	${KERNELDIR}/net/sched/cls_flow.ko \
	${KERNELDIR}/net/sched/cls_tcindex.ko \
	${KERNELDIR}/net/sched/em_nbyte.ko \
	${KERNELDIR}/net/sched/act_gact.ko \
	${KERNELDIR}/net/sched/cls_rsvp6.ko \
	${KERNELDIR}/net/sched/sch_multiq.ko \
	${KERNELDIR}/net/sched/act_ipt.ko \
	${KERNELDIR}/net/sched/sch_gred.ko \
	${KERNELDIR}/net/sched/sch_fq_codel.ko \
	${KERNELDIR}/net/sched/act_mirred.ko \
	${KERNELDIR}/net/sched/act_csum.ko \
	${KERNELDIR}/net/sched/em_text.ko \
	${KERNELDIR}/net/sched/sch_netem.ko \
	${KERNELDIR}/net/sched/cls_basic.ko \
	${KERNELDIR}/net/sched/sch_prio.ko \
	${KERNELDIR}/net/sched/sch_codel.ko \
	${KERNELDIR}/net/sched/act_police.ko \
	${KERNELDIR}/net/sched/sch_mqprio.ko \
	${KERNELDIR}/net/sched/sch_tbf.ko \
	${KERNELDIR}/net/sched/sch_cbq.ko \
	${KERNELDIR}/net/sched/sch_hfsc.ko \
	${KERNELDIR}/net/sched/em_meta.ko \
	${KERNELDIR}/net/sched/sch_red.ko \
	${KERNELDIR}/net/sched/act_nat.ko \
	${KERNELDIR}/net/sched/act_pedit.ko \
	${KERNELDIR}/net/sched/cls_fw.ko \
	${KERNELDIR}/net/sched/sch_sfq.ko \
	${KERNELDIR}/net/sched/act_skbedit.ko \
	${KERNELDIR}/net/sched/cls_route.ko \
	${KERNELDIR}/net/sched/sch_teql.ko \
	${KERNELDIR}/net/sched/sch_sfb.ko \
	${KERNELDIR}/net/llc/llc.ko \
	${KERNELDIR}/net/802/stp.ko \
	${KERNELDIR}/net/802/psnap.ko \
	${KERNELDIR}/net/802/p8022.ko \
	${KERNELDIR}/fs/cifs/cifs.ko \
	${KERNELDIR}/fs/fscache/fscache.ko \
	${KERNELDIR}/arch/arm/crypto/aes-arm-bs.ko \
	${KERNELDIR}/arch/arm/crypto/sha2-arm-ce.ko \
	${KERNELDIR}/arch/arm/crypto/sha512-arm.ko \
	${KERNELDIR}/arch/arm/crypto/sha1-arm.ko \
	${KERNELDIR}/arch/arm/crypto/aes-arm-ce.ko \
	${KERNELDIR}/arch/arm/crypto/ghash-arm-ce.ko \
	${KERNELDIR}/arch/arm/crypto/aes-arm.ko \
	${KERNELDIR}/arch/arm/crypto/sha1-arm-neon.ko \
	${KERNELDIR}/arch/arm/crypto/sha256-arm.ko \
	${KERNELDIR}/arch/arm/crypto/sha1-arm-ce.ko \
	${KERNELDIR}/crypto/ablk_helper.ko \
	${KERNELDIR}/crypto/algif_hash.ko \
	${KERNELDIR}/crypto/cryptd.ko \
	${KERNELDIR}/crypto/tcrypt.ko \
	${KERNELDIR}/crypto/echainiv.ko \
	${KERNELDIR}/crypto/md4.ko \
	${KERNELDIR}/crypto/af_alg.ko \
	${KERNELDIR}/crypto/algif_skcipher.ko \
	${KERNELDIR}/crypto/sha512_generic.ko \
	${KERNELDIR}/drivers/net/wireless/ti/wl18xx/wl18xx.ko \
	${KERNELDIR}/drivers/net/wireless/ti/wlcore/wlcore.ko \
	${KERNELDIR}/drivers/net/wireless/ti/wlcore/wlcore_sdio.ko \
	${KERNELDIR}/drivers/net/usb/usbnet.ko \
	${KERNELDIR}/drivers/net/usb/cdc_ncm.ko \
	${KERNELDIR}/drivers/net/usb/r8152.ko \
	${KERNELDIR}/drivers/net/usb/pegasus.ko \
	${KERNELDIR}/drivers/net/usb/smsc95xx.ko \
	${KERNELDIR}/drivers/net/usb/cdc_ether.ko \
	${KERNELDIR}/drivers/net/mii.ko \
	${KERNELDIR}/drivers/leds/leds-tlc591xx.ko \
	${KERNELDIR}/drivers/gpio/gpio-pisosr.ko \
	${KERNELDIR}/drivers/watchdog/omap_wdt.ko \
	${KERNELDIR}/drivers/scsi/sr_mod.ko \
	${KERNELDIR}/drivers/scsi/sd_mod.ko \
	${KERNELDIR}/drivers/usb/musb/musb_hdrc.ko \
	${KERNELDIR}/drivers/usb/misc/usbtest.ko \
	${KERNELDIR}/drivers/usb/host/ehci-omap.ko \
	${KERNELDIR}/drivers/usb/class/cdc-acm.ko \
	${KERNELDIR}/drivers/usb/storage/usb-storage.ko \
	${KERNELDIR}/drivers/usb/serial/usbserial.ko \
	${KERNELDIR}/drivers/crypto/omap-aes-driver.ko \
	${KERNELDIR}/drivers/crypto/omap-des.ko \
	${KERNELDIR}/drivers/crypto/omap-sham.ko \
	${KERNELDIR}/drivers/bluetooth/btwilink.ko \
	${KERNELDIR}/drivers/ata/ahci_platform.ko \
	${KERNELDIR}/drivers/ata/libahci_platform.ko \
	${KERNELDIR}/drivers/ata/sata_mv.ko \
	${KERNELDIR}/drivers/ata/libahci.ko \
	${KERNELDIR}/drivers/cdrom/cdrom.ko \
	${KERNELDIR}/drivers/media/platform/soc_camera/soc_camera.ko \
	${KERNELDIR}/drivers/media/platform/soc_camera/soc_camera_platform.ko \
	${KERNELDIR}/drivers/media/platform/soc_camera/soc_mediabus.ko \
	${KERNELDIR}/drivers/media/usb/uvc/uvcvideo.ko \
	${KERNELDIR}/drivers/media/v4l2-core/videobuf2-vmalloc.ko \
	${KERNELDIR}/drivers/media/v4l2-core/videobuf-core.ko \
	${KERNELDIR}/drivers/input/misc/adxl34x-i2c.ko \
	${KERNELDIR}/drivers/input/misc/adxl34x-spi.ko \
	${KERNELDIR}/drivers/input/misc/adxl34x.ko \
	${KERNELDIR}/drivers/input/misc/rotary_encoder.ko \
	${KERNELDIR}/drivers/input/touchscreen/ldc3001_ts.ko \
	${KERNELDIR}/drivers/input/touchscreen/pixcir_i2c_ts.ko \
	${KERNELDIR}/drivers/input/touchscreen/goodix.ko \
	${KERNELDIR}/drivers/input/touchscreen/atmel_mxt_ts.ko \
	${KERNELDIR}/drivers/input/touchscreen/st1232.ko \
	${KERNELDIR}/drivers/input/touchscreen/edt-ft5x06.ko \
	${KERNELDIR}/drivers/input/matrix-keymap.ko \
	${KERNELDIR}/drivers/input/input-polldev.ko \
	${KERNELDIR}/drivers/input/keyboard/qt1070.ko \
	${KERNELDIR}/drivers/input/keyboard/matrix_keypad.ko \
	${KERNELDIR}/drivers/char/hw_random/rng-core.ko \
	${KERNELDIR}/drivers/char/hw_random/omap-rng.ko \
	${KERNELDIR}/lib/crc-itu-t.ko \
	${KERNELDIR}/lib/crc-ccitt.ko \
	${KERNELDIR}/lib/crc7.ko \

