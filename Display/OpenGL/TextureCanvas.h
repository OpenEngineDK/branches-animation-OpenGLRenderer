// OpenGL texture frame
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_TEXTURE_CANVAS_H_
#define _OPENGL_TEXTURE_CANVAS_H_

#include <Display/ICanvas.h>
#include <Display/IViewingVolume.h>
#include <Resources/ITexture2D.h>

#include <Core/IListener.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

using Core::IListener;
using Display::InitializeEventArg;
using Display::RedrawEventArg;
using Display::DeinitializeEventArg;

class TextureCanvas : public ICanvas, public ICanvasListener {
private:
    class CustomTexture : public ITexture2D {
        friend class TextureCanvas;
    private:
    public:
        unsigned int GetChannelSize() { return 8; };
        void Load() {}
        void Unload() {}
        void Reverse() {}
        void ReverseVertecally() {}
        void ReverseHorizontally() {}
    private:
    };
    CustomTexture* ctex;
    ITexture2DPtr tex;
    inline void SetupTexture();
public:
    // TextureCanvas(IFrame& frame);
    TextureCanvas(ICanvas& canvas);
    virtual ~TextureCanvas();

    void Handle(InitializeEventArg arg);
    void Handle(RedrawEventArg arg);
    void Handle(ResizeEventArg arg);
    void Handle(DeinitializeEventArg arg);

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    unsigned int GetDepth() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    void SetDepth(const unsigned int depth);
    ITexture2DPtr GetTexture();
};

} // NS OpenGL
} // NS Display
} // NS OpenEngine

#endif // _OPENGL_TEXTURE_CANVAS_H_
