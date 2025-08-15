//
// Created by hanji on 2024/10/20.
//

#ifndef FOLD_CRAFT_LAUNCHER_SHADERCONV_H
#define FOLD_CRAFT_LAUNCHER_SHADERCONV_H

#include <GL/gl.h>
#include <string>

std::string MesaConvertShader(const char *src, GLenum type, unsigned int glsl, unsigned int essl);

#endif //FOLD_CRAFT_LAUNCHER_SHADERCONV_H
