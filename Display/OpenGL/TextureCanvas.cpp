// OpenGL texture frame
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Display/OpenGL/TextureCanvas.h>
#include <Meta/OpenGL.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

    TextureCanvas::TextureCanvas(ICanvas& canvas)
        : ctex(new CustomTexture())
        , tex(ITexture2DPtr(ctex))
    {
        ctex->width = canvas.GetWidth();
        ctex->height = canvas.GetHeight();
        // ctex->width = 800;
        // ctex->height = 600;
        ctex->SetID(-1); // make sure that a texture loader wont try to bind this texture
        ctex->channels = 4;
        ctex->format = Resources::RGBA;
    }

    TextureCanvas::~TextureCanvas() {

    }

    void TextureCanvas::SetupTexture() {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     ctex->width, ctex->height, 0, GL_BGRA, 
                     GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
        CHECK_FOR_GL_ERROR();
        resizeEvent.Notify(ResizeEventArg(*this));
    }

    void TextureCanvas::Handle(Display::InitializeEventArg arg) {
        logger.info << "INIT TEXTUREFRAME" << logger.end;
        ctex->width = arg.canvas.GetWidth();
        ctex->height = arg.canvas.GetHeight();
        glGenTextures(1, &ctex->id);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, ctex->id);
        CHECK_FOR_GL_ERROR();
        SetupTexture();
        initEvent.Notify(Display::InitializeEventArg(*this));
   }

    void TextureCanvas::Handle(ResizeEventArg arg) {
    }

    void TextureCanvas::Handle(RedrawEventArg arg) {
        redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));
        glBindTexture(GL_TEXTURE_2D, ctex->id);
        CHECK_FOR_GL_ERROR();
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ctex->width, ctex->height, 0);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_FOR_GL_ERROR();
    }

    void TextureCanvas::Handle(DeinitializeEventArg arg) {
 
    }

    unsigned int TextureCanvas::GetWidth() const {
        return ctex->width;
    }

    unsigned int TextureCanvas::GetHeight() const {
        return ctex->height;
    }
    
    unsigned int TextureCanvas::GetDepth() const {
        return 32;
    }

    void TextureCanvas::SetWidth(const unsigned int width) {
        ctex->width = width;
        SetupTexture();
    }

    void TextureCanvas::SetHeight(const unsigned int height) {
        ctex->height = height;
        SetupTexture();
    }
    
    void TextureCanvas::SetDepth(const unsigned int depth) {
        
    }

    ITexture2DPtr TextureCanvas::GetTexture() {
        return tex;
    }

} // NS OpenGL
} // NS Display
} // NS OpenEngine
