# Copyright (C) 2015 Texas Instruments
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

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), jacinto6))

include $(CLEAR_VARS)

LOCAL_MODULE := audio.jamr3.$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SRC_FILES := jamr3_audio_hw.c

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	system/media/audio_route/include \
	system/media/audio_utils/include \
	system/media/audio_effects/include

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libtinyalsa \
	libaudioutils \
	libaudioroute

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif
