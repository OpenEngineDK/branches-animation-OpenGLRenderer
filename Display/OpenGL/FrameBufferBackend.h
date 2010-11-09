// OpenGL framebuffer backend for canvases
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_FRAME_BUFFER_BACKEND_H_
#define _OPENGL_FRAME_BUFFER_BACKEND_H_

#include <Display/ICanvasBackend.h>
#include <Meta/OpenGL.h>
#include <Math/Vector.h>

namespace OpenEngine {
    namespace Renderers{
        class IRenderer;
    }
    namespace Resources {
        class FrameBuffer;
    }
namespace Display {
namespace OpenGL {

class FrameBufferBackend : public ICanvasBackend {
private:
    Renderers::IRenderer* renderer; // ugly stuff to avoid writing explicit FBO loading code. A ya ya.

    GLint prevFb;
    Math::Vector<4, GLint> prevDims;
    
    Resources::FrameBuffer* fb;

public:
    FrameBufferBackend(Renderers::IRenderer* renderer, Resources::FrameBuffer* fbo = NULL);
    virtual ~FrameBufferBackend();
    void Create(unsigned int width, unsigned int height);
    void Init(unsigned int width, unsigned int height);
    void Deinit();
    void Pre();
    void Post();
    void SetDimensions(unsigned int width, unsigned int height);
    ICanvasBackend* Clone();
    Resources::ITexture2DPtr GetTexture();
};

}
}
}

#endif
