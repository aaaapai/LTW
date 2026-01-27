/**
 * Optimized for PojavLauncher - Framebuffer Performance & Linker Fix
 * Created by: artDev
 * Modified by: Gemini (2026)
 */

#include "proc.h"
#include "egl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <GLES3/gl3.h>

#ifndef APIENTRY
#define APIENTRY
#endif

static framebuffer_t* get_framebuffer(GLenum target) {
    if(!current_context) return NULL;
    GLuint fb = 0;
    switch (target) {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER: fb = current_context->draw_framebuffer; break;
        case GL_READ_FRAMEBUFFER: fb = current_context->read_framebuffer; break;
        default: return NULL;
    }
    return (framebuffer_t*)unordered_map_get(current_context->framebuffer_map, (void*)(uintptr_t)fb);
}

static GLuint get_attachment_idx(GLenum attachment) {
    if(attachment >= GL_COLOR_ATTACHMENT0 && attachment < GL_COLOR_ATTACHMENT0 + MAX_DRAWBUFFERS) {
        return attachment - GL_COLOR_ATTACHMENT0;
    }
    return (GLuint)-1;
}

static GLenum map_attachment(framebuffer_t* framebuffer, GLenum attachment) {
    for(GLsizei i = 0; i < framebuffer->nbuffers; i++) {
        if(framebuffer->virt_drawbuffers[i] == attachment) {
            return (GLenum)(i + GL_COLOR_ATTACHMENT0);
        }
    }
    return GL_NONE;
}

void rebind_framebuffer(GLenum target, framebuffer_t *framebuffer, GLenum virt_attachment) {
    GLuint virt_index = get_attachment_idx(virt_attachment);
    if(virt_index == (GLuint)-1) return;
    
    GLenum phys_attachment = map_attachment(framebuffer, virt_attachment);
    if(phys_attachment == GL_NONE) return;

    GLuint obj = framebuffer->color_objects[virt_index];
    GLenum fb_target = framebuffer->color_targets[virt_index];

    if (fb_target == GL_TEXTURE_2D) {
        es3_functions.glFramebufferTexture2D(target, phys_attachment, GL_TEXTURE_2D, obj, framebuffer->color_levels[virt_index]);
    } else if (fb_target == GL_RENDERBUFFER) {
        es3_functions.glFramebufferRenderbuffer(target, phys_attachment, GL_RENDERBUFFER, obj);
    } else if (fb_target == GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER) {
        es3_functions.glFramebufferTextureLayer(target, phys_attachment, obj, framebuffer->color_levels[virt_index], framebuffer->color_layers[virt_index]);
    } else if (fb_target == GL_NONE) {
        es3_functions.glFramebufferRenderbuffer(target, phys_attachment, GL_RENDERBUFFER, 0);
    } else {
        es3_functions.glFramebufferTexture2D(target, phys_attachment, fb_target, obj, framebuffer->color_levels[virt_index]);
    }
}


void APIENTRY glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    if(!current_context) return;
    framebuffer_t *framebuffer = get_framebuffer(target);
    GLuint attachment_idx = get_attachment_idx(attachment);
    if(!framebuffer || attachment_idx == (GLuint)-1) {
        es3_functions.glFramebufferTextureLayer(target, attachment, texture, level, layer);
        return;
    }
    if(texture == 0) {
        framebuffer->color_targets[attachment_idx] = GL_NONE;
    } else {
        framebuffer->color_targets[attachment_idx] = GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER;
        framebuffer->color_objects[attachment_idx] = texture;
        framebuffer->color_levels[attachment_idx] = level;
        framebuffer->color_layers[attachment_idx] = layer;
    }
    rebind_framebuffer(target, framebuffer, attachment);
}

void APIENTRY glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    if(!current_context) return;
    framebuffer_t *framebuffer = get_framebuffer(target);
    GLuint attachment_idx = get_attachment_idx(attachment);
    if(!framebuffer || attachment_idx == (GLuint)-1) {
        es3_functions.glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
        return;
    }
    if(renderbuffer == 0) {
        framebuffer->color_targets[attachment_idx] = GL_NONE;
    } else {
        framebuffer->color_targets[attachment_idx] = renderbuffertarget;
        framebuffer->color_objects[attachment_idx] = renderbuffer;
    }
    rebind_framebuffer(target, framebuffer, attachment);
}

void APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params) {
    if(!current_context) return;
    framebuffer_t *framebuffer = get_framebuffer(target);
    GLuint attachment_idx = get_attachment_idx(attachment);
    if(!framebuffer || attachment_idx == (GLuint)-1) {
        es3_functions.glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
        return;
    }
    
    GLenum fb_target = framebuffer->color_targets[attachment_idx];
    switch (pname) {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
            if (fb_target == GL_NONE) *params = GL_NONE;
            else if (fb_target == GL_RENDERBUFFER) *params = GL_RENDERBUFFER;
            else *params = GL_TEXTURE;
            break;
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
            *params = (GLint)framebuffer->color_objects[attachment_idx];
            break;
        default:
            es3_functions.glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
            break;
    }
}


