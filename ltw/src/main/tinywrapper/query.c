#include <GL/gl.h>
#include <GL/glext.h>
#include "egl.h"
#include "proc.h"

static inline bool check_ctx_fast() {
    return __builtin_expect(current_context != NULL, 1);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, int64_t* params) {
    if (!check_ctx_fast()) return;

   
    if (__builtin_expect(!current_context->timer_query, 0)) {
        if (params) *params = 1;
        return;
    }

  
    es3_functions.glGetQueryObjecti64vEXT(id, pname, params);
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, uint64_t* params) {
    if (!check_ctx_fast()) return;

    if (__builtin_expect(!current_context->timer_query, 0)) {
        if (params) *params = 1;
        return;
    }

    es3_functions.glGetQueryObjectui64vEXT(id, pname, params);
}

void glQueryCounter(GLuint id, GLenum target) {
   
    if (__builtin_expect(current_context && current_context->timer_query, 1)) {
        es3_functions.glQueryCounterEXT(id, target);
    }
}

void glGetQueryObjectiv(GLuint id, GLenum name, GLint * params) {
    if (!check_ctx_fast()) return;

  
    es3_functions.glGetQueryObjectuiv(id, name, (GLuint*)params);
}
