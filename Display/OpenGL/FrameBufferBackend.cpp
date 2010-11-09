// OpenGL framebuffer backend for canvases
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Display/OpenGL/FrameBufferBackend.h>
#include <Renderers/IRenderer.h>
#include <Resources/FrameBuffer.h>
#include <Logging/Logger.h>

namespace OpenEngine {
    using namespace Math;
    using namespace Renderers;
    using namespace Resources;
namespace Display {
namespace OpenGL {

    FrameBufferBackend::FrameBufferBackend(IRenderer* renderer, FrameBuffer* fb)
        : renderer(renderer), prevFb(0), fb(fb) {}

    FrameBufferBackend::~FrameBufferBackend(){
        delete fb;
    }

    void FrameBufferBackend::Create(unsigned int width, unsigned int height){
    }

    void FrameBufferBackend::Init(unsigned int width, unsigned int height){
        if (fb == NULL)
            fb = new FrameBuffer(Vector<2, int>(width, height), 1, true);

        renderer->BindFrameBuffer(fb);
    }

    void FrameBufferBackend::Deinit(){

    }

    void FrameBufferBackend::Pre(){
        glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &prevFb);
        glGetIntegerv(GL_VIEWPORT, prevDims.ToArray());

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb->GetID());
        CHECK_FOR_GL_ERROR();
    }

    void FrameBufferBackend::Post(){
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, prevFb);

        Vector<2, int> dims = fb->GetDimension();
        glBlitFramebufferEXT(prevDims[0], prevDims[1], prevDims[2], prevDims[3], 
			     0, 0, dims[0], dims[1], 
			     GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBlitFramebufferEXT(prevDims[0], prevDims[1], prevDims[2], prevDims[3], 
			     0, 0, dims[0], dims[1], 
			     GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prevFb);
        CHECK_FOR_GL_ERROR();
    }

    void FrameBufferBackend::SetDimensions(unsigned int width, unsigned int height){
        throw Exception("Changing dimension on Render buffer backend not implemented!");
    }

    ICanvasBackend* FrameBufferBackend::Clone(){
        return new FrameBufferBackend(renderer, fb);
    }

    Resources::ITexture2DPtr FrameBufferBackend::GetTexture(){
        return fb->GetTexAttachment(0);
    }
}
}
}
