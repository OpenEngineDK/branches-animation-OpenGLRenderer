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
{
}

RenderCanvas::~RenderCanvas() {
    
}

void RenderCanvas::Handle(Display::InitializeEventArg arg) {
    CreateTexture();
    SetTextureWidth(arg.canvas.GetWidth());
    SetTextureHeight(arg.canvas.GetHeight());
    SetupTexture();
    ((IListener<Renderers::InitializeEventArg>*)renderer)->Handle(Renderers::InitializeEventArg(*this));
}

void RenderCanvas::Handle(ResizeEventArg arg) {
}

void RenderCanvas::Handle(Display::ProcessEventArg arg) {
        ((IListener<Renderers::ProcessEventArg>*)renderer)
            ->Handle(Renderers::ProcessEventArg(*this, arg.start, arg.approx));
        CopyToTexture();
}

void RenderCanvas::Handle(DeinitializeEventArg arg) { 
    ((IListener<Renderers::DeinitializeEventArg>*)renderer)->Handle(Renderers::DeinitializeEventArg(*this));
}

unsigned int RenderCanvas::GetWidth() const {
    return GetTextureWidth();
}

unsigned int RenderCanvas::GetHeight() const {
    return GetTextureHeight();
}
    
void RenderCanvas::SetWidth(const unsigned int width) {
    SetTextureWidth(width);
}

void RenderCanvas::SetHeight(const unsigned int height) {
    SetTextureHeight(height);
}
    
ITexture2DPtr RenderCanvas::GetTexture() {
    return tex;
}


} // NS OpenGL
} // NS Display
} // NS OpenEngine
