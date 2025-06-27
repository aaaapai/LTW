LOCAL_PATH := $(call my-dir)
HERE_PATH := $(LOCAL_PATH)

LOCAL_PATH := $(HERE_PATH)


include $(CLEAR_VARS)
LOCAL_MODULE := regal
LOCAL_SRC_FILES := Regal/libRegal.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := spirv-cross
LOCAL_SRC_FILES := SPIRVCross/libspirv-cross-c-shared.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := shaderc
LOCAL_SRC_FILES := shaderc/libshaderc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tinywrapper
LOCAL_SHARED_LIBRARIES := spirv-cross shaderc
LOCAL_LDLIBS := -lGLESv3 -lEGL
LOCAL_SRC_FILES := main.c string_utils.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/tinywrapper
include $(BUILD_SHARED_LIBRARY)
