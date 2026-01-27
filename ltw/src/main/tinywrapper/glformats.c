/**
 * Optimized for Mobile Performance by Gemini
 * Created by: artDev
 * Copyright (c) 2025 artDev, SerpentSpirale, CADIndie.
 * For use under LGPL-3.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <GLES3/gl3.h>
#include "egl.h"
#include "glformats.h"
#include "libraryinternal.h"

#ifndef INTERNAL
#define INTERNAL __attribute__((visibility("hidden")))
#endif

#ifndef GL_RGBA12
#define GL_RGBA12 0x805A
#endif
#ifndef GL_RGBA16
#define GL_RGBA16 0x805B
#endif

#ifndef GL_RGB12
#define GL_RGB12 0x8053
#endif
#ifndef GL_RGB16
#define GL_RGB16 0x8054
#endif

#ifndef GL_DEPTH_COMPONENT32
#define GL_DEPTH_COMPONENT32 0x81A7
#endif
#ifndef GL_DEPTH_COMPONENT32F
#define GL_DEPTH_COMPONENT32F 0x8CAC
#endif

#ifndef GL_R11F_G11F_B10F
#define GL_R11F_G11F_B10F 0x8C3A
#endif


static inline GLint pick_depth_internalformat(GLenum* type, bool* convert) {
    if (*type == GL_FLOAT) {
        return GL_DEPTH_COMPONENT24;
    }
    if (*type == GL_UNSIGNED_SHORT || *type == GL_UNSIGNED_INT) {
        return GL_DEPTH_COMPONENT24;
    }
    *convert = true;
    *type = GL_UNSIGNED_SHORT;
    return GL_DEPTH_COMPONENT16;
}

static inline GLint pick_depth_stencil_internalformat(GLenum* type, bool* convert) {
    if (*type == 0x84FA || *type == 0x8DAD /* GL_FLOAT_32_UNSIGNED_INT_24_8_REV */) {
        return GL_DEPTH24_STENCIL8;
    }
    *convert = true;
    *type = 0x84FA; 
    return GL_DEPTH24_STENCIL8;
}

static inline GLint pick_red_internalformat(GLenum* type, bool* convert) {
    switch(*type) {
        case GL_BYTE: return GL_R8_SNORM;
        case GL_UNSIGNED_BYTE: return GL_R8;
        case GL_HALF_FLOAT:
        case GL_FLOAT: return GL_R16F; 
        default: *convert = true; *type = GL_UNSIGNED_BYTE; return GL_R8;
    }
}

static inline GLint pick_rg_internalformat(GLenum* type, bool* convert) {
    switch(*type) {
        case GL_BYTE: return GL_RG8_SNORM;
        case GL_UNSIGNED_BYTE: return GL_RG8;
        case GL_HALF_FLOAT:
        case GL_FLOAT: return GL_RG16F;
        default: *convert = true; *type = GL_UNSIGNED_BYTE; return GL_RG8;
    }
}


