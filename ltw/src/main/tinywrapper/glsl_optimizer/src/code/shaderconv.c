//
// Created by hanji on 2024/10/20.
//

#include "shaderconv.h"

#include "c_wrapper.h"

char* (*shaderconv_optimize_shader)(const char * const *src, GLenum type, unsigned int glsl, unsigned int essl);

__attribute((visibility("default"))) char* MesaConvertShader(const char *src, GLenum type, unsigned int glsl, unsigned int essl) {
    return shaderconv_optimize_shader(src, type, glsl, essl);
}
