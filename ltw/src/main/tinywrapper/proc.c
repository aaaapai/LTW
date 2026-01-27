/**
 * Optimized by Gemini for artDev.
 * Target: Snapdragon 778G / Adreno 642L
 * Goal: Maximize FPS by reducing symbol resolution overhead.
 */

#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <android/log.h>
#include <string.h>
#include <stdint.h>
#include "proc.h"
#include "egl.h"
#include "libraryinternal.h"

#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"
#include "GL/glext.h"

INTERNAL eglMustCastToProperFunctionPointerType (*host_eglGetProcAddress)(const char *procname);
INTERNAL es3_functions_t es3_functions;

// 错误处理函数保持不变
static void error_sysegl() {
    __android_log_print(ANDROID_LOG_ERROR, "LTWInit", "Failed to load system EGL: %s", dlerror());
    abort();
}

static void error_init(const char* functionName) {
    __android_log_print(ANDROID_LOG_ERROR, "LTWInit", "Failed to load function \"%s\"", functionName);
    abort();
}

static void init_es3_proc() {
#define GLESFUNC(name, type) es3_functions.name = (type)host_eglGetProcAddress(#name); if(es3_functions.name == NULL) error_init(#name);
#include "es3_functions.h"
#undef GLESFUNC
#define GLESFUNC(name, type) es3_functions.name = (type)host_eglGetProcAddress(#name);
#include "es3_extended.h"
#undef GLESFUNC
}

__attribute__((constructor, used)) void proc_init(){
    const char* systemEglPath = "libEGL.so";
    const char* eglPath = getenv("LIBGL_EGL") != NULL ? getenv("LIBGL_EGL") : systemEglPath;
    
    // 优化：使用 RTLD_NOW 减少运行时符号解析延迟
    void* eglHandle = dlopen(eglPath, RTLD_NOW | RTLD_LOCAL);
    if(eglHandle == NULL){
        eglHandle = dlopen(systemEglPath, RTLD_NOW | RTLD_LOCAL);
        if(eglHandle == NULL) error_sysegl();
    }
    host_eglGetProcAddress = dlsym(eglHandle, "eglGetProcAddress");
    if(host_eglGetProcAddress == NULL) error_sysegl();
    init_egl();
    init_es3_proc();
}

__attribute__((used)) eglMustCastToProperFunctionPointerType glXGetProcAddress(const char *procname) {
    return eglGetProcAddress(procname);
}

// 优化：消除 VLA，使用固定栈内存，并修复原代码的越界 Bug
static eglMustCastToProperFunctionPointerType resolve_stub(const char* procname) {
    char stub_procname[256];
    // 使用整数赋值写入 "stub_" 前缀 (小端序 'stub')
    *(uint32_t*)stub_procname = 0x62757473; 
    stub_procname[4] = '_';
    
    size_t i = 0;
    while (procname[i] && i < 250) {
        stub_procname[i + 5] = procname[i];
        i++;
    }
    stub_procname[i + 5] = '\0';
    return dlsym(RTLD_DEFAULT, stub_procname);
}

// 核心优化函数
eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname) {
    // 1. 极速过滤：非 gl/egl 开头的直接跳过
    if (__builtin_expect(!procname || procname[0] < 'e', 0)) goto fallback;

    // 将前 2 字节转为 16 位整数进行单次比对
    uint16_t prefix = *(uint16_t*)procname;

    // 2. 处理 EGL (0x6765 = 'eg')
    if (__builtin_expect(prefix == 0x6765, 0)) {
        if (procname[2] == 'l') {
            const char* p = procname + 3;
            if (!strcmp(p, "CreateContext")) return (void*)eglCreateContext;
            if (!strcmp(p, "DestroyContext")) return (void*)eglDestroyContext;
            if (!strcmp(p, "MakeCurrent")) return (void*)eglMakeCurrent;
        }
    }

    // 3. 处理 GL (0x6C67 = 'gl') - 绝大多数调用走这里
    if (__builtin_expect(prefix == 0x6C67, 1)) {
        // 获取第三个字符作为快速区分标志 (Quick-Fail)
        char p2 = procname[2];

#define GLESOVERRIDE(name)                                        \
        if (p2 == #name[2] && !strcmp(procname, #name)) {         \
            return (eglMustCastToProperFunctionPointerType) name; \
        }
#include "es3_overrides.h"
#undef GLESOVERRIDE
    }

fallback:;
    // 4. 分支预测：绝大多数函数驱动都能找到，resolve_stub 是极少数情况
    eglMustCastToProperFunctionPointerType function = host_eglGetProcAddress(procname);
    if (__builtin_expect(function == NULL, 0)) {
        function = resolve_stub(procname);
    }
    return function;
}
