// OpenGL render canvas implementation
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_TEXTURE_COPY_RENDER_CANVAS_H_
#define _OPENGL_TEXTURE_COPY_RENDER_CANVAS_H_

#include <Display/IRenderCanvas.h>
#include <Display/OpenGL/TextureCanvasBase.h>
#include <Display/IViewingVolume.h>
#include <Resources/ITexture2D.h>

#include <Core/IListener.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

class RenderCanvas : public TextureCanvasBase, public IRenderCanvas {
private:
public:
    RenderCanvas();
    virtual ~RenderCanvas();

    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::ProcessEventArg arg);
    void Handle(Display::ResizeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    ITexture2DPtr GetTexture();
};

} // NS OpenGL
} // NS Display
} // NS OpenEngine

#endif // _OPENGL_TEXTURE_COPY_RENDER_CANVAS_H_
