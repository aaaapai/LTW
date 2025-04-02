//
// Created by serpentspirale on 17/06/23.
//

#include <cstring>
#include "c_wrapper.h"
#include "GlslConvert.h"

GlslConvert::ShaderStage getStageForGlEnum(GLenum shader_type) {
    switch(shader_type) {
        case GL_VERTEX_SHADER: return GlslConvert::MESA_SHADER_VERTEX;
        case GL_FRAGMENT_SHADER: return GlslConvert::MESA_SHADER_FRAGMENT;
        case GL_COMPUTE_SHADER: return GlslConvert::MESA_SHADER_COMPUTE;
        case GL_GEOMETRY_SHADER: return GlslConvert::MESA_SHADER_GEOMETRY;
        case GL_TESS_CONTROL_SHADER: return GlslConvert::MESA_SHADER_TESS_CTRL;
        case GL_TESS_EVALUATION_SHADER: return GlslConvert::MESA_SHADER_TESS_EVAL;
        default: return GlslConvert::MESA_SHADER_NONE;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

GlslConvert::OptimizationStruct optimizationStruct {}; // Default struct with everything enabled



__attribute((visibility("default"))) char *optimize_shader(const char *src, GLenum type, unsigned int glsl, unsigned int essl) {
    GlslConvert& converter = GlslConvert::Instance();
    GlslConvert::ShaderStage stage = getStageForGlEnum(type);
    if(stage == GlslConvert::MESA_SHADER_NONE) {
        printf("Unknown shader type %x\n", type);
        return nullptr;
    }
    char * optimized_shader = converter.Optimize(
            src,
            stage,
            GlslConvert::API_OPENGL_CORE,
            GlslConvert::LANGUAGE_TARGET_GLSL,
            glsl,
            essl,
            true,
            optimizationStruct
            );
    if(converter.Failed()) {
        printf("Shader conversion failed!\n%s\n", converter.GetLog().c_str());
        return nullptr;
    }
    return optimized_shader;
}

#ifdef __cplusplus
}
#endif
