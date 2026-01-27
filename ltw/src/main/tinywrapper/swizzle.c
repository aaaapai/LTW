#include "proc.h"
#include "egl.h"
#include <string.h>
#include <stdlib.h>
#include "libraryinternal.h"

#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46


static inline void swizzle_process_bgra(GLenum* s) {
    GLenum tmp = s[0];
    s[0] = s[2];
    s[2] = tmp;
}

static inline void swizzle_process_endianness(GLenum* s) {
    GLenum t0 = s[0], t1 = s[1], t2 = s[2], t3 = s[3];
    s[0] = t3; s[1] = t2; s[2] = t1; s[3] = t0;
}

static texture_swizzle_track_t* get_swizzle_track(GLenum target) {
    GLenum getter = get_textarget_query_param(target);
    if (getter == 0) return NULL;

    GLint texture;
    es3_functions.glGetIntegerv(getter, &texture);
    
  
    texture_swizzle_track_t* track = unordered_map_get(current_context->texture_swztrack_map, (void*)(intptr_t)texture);
    
    if (track == NULL) {
        track = malloc(sizeof(texture_swizzle_track_t));
       
        track->upload_bgra = false;
        track->goofy_byte_order = false;
        
        
        es3_functions.glGetTexParameteriv(target, GL_TEXTURE_SWIZZLE_R, (GLint*)&track->original_swizzle[0]);
        es3_functions.glGetTexParameteriv(target, GL_TEXTURE_SWIZZLE_G, (GLint*)&track->original_swizzle[1]);
        es3_functions.glGetTexParameteriv(target, GL_TEXTURE_SWIZZLE_B, (GLint*)&track->original_swizzle[2]);
        es3_functions.glGetTexParameteriv(target, GL_TEXTURE_SWIZZLE_A, (GLint*)&track->original_swizzle[3]);
        unordered_map_put(current_context->texture_swztrack_map, (void*)(intptr_t)texture, track);
    }
    return track;
}

static void apply_swizzles(GLenum target, texture_swizzle_track_t* track) {
    GLenum final_swizzle[4];
    memcpy(final_swizzle, track->original_swizzle, 4 * sizeof(GLenum));
    
    if (track->goofy_byte_order) swizzle_process_endianness(final_swizzle);
    if (track->upload_bgra) swizzle_process_bgra(final_swizzle);

    es3_functions.glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, (GLint*)final_swizzle);
    
    for (int i = 0; i < 4; i++) {
        es3_functions.glTexParameteri(target, GL_TEXTURE_SWIZZLE_R + i, final_swizzle[i]);
    }
    */
}

INTERNAL void swizzle_process_upload(GLenum target, GLenum* format, GLenum* type) {
    bool is_bgra = (*format == GL_BGRA_EXT);
    bool is_goofy = (*type == 0x8035);
    bool is_rev_short = (*type == 0x8367);

    if (!is_bgra && !is_goofy && !is_rev_short) return;

    texture_swizzle_track_t* track = get_swizzle_track(target);
    if (track == NULL) return;

    if (is_bgra) *format = GL_RGBA;
    if (is_goofy || is_rev_short) *type = GL_UNSIGNED_BYTE;

    if (is_goofy != track->goofy_byte_order || is_bgra != track->upload_bgra) {
        track->goofy_byte_order = is_goofy;
        track->upload_bgra = is_bgra;
        apply_swizzles(target, track);
    }
}

INTERNAL void swizzle_process_swizzle_param(GLenum target, GLenum swizzle_param, const GLenum* swizzle) {
    if (swizzle_param < GL_TEXTURE_SWIZZLE_R || swizzle_param > GL_TEXTURE_SWIZZLE_RGBA) return;

    texture_swizzle_track_t* track = get_swizzle_track(target);
    if (track == NULL) return;

    if (swizzle_param == GL_TEXTURE_SWIZZLE_RGBA) {
        memcpy(track->original_swizzle, swizzle, 16);
    } else {
        track->original_swizzle[swizzle_param - GL_TEXTURE_SWIZZLE_R] = *swizzle;
    }
    
    apply_swizzles(target, track);
}
