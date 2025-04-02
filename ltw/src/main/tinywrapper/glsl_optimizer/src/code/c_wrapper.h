//
// Created by serpentspirale on 17/06/23.
//



#ifndef GL4ES_C_WRAPPER_H
#define GL4ES_C_WRAPPER_H

#include "GL/gl.h"

#ifdef __cplusplus
extern "C" {
#endif

char* MesaConvertShader(const char * const *src, GLenum type, unsigned int glsl, unsigned int essl);
  
#ifdef __cplusplus
} /* extern C */
#endif


#endif //GL4ES_C_WRAPPER_H
