#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <GLES3/gl3.h>


#include "unordered_map/unordered_map.h"
#include "string_utils.h"
#include "egl.h"
#include "proc.h"


extern char* optimize_shader(const char* source, GLenum type, int gles_ver, int shader_ver);

typedef struct {
    GLenum shader_type;
    const GLchar* source;
} shader_info_t;

typedef struct {
    GLuint frag_shader;
    GLchar* colorbindings[MAX_DRAWBUFFERS];
} program_info_t;



GLuint glCreateProgram(void) {
    if(!current_context) return 0;
    GLuint phys_program = es3_functions.glCreateProgram();
    if(phys_program == 0) return phys_program;
    program_info_t *prog_info = calloc(1, sizeof(program_info_t));
    if(prog_info == NULL) abort();
    unordered_map_put(current_context->program_map, (void*)(uintptr_t)phys_program, prog_info);
    return phys_program;
}

void glDeleteProgram(GLuint program) {
    if(!current_context) return;
    es3_functions.glDeleteProgram(program);
    program_info_t *old_info = unordered_map_remove(current_context->program_map, (void*)(uintptr_t)program);
    if(old_info) {
        for(int i = 0; i < MAX_DRAWBUFFERS; i++) if(old_info->colorbindings[i]) free(old_info->colorbindings[i]);
        free(old_info);
    }
}



static void insert_fragout_pos(char* source, int* size, const char* name, GLuint pos) {
    char src_string[256], dst_string[256];
    snprintf(src_string, sizeof(src_string), "/* LTW INSERT LOCATION %s LTW */", name);
    snprintf(dst_string, sizeof(dst_string), "layout(location = %u) ", pos);
    gl4es_inplace_replace_simple(source, size, src_string, dst_string);
}

void glLinkProgram(GLuint program) {
    if(!current_context) return;
    program_info_t* p_info = unordered_map_get(current_context->program_map, (void*)(uintptr_t)program);
    if(!p_info || p_info->frag_shader == 0) goto fallthrough;

    shader_info_t *s_info = unordered_map_get(current_context->shader_map, (void*)(uintptr_t)p_info->frag_shader);
    if(!s_info || !s_info->source) goto fallthrough;

    int nsrc_size = (int)(strlen(s_info->source) + 1);
    char* new_source = malloc(nsrc_size + 1024);
    memcpy(new_source, s_info->source, nsrc_size);

    bool changed = false;
    for(GLuint i = 0; i < MAX_DRAWBUFFERS; i++) {
        if(p_info->colorbindings[i]) {
            insert_fragout_pos(new_source, &nsrc_size, p_info->colorbindings[i], i);
            changed = true;
        }
    }

    if(!changed) { free(new_source); goto fallthrough; }

    GLuint patched = es3_functions.glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* const_src = (const GLchar*)new_source;
    es3_functions.glShaderSource(patched, 1, &const_src, NULL);
    es3_functions.glCompileShader(patched);
    
    es3_functions.glDetachShader(program, p_info->frag_shader);
    es3_functions.glAttachShader(program, patched);
    es3_functions.glLinkProgram(program);
    es3_functions.glDeleteShader(patched);
    free(new_source);
    return;

fallthrough:
    es3_functions.glLinkProgram(program);
}


void glShaderSource(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) {
    if(!current_context) return;
    shader_info_t* s_info = unordered_map_get(current_context->shader_map, (void*)(uintptr_t)shader);
    if(!s_info) {
        es3_functions.glShaderSource(shader, count, string, length);
        return;
    }

    size_t target_length = 0;
    for(GLsizei i = 0; i < count; i++) {
        target_length += (length && length[i] >= 0) ? length[i] : strlen(string[i]);
    }

    char* target_string = malloc(target_length + 1);
    size_t offset = 0;
    for(GLsizei i = 0; i < count; i++) {
        size_t l = (length && length[i] >= 0) ? length[i] : strlen(string[i]);
        memcpy(&target_string[offset], string[i], l);
        offset += l;
    }
    target_string[target_length] = '\0';

    
    if (s_info->shader_type == GL_FRAGMENT_SHADER) {
        char* p = strstr(target_string, "precision highp float;");
        if (p) memcpy(p, "precision mediump float;", 23);
    }

    
    GLchar* new_source = optimize_shader(target_string, s_info->shader_type, 460, current_context->shader_version);
    
    if(s_info->source) free((void*)s_info->source);
    s_info->source = new_source ? new_source : target_string;
    
    es3_functions.glShaderSource(shader, 1, &s_info->source, NULL);
    
    if(new_source && target_string) free(target_string);
}


void glAttachShader(GLuint program, GLuint shader) {
    if(!current_context) return;
    es3_functions.glAttachShader(program, shader);
    program_info_t* p_info = unordered_map_get(current_context->program_map, (void*)(uintptr_t)program);
    shader_info_t* s_info = unordered_map_get(current_context->shader_map, (void*)(uintptr_t)shader);
    if(p_info && s_info && s_info->shader_type == GL_FRAGMENT_SHADER) p_info->frag_shader = shader;
}

GLuint glCreateShader(GLenum type) {
    if(!current_context) return 0;
    GLuint phys = es3_functions.glCreateShader(type);
    shader_info_t* info = calloc(1, sizeof(shader_info_t));
    info->shader_type = type;
    unordered_map_put(current_context->shader_map, (void*)(uintptr_t)phys, info);
    return phys;
}

void glDeleteShader(GLuint shader) {
    if(!current_context) return;
    es3_functions.glDeleteShader(shader);
    shader_info_t *info = unordered_map_remove(current_context->shader_map, (void*)(uintptr_t)shader);
    if(info) {
        if(info->source) free((void*)info->source);
        free(info);
    }
}

void glBindFragDataLocation(GLuint program, GLuint color, const char* name) {
    if(!current_context) return;
    program_info_t *p_info = unordered_map_get(current_context->program_map, (void*)(uintptr_t)program);
    if(p_info && color < MAX_DRAWBUFFERS) {
        if(p_info->colorbindings[color]) free(p_info->colorbindings[color]);
        p_info->colorbindings[color] = strdup(name);
    }
}

void glGetShaderiv(GLuint shader, GLuint pname, GLint* params) {
    if(!current_context) return;
    shader_info_t* s_info = unordered_map_get(current_context->shader_map, (void*)(uintptr_t)shader);
    if(s_info && s_info->shader_type == GL_FRAGMENT_SHADER && pname == GL_COMPILE_STATUS) {
        *params = GL_TRUE;
        return;
    }
    es3_functions.glGetShaderiv(shader, pname, params);
}
