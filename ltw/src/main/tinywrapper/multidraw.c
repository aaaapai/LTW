 * Created by: artDev
 * Optimized for Snapdragon 778G / Adreno GPU
 * Focus: High FPS without modifying external headers.
 */

#include <proc.h>
#include <egl.h>
#include "basevertex.h"

static inline GLsizeiptr get_type_size_fast(GLenum type) {
    if (type == GL_UNSIGNED_INT) return 4;
    if (type == GL_UNSIGNED_SHORT) return 2;
    if (type == GL_UNSIGNED_BYTE) return 1;
    return 4; 
}

void glMultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount) {
    if (__builtin_expect(!current_context, 0)) return;

    void (*draw_arrays)(GLenum, GLint, GLsizei) = es3_functions.glDrawArrays;

    for (int i = 0; i < primcount; i++) {
        GLsizei c = count[i];
        if (__builtin_expect(c > 0, 1)) {
            draw_arrays(mode, first[i], c);
        }
    }
}

void glMultiDrawElements(GLenum mode, GLsizei *count, GLenum type, const void * const *indices, GLsizei primcount) {
    if (__builtin_expect(!current_context, 0)) return;

    GLint elementbuffer = 0;
    es3_functions.glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementbuffer);

    void (*buf_data)(GLenum, GLsizeiptr, const void*, GLenum) = es3_functions.glBufferData;
    void (*buf_bind)(GLenum, GLuint) = es3_functions.glBindBuffer;
    void (*copy_sub)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) = es3_functions.glCopyBufferSubData;
    void (*buf_sub)(GLenum, GLintptr, GLsizeiptr, const void*) = es3_functions.glBufferSubData;

    buf_bind(GL_COPY_WRITE_BUFFER, current_context->multidraw_element_buffer);

    GLsizeiptr typebytes = get_type_size_fast(type);
    GLsizeiptr total = 0;
    
    for (GLsizei i = 0; i < primcount; i++) {
        total += count[i];
    }
    
    if (__builtin_expect(total == 0, 0)) return;

    buf_data(GL_COPY_WRITE_BUFFER, total * typebytes, NULL, GL_STREAM_DRAW);

    GLsizeiptr offset = 0;
    
    if (elementbuffer != 0) {
        for (GLsizei i = 0; i < primcount; i++) {
            GLsizeiptr ic = (GLsizeiptr)count[i] * typebytes;
            if (ic > 0) {
                copy_sub(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, (GLintptr)indices[i], offset, ic);
                offset += ic;
            }
        }
    } else {
        for (GLsizei i = 0; i < primcount; i++) {
            GLsizeiptr ic = (GLsizeiptr)count[i] * typebytes;
            if (ic > 0) {
                buf_sub(GL_COPY_WRITE_BUFFER, offset, ic, indices[i]);
                offset += ic;
            }
        }
    }

    buf_bind(GL_ELEMENT_ARRAY_BUFFER, current_context->multidraw_element_buffer);
    es3_functions.glDrawElements(mode, (GLsizei)total, type, (const void*)0);

    if (elementbuffer != 0) {
        buf_bind(GL_ELEMENT_ARRAY_BUFFER, (GLuint)elementbuffer);
    }
}
