LOCAL_PATH := $(call my-dir)
HERE_PATH := $(LOCAL_PATH)

LOCAL_PATH := $(HERE_PATH)


include $(CLEAR_VARS)
LOCAL_MODULE := regal
LOCAL_SRC_FILES := Regal/libRegal.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := spirv-cross
LOCAL_SRC_FILES := tinywrapper/SPIRVCross/libspirv-cross-c-shared.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := shaderc
LOCAL_SRC_FILES := tinywrapper/shaderc/libshaderc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tinywrapper
LOCAL_SHARED_LIBRARIES := regal spirv-cross shaderc
LOCAL_LDLIBS := -lGLESv3
LOCAL_SRC_FILES := tinywrapper/main.c tinywrapper/string_utils.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/tinywrapper
include $(BUILD_SHARED_LIBRARY)
