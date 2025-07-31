//
// Created by hanji on 2024/10/20.
//

#include "shaderconv.h"

#include "c_wrapper.h"

char* (*optimize_shader)(const char *src, GLenum type, unsigned int glsl, unsigned int essl);

__attribute((visibility("default"))) char* MesaConvertShader(const char *src, GLenum type, unsigned int glsl, unsigned int essl) {
    return optimize_shader(src, type, glsl, essl);
}
