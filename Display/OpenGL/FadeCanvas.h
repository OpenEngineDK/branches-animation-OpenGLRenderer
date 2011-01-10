// Cross fade between canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Publicc License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_FADE_CANVAS_H_
#define _OPENGL_FADE_CANVAS_H_

#include <Display/ICanvas.h>
#include <Display/ICanvasBackend.h>
#include <Resources/ITexture2D.h>
#include <Display/OpenGL/BlendCanvas.h>
#include <list>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

using Display::ICanvas;
using Display::ICanvasBackend;
using Display::OpenGL::BlendCanvas;
using Resources::ITexture2DPtr;

using std::list;

class FadeCanvas: public ICanvas {
private:
    bool fade;
    float progress, duration;    
    BlendCanvas* bc;
    ICanvas *source, *target;
    list<ICanvas*> inits;
public:
    FadeCanvas(ICanvasBackend* backend);
    virtual ~FadeCanvas();
    
    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);
    void Handle(Display::ProcessEventArg arg);
    void Handle(Display::ResizeEventArg arg);
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    ITexture2DPtr GetTexture();

    void FadeIn(ICanvas* canvas, float duration);
    void FadeTo(ICanvas* canvas, float duration);

    void InitCanvas(ICanvas* canvas);
};

}
}
}
#endif //_OPENGL_FADE_CANVAS_H_
