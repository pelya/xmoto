#ifdef ENABLE_OPENGL
/* Pull in OpenGL headers */
/* following scissored from SDL_opengl.h */
#define __glext_h_ /* Don't let gl.h include glext.h */
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/gl.h> /* Header File For The OpenGL Library */
#include <OpenGL/glu.h> /* Header File For The GLU Library */
#elif defined(__MACOS__)
#include <gl.h> /* Header File For The OpenGL Library */
#include <glu.h> /* Header File For The GLU Library */
#elif defined(__ANDROID__)
#undef __glext_h_
#define GL_GLEXT_PROTOTYPES 1
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GL/glu.h> /* Not in NDK */
#else
#include <GL/gl.h> /* Header File For The OpenGL Library */
#include <GL/glu.h> /* Header File For The GLU Library */
#endif
#undef __glext_h_

#if defined(__ANDROID__)
#define glOrtho glOrthof
#define GLcharARB GLchar
#define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES

// Not available in GLES
typedef void * GLhandleARB;
#else
#include "xmoto/glext.h"
#endif

#endif
