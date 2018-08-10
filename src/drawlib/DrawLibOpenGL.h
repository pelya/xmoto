/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#ifndef __DRAWLIBOPENGL_H__
#define __DRAWLIBOPENGL_H__

#include "DrawLib.h"
#include "include/xm_OpenGL.h"

#if defined(__ANDROID__)

typedef void GL_APIENTRY (*PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void GL_APIENTRY (*PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void GL_APIENTRY (*PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void GL_APIENTRY (*PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef GLboolean GL_APIENTRY (*PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void GL_APIENTRY (*PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void GL_APIENTRY (*PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void GL_APIENTRY (*PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void GL_APIENTRY (*PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY (*PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean GL_APIENTRY (*PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void GL_APIENTRY (*PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void GL_APIENTRY (*PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void GL_APIENTRY (*PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum GL_APIENTRY (*PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void GL_APIENTRY (*PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void GL_APIENTRY (*PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void GL_APIENTRY (*PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void GL_APIENTRY (*PFNGLBINDATTRIBLOCATIONARBPROC) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef void GL_APIENTRY (*PFNGLGETACTIVEATTRIBARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint GL_APIENTRY (*PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void GL_APIENTRY (*PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);
typedef void GL_APIENTRY (*PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void GL_APIENTRY (*PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB GL_APIENTRY (*PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef GLhandleARB (*PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void GL_APIENTRY (*PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void GL_APIENTRY (*PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void GL_APIENTRY (*PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef void GL_APIENTRY (*PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);
typedef void GL_APIENTRY (*PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void GL_APIENTRY (*PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);

#endif

class DrawLibOpenGL : public DrawLib {
public:
  DrawLibOpenGL();
  virtual ~DrawLibOpenGL();

  virtual void init(unsigned int nDispWidth,
                    unsigned int nDispHeight,
                    unsigned int nDispBPP,
                    bool bWindowed);
  virtual void unInit();

  virtual void glVertexSP(float x, float y);
  virtual void glVertex(float x, float y);

  // texture coordinate
  virtual void glTexCoord(float x, float y);
  virtual void setColor(Color color);

  /**
   * set the texture for drawing
   * the value may be NULL to disable texture
   * every end draw will reset the texture to NULL
   **/
  virtual void setTexture(Texture *texture, BlendMode blendMode);
  virtual void setBlendMode(BlendMode blendMode);

  /**
   * enables clipping and sets the clipping borders
   **/
  virtual void setClipRect(int x, int y, unsigned int w, unsigned int h);
  virtual void setClipRect(SDL_Rect *i_clip_rect);
  virtual void setScale(float x, float y);
  virtual void setTranslate(float x, float y);
  virtual void setMirrorY();
  virtual void setRotateZ(float i_angle);
  virtual void setLineWidth(float width);

  /**
   * returns the current screen clipping
   **/
  virtual void getClipRect(int *o_px,
                           int *o_py,
                           int *o_pnWidth,
                           int *o_pnHeight);

  /**
   * Start drawing ... used in combination with glVertex
   **/
  virtual void startDraw(DrawMode mode);

  /**
   * End draw
   **/
  virtual void endDraw();
  virtual void endDrawKeepProperties(); /* to keep textures, ... to render
                                           several times the same entity fastly
                                           */
  virtual void removePropertiesAfterEnd(); /* remove properties endDraw =
                                              endDrawKeepProperties +
                                              removePropertiesAfterEnd */

  /**
   * Clears the screen with the configured background
   **/
  virtual void clearGraphics();

  /**
   * Flush the graphics. In memory graphics will now be displayed
   **/
  virtual void flushGraphics();

  virtual FontManager *getFontManager(const std::string &i_fontFile,
                                      unsigned int i_fontSize,
                                      unsigned int i_fixedFontSize = 0);

  virtual Img *grabScreen(int i_reduce = 1);
  virtual bool isExtensionSupported(std::string Ext);

  /* Extensions */
  PFNGLGENBUFFERSARBPROC glGenBuffersARB;
  PFNGLBINDBUFFERARBPROC glBindBufferARB;
  PFNGLBUFFERDATAARBPROC glBufferDataARB;
  PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;

  /* Extensions (for render-to-texture) */
  PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
  PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
  PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
  PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
  PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
  PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
  PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
  PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
  PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
  PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
  PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
  PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
  PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
  PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
  glGetFramebufferAttachmentParameterivEXT;
  PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;

  /* Extensions (for shaders) */
  PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;
  PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARB;
  PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB;
  PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
  PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
  PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
  PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
  PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
  PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
  PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
  PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB;
  PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
  PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
};

#endif
