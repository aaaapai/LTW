//
// Created by hanji on 2024/10/20.
//

#ifndef FOLD_CRAFT_LAUNCHER_SHADERCONV_H
#define FOLD_CRAFT_LAUNCHER_SHADERCONV_H

#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

std:: MesaConvertShader(const char *src, GLenum type, unsigned int glsl, unsigned int essl);

#ifdef __cplusplus
} /* extern C */
#endif

#endif //FOLD_CRAFT_LAUNCHER_SHADERCONV_H
