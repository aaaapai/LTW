//
// Created by hanji on 2024/10/20.
//

#include <string>
#include "shaderconv.h"

#include "c_wrapper.h"

__attribute((visibility("default"))) std::string MesaConvertShader(const char *src, GLenum type, unsigned int glsl, unsigned int essl) {
    return optimize_shader(src, type, glsl, essl);
}
