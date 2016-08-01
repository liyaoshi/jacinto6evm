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

# Audio Post Processing Engine (APPE)
APPE_AUDIO := false

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/ti/jacinto6evm/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES := \
	$(LOCAL_KERNEL):kernel \
	device/ti/jacinto6evm/tablet_core_hardware_jacinto6evm.xml:system/etc/permissions/tablet_core_hardware_jacinto6evm.xml \
	device/ti/jacinto6evm/init.jacinto6evmboard.rc:root/init.jacinto6evmboard.rc \
	device/ti/jacinto6evm/init.jacinto6evmboard.usb.rc:root/init.jacinto6evmboard.usb.rc \
	device/ti/jacinto6evm/ueventd.jacinto6evmboard.rc:root/ueventd.jacinto6evmboard.rc \
	device/ti/jacinto6evm/fstab.jacinto6evmboard:root/fstab.jacinto6evmboard \
	device/ti/jacinto6evm/media_profiles.xml:system/etc/media_profiles.xml \
	device/ti/jacinto6evm/media_codecs.xml:system/etc/media_codecs.xml \
	frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
	frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
	frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
	device/ti/jacinto6evm/EP05120M09.idc:system/usr/idc/EP05120M09.idc \
	device/ti/jacinto6evm/Atmel_maXTouch_Touchscreen.idc:system/usr/idc/Atmel_maXTouch_Touchscreen.idc \
	device/ti/jacinto6evm/LDC_3001_TouchScreen_Controller.idc:system/usr/idc/LDC_3001_TouchScreen_Controller.idc \

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
	frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \

# Audio
ifeq ($(APPE_AUDIO),true)
PRODUCT_COPY_FILES += \
	hardware/ti/radio/vis_sdk/packages/android/hal/mixer_paths.xml:system/etc/mixer_paths.xml
else
PRODUCT_COPY_FILES += \
	device/ti/jacinto6evm/audio/primary/mixer_paths.xml:system/etc/mixer_paths.xml \
	device/ti/jacinto6evm/audio/jamr3/jamr3_mixer_paths.xml:system/etc/jamr3_mixer_paths.xml
endif

PRODUCT_COPY_FILES += \
	device/ti/jacinto6evm/audio/audio_policy.conf:system/etc/audio_policy.conf

# cpuset configuration
PRODUCT_COPY_FILES += \
	device/ti/jacinto6evm/init.jacinto6evmboard.cpuset.sh:system/bin/init.jacinto6evmboard.cpuset.sh

PRODUCT_PROPERTY_OVERRIDES := \
	hwui.render_dirty_regions=false

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mtp

PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=131072

PRODUCT_CHARACTERISTICS := tablet,nosdcard

DEVICE_PACKAGE_OVERLAYS := \
	device/ti/jacinto6evm/overlay

PRODUCT_PACKAGES += \
	com.android.future.usb.accessory

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160

# WI-Fi
PRODUCT_PACKAGES += \
	wpa_supplicant \
	wpa_supplicant.conf \
	wpa_supplicant_overlay.conf \
	libwpa_client \
	hostapd \
	hostapd.conf \
	dhcpcd.conf \
	wifical.sh \
	TQS_D_1.7.ini \
	TQS_D_1.7_127x.ini \
	crda \
	regulatory.bin \
	wlconf

PRODUCT_PACKAGES += \
	LegacyCamera \
	camera_test \
	ion_tiler_test \
	iontest \
	ion_ti_test2 \
	vpetest \
	modetest \
	libdrm

# Audio HAL modules
PRODUCT_PACKAGES += audio.primary.jacinto6
PRODUCT_PACKAGES += audio.hdmi.jacinto6
# BlueDroid a2dp Audio HAL module
PRODUCT_PACKAGES += audio.a2dp.default
# Remote submix
PRODUCT_PACKAGES += audio.r_submix.default
# JAMR3 Audio HAL module
ifneq ($(APPE_AUDIO),true)
PRODUCT_PACKAGES += audio.jamr3.jacinto6
endif

PRODUCT_PACKAGES += \
	tinymix \
	tinyplay \
	tinycap

# Radio
PRODUCT_PACKAGES += \
	RadioApp \
	lad_dra7xx \
	libtiipc \
	libtiipcutils \
	libtitransportrpmsg

# Launcher3
PRODUCT_PACKAGES += Launcher3

# Enable AAC 5.1 decode (decoder)
PRODUCT_PROPERTY_OVERRIDES += \
	media.aac_51_output_enabled=true

$(call inherit-product, frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk)
$(call inherit-product-if-exists, hardware/ti/dra7xx/jacinto6.mk)
#$(call inherit-product-if-exists, hardware/ti/wpan/ti-wpan-products.mk)
$(call inherit-product-if-exists, device/ti/proprietary-open/jacinto6/ti-jacinto6-vendor.mk)
$(call inherit-product-if-exists, device/ti/proprietary-open/jacinto6/ducati-full_jacinto6evm.mk)
$(call inherit-product-if-exists, device/ti/proprietary-open/wl12xx/wlan/wl12xx-wlan-fw-products.mk)
$(call inherit-product-if-exists, device/ti/proprietary-open/wl12xx/wpan/wl12xx-wpan-fw-products.mk)
