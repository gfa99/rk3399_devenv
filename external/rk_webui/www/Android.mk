LOCAL_PATH:= $(call my-dir)

etc_dir := $(TARGET_OUT)/etc/boa/www

#####home.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := home.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate liblollipop_config
LOCAL_MODULE := home.cgi
include $(BUILD_EXECUTABLE)

#####wifi.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := wifi.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate liblollipop_config
LOCAL_MODULE := wifi.cgi
include $(BUILD_EXECUTABLE)

#####scan.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := scan.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := scan.cgi
include $(BUILD_EXECUTABLE)

#####connect.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := connect.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := connect.cgi
include $(BUILD_EXECUTABLE)

#####add_network.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := add_network.c \
                html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := add_network.cgi
include $(BUILD_EXECUTABLE)

#####add_network_connect.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := add_network_connect.c \
                html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := add_network_connect.cgi
include $(BUILD_EXECUTABLE)

#####disconnect.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := disconnect.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := disconnect.cgi
include $(BUILD_EXECUTABLE)

#####forget.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := forget.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := forget.cgi
include $(BUILD_EXECUTABLE)

#####dialog.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dialog.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := dialog.cgi
include $(BUILD_EXECUTABLE)

#####connectSavedAp.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := connectSavedAp.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := connectSavedAp.cgi
include $(BUILD_EXECUTABLE)

#####key.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := key.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE := key.cgi
include $(BUILD_EXECUTABLE)

#####play_control.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := media_control.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE := media_control.cgi
include $(BUILD_EXECUTABLE)

#####settings.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := settings.c \

ifeq ($(strip $(BOARD_WITH_DONGLE_CHINESS_LANGUAGE)), true)
LOCAL_CFLAGS += -DSUPPORT_CHINESE_LANGUAGE
endif
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate liblollipop_config
LOCAL_MODULE := settings.cgi
include $(BUILD_EXECUTABLE)

#####overscan.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := overscan.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc \
				libcutils \
				liblollipop_web_operate \
				liblollipop_socket_ipc \
				liblollipop_config \

LOCAL_MODULE := overscan.cgi
include $(BUILD_EXECUTABLE)

#####language.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := language.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc \
				libcutils \
				liblollipop_web_operate \
				liblollipop_config \

LOCAL_MODULE := language.cgi
include $(BUILD_EXECUTABLE)

#####softap_freq.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := softap_freq.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc \
                                libcutils \
                                liblollipop_web_operate \
                                liblollipop_config \

LOCAL_MODULE := softap_freq.cgi
include $(BUILD_EXECUTABLE)

#####password.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := password.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc \
				libcutils \
				liblollipop_web_operate \
				liblollipop_config \

LOCAL_MODULE := password.cgi
include $(BUILD_EXECUTABLE)

#####ota_host.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := ota_host.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc \
				libcutils \
				liblollipop_web_operate \
				liblollipop_config \

LOCAL_MODULE := ota_host.cgi
include $(BUILD_EXECUTABLE)


#####test.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := test.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE := test.cgi
include $(BUILD_EXECUTABLE)

#####inputUrl.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := inputUrl.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE := inputUrl.cgi
include $(BUILD_EXECUTABLE)

#####playUrl.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := playUrl.c \
		html_string.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils liblollipop_web_operate
LOCAL_MODULE := playUrl.cgi
include $(BUILD_EXECUTABLE)

#####debug.cgi

include $(CLEAR_VARS)
LOCAL_SRC_FILES := debug.c \

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE := debug.cgi
include $(BUILD_EXECUTABLE)