void pick_format(GLint *internalformat, GLenum* type, GLenum* format) {
    GLint inf = *internalformat;

    if (inf == GL_RGBA12 || inf == GL_RGBA16 || inf == GL_RGBA16F || inf == GL_RGBA32F) {
        *internalformat = GL_RGBA8; 
    } 
    else if (inf == GL_DEPTH_COMPONENT32F || inf == GL_DEPTH_COMPONENT32 || inf == GL_DEPTH_COMPONENT) {
        *internalformat = GL_DEPTH_COMPONENT24;
    }
    else if (inf == GL_DEPTH32F_STENCIL8 || inf == GL_DEPTH_STENCIL) {
        *internalformat = GL_DEPTH24_STENCIL8;
    }
    else if (inf == GL_RGB8_SNORM || (inf >= GL_RGB8I && inf <= GL_RGB16I) || inf == GL_RGB32I || 
             inf == GL_RGB12 || inf == GL_RGB16 || inf == GL_RGB16F || inf == GL_RGB32F) {
        *internalformat = GL_R11F_G11F_B10F; 
    }
    else if (inf == GL_RGB8UI) {
        *internalformat = GL_RGB8;
    }

    inf = *internalformat;

    if (__builtin_expect(inf == GL_RGBA8, 1)) {
        *format = GL_RGBA;
        *type = GL_UNSIGNED_BYTE;
        return;
    }

    switch (inf) {
        case GL_RGB: 
        case GL_RGB8: 
            *format = GL_RGB; *type = GL_UNSIGNED_BYTE; break;
        
        case GL_LUMINANCE_ALPHA: *format = GL_LUMINANCE_ALPHA; *type = GL_UNSIGNED_BYTE; break;
        case GL_LUMINANCE:       *format = GL_LUMINANCE;       *type = GL_UNSIGNED_BYTE; break;
        case GL_ALPHA:           *format = GL_ALPHA;           *type = GL_UNSIGNED_BYTE; break;
        
        case GL_R8:   *format = GL_RED; *type = GL_UNSIGNED_BYTE; break;
        case GL_R16F: 
        case GL_R32F: *format = GL_RED; *type = GL_HALF_FLOAT; break;
        
        case GL_RG8:   *format = GL_RG; *type = GL_UNSIGNED_BYTE; break;
        case GL_RG16F: 
        case GL_RG32F: *format = GL_RG; *type = GL_HALF_FLOAT; break;
        
        case GL_R11F_G11F_B10F: 
        case GL_RGB16F: *format = GL_RGB; *type = GL_HALF_FLOAT; break;
        
        case GL_RGBA16F: *format = GL_RGBA; *type = GL_HALF_FLOAT; break;
        
        case GL_DEPTH_COMPONENT16: *format = GL_DEPTH_COMPONENT; *type = GL_UNSIGNED_SHORT; break;
        case GL_DEPTH_COMPONENT24: *format = GL_DEPTH_COMPONENT; *type = GL_UNSIGNED_INT; break;
        case GL_DEPTH_COMPONENT32F: *format = GL_DEPTH_COMPONENT; *type = GL_UNSIGNED_INT; break;
        
        case GL_DEPTH24_STENCIL8: *format = GL_DEPTH_STENCIL; *type = GL_UNSIGNED_INT_24_8; break;
        case GL_STENCIL_INDEX8:   *format = GL_STENCIL_INDEX; *type = GL_UNSIGNED_BYTE; break;
        
        default:
            break;
    }
}

INTERNAL void pick_internalformat(GLint *internalformat, GLenum* type, GLenum* format, GLvoid const** data) {
    if (__builtin_expect(*data == NULL, 1)) {
        pick_format(internalformat, type, format);
        return;
    }

    bool convert_data = false;
    GLint inf = *internalformat;

    if (inf == GL_DEPTH_COMPONENT32F || inf == GL_DEPTH_COMPONENT32) {
        *internalformat = GL_DEPTH_COMPONENT24;
        *type = GL_UNSIGNED_INT;
    } 
    else if (inf == GL_DEPTH_COMPONENT) {
        *internalformat = pick_depth_internalformat(type, &convert_data);
    }
    else if (inf == GL_DEPTH_STENCIL) {
        *internalformat = pick_depth_stencil_internalformat(type, &convert_data);
    }
    else if (inf == GL_RED) {
        *internalformat = pick_red_internalformat(type, &convert_data);
    }
    else if (inf == GL_RG) {
        *internalformat = pick_rg_internalformat(type, &convert_data);
    }
    else if (inf >= GL_R8I && inf <= GL_RGBA32UI) {
         if (inf <= GL_R32UI)        *format = GL_RED_INTEGER;
         else if (inf <= GL_RG32UI)  *format = GL_RG_INTEGER;
         else if (inf <= GL_RGB32UI) *format = GL_RGB_INTEGER;
         else                        *format = GL_RGBA_INTEGER;
    }

    if (*data != NULL && convert_data) {
    }
}
