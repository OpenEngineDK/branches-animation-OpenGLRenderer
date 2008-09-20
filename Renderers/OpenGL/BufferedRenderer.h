// OpenGL buffered renderer implementation.
// renderers a scene to a 2d texture using a Frame Buffer Object (FBO)
//
// initial code based on:
// http://www.gamedev.net/reference/articles/article2331.asp
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _BUFFERED_RENDERER_H_
#define _BUFFERED_RENDERER_H_

#include <Meta/OpenGL.h>
#include <Renderers/OpenGL/Renderer.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

/**
 * BufferedRenderer using OpenGL
 *
 * @class BufferedRenderer BufferedRenderer.h Renderers/OpenGL/Renderer.h
 */
class BufferedRenderer : public Renderer {
public:
    BufferedRenderer();
    ~BufferedRenderer();

    void Handle(InitializeEventArg arg);
    void Handle(ProcessEventArg arg);
    void Handle(DeinitializeEventArg arg);
private:
    GLuint fbo, depthbuffer, img;
    unsigned int width, height;

    std::string EnumToString(GLenum status);
    void RenderTextureInOrtho();
};

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers

#endif // _BUFFERED_RENDERER_H_
