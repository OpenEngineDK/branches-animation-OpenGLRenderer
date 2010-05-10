// OpenGL texture frame
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Display/OpenGL/RenderCanvas.h>
#include <Renderers/IRenderer.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

RenderCanvas::RenderCanvas()
    : TextureCanvasBase()
    , IRenderCanvas()
    , init(false)
{
}

RenderCanvas::~RenderCanvas() {
    
}

void RenderCanvas::Handle(Display::InitializeEventArg arg) {
    if (init) return;
    CreateTexture();
    SetTextureWidth(arg.canvas.GetWidth());
    SetTextureHeight(arg.canvas.GetHeight());
    SetupTexture();
    ((IListener<Renderers::InitializeEventArg>*)renderer)->Handle(Renderers::InitializeEventArg(*this));
    init = true;
}

void RenderCanvas::Handle(ResizeEventArg arg) {
    SetTextureWidth(arg.canvas.GetWidth());
    SetTextureWidth(arg.canvas.GetWidth());
    SetupTexture();
}

void RenderCanvas::Handle(Display::ProcessEventArg arg) {
    #ifdef OE_SAFE
    if (renderer == NULL) throw new Exception("NULL renderer in RenderCanvas.");
    #endif
    ((IListener<Renderers::ProcessEventArg>*)renderer)
        ->Handle(Renderers::ProcessEventArg(*this, arg.start, arg.approx));
    CopyToTexture();
}

void RenderCanvas::Handle(DeinitializeEventArg arg) { 
    if (!init) return;
    ((IListener<Renderers::DeinitializeEventArg>*)renderer)->Handle(Renderers::DeinitializeEventArg(*this));
    init = false;
}

unsigned int RenderCanvas::GetWidth() const {
    return GetTextureWidth();
}

unsigned int RenderCanvas::GetHeight() const {
    return GetTextureHeight();
}
    
void RenderCanvas::SetWidth(const unsigned int width) {
    SetTextureWidth(width);
    if (init) SetupTexture();
}

void RenderCanvas::SetHeight(const unsigned int height) {
    SetTextureHeight(height);
    if (init) SetupTexture();
}
    
ITexture2DPtr RenderCanvas::GetTexture() {
    return tex;
}


} // NS OpenGL
} // NS Display
} // NS OpenEngine