void APIENTRY glClearBufferfv(GLenum buffer, GLint drawBuffer, const GLfloat * value) {
    if (buffer == GL_COLOR) {
        framebuffer_t *framebuffer = get_framebuffer(GL_DRAW_FRAMEBUFFER);
        if(framebuffer) {
            GLenum attachment = map_attachment(framebuffer, GL_COLOR_ATTACHMENT0 + drawBuffer);
            if (attachment != GL_NONE) drawBuffer = attachment - GL_COLOR_ATTACHMENT0;
        }
    }
    es3_functions.glClearBufferfv(buffer, drawBuffer, value);
}

void APIENTRY glClearBufferiv(GLenum buffer, GLint drawBuffer, const GLint * value) {
    if (buffer == GL_COLOR) {
        framebuffer_t *framebuffer = get_framebuffer(GL_DRAW_FRAMEBUFFER);
        if(framebuffer) {
            GLenum attachment = map_attachment(framebuffer, GL_COLOR_ATTACHMENT0 + drawBuffer);
            if (attachment != GL_NONE) drawBuffer = attachment - GL_COLOR_ATTACHMENT0;
        }
    }
    es3_functions.glClearBufferiv(buffer, drawBuffer, value);
}

void APIENTRY glClearBufferuiv(GLenum buffer, GLint drawBuffer, const GLuint * value) {
    if (buffer == GL_COLOR) {
        framebuffer_t *framebuffer = get_framebuffer(GL_DRAW_FRAMEBUFFER);
        if(framebuffer) {
            GLenum attachment = map_attachment(framebuffer, GL_COLOR_ATTACHMENT0 + drawBuffer);
            if (attachment != GL_NONE) drawBuffer = attachment - GL_COLOR_ATTACHMENT0;
        }
    }
    es3_functions.glClearBufferuiv(buffer, drawBuffer, value);
}


void APIENTRY glDrawBuffers(GLsizei n, const GLenum* buffers) {
    if(!current_context) return;
    framebuffer_t *framebuffer = get_framebuffer(GL_DRAW_FRAMEBUFFER);
    if(!framebuffer) {
        es3_functions.glDrawBuffers(n, buffers);
        return;
    }

    framebuffer->nbuffers = n;
    memcpy(framebuffer->virt_drawbuffers, buffers, n * sizeof(GLenum));

    GLenum phys_drawbuffers[16]; 
    GLsizei count = (n > 16) ? 16 : n;

    for(GLsizei i = 0; i < count; i++) {
        GLenum buffer = buffers[i];
        if(buffer != GL_NONE) {
            rebind_framebuffer(GL_DRAW_FRAMEBUFFER, framebuffer, buffer);
            phys_drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        } else {
            phys_drawbuffers[i] = GL_NONE;
        }
    }
    es3_functions.glDrawBuffers(count, phys_drawbuffers);
}

void APIENTRY glDrawBuffer(GLenum buffer) {
    glDrawBuffers(1, &buffer);
}

GLenum APIENTRY glCheckFramebufferStatus(GLenum target) {
    if(!current_context) return GL_FRAMEBUFFER_UNDEFINED;
    return es3_functions.glCheckFramebufferStatus(target);
}

void APIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    if(!current_context) return;
    framebuffer_t *framebuffer = get_framebuffer(target);
    GLuint attachment_idx = get_attachment_idx(attachment);
    
    if(!framebuffer || attachment_idx == (GLuint)-1) {
        es3_functions.glFramebufferTexture2D(target, attachment, textarget, texture, level);
        return;
    }

    framebuffer->color_targets[attachment_idx] = (texture == 0) ? GL_NONE : textarget;
    framebuffer->color_objects[attachment_idx] = texture;
    framebuffer->color_levels[attachment_idx] = level;
    
    rebind_framebuffer(target, framebuffer, attachment);
}

void APIENTRY glGenFramebuffers(GLsizei n, GLuint* framebuffers) {
    if(!current_context) return;
    es3_functions.glGenFramebuffers(n, framebuffers);
    for(GLsizei i = 0; i < n; i++) {
        framebuffer_t* fb = (framebuffer_t*)calloc(1, sizeof(framebuffer_t));
        fb->nbuffers = 1;
        fb->virt_drawbuffers[0] = GL_COLOR_ATTACHMENT0;
        unordered_map_put(current_context->framebuffer_map, (void*)(uintptr_t)framebuffers[i], fb);
    }
}

void APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
    if(!current_context) return;
    es3_functions.glDeleteFramebuffers(n, framebuffers);
    for(GLsizei i = 0; i < n; i++) {
        framebuffer_t* fb = (framebuffer_t*)unordered_map_remove(current_context->framebuffer_map, (void*)(uintptr_t)framebuffers[i]);
        if(fb) free(fb);
    }
}

void APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer) {
    if(!current_context) return;
    if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER) {
        if (current_context->draw_framebuffer == framebuffer && target != GL_READ_FRAMEBUFFER) return;
        current_context->draw_framebuffer = framebuffer;
    }
    if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER) {
        current_context->read_framebuffer = framebuffer;
    }
    es3_functions.glBindFramebuffer(target, framebuffer);
}
