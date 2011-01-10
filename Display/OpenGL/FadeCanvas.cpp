// Cross fade between canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------


#include <Display/OpenGL/FadeCanvas.h>
#include <Math/Vector.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

using namespace Display::OpenGL;

FadeCanvas::FadeCanvas(ICanvasBackend* backend)
    : fade(false)
     , bc(new BlendCanvas(backend))
     , source(NULL)
     , target(NULL)
{
}

FadeCanvas::~FadeCanvas() {
    delete bc;
}

void FadeCanvas::Handle(Display::InitializeEventArg arg) {
    list<ICanvas*>::iterator i = inits.begin();
    for (; i != inits.end(); ++i) {
        ((IListener<Display::InitializeEventArg>*)*i)->Handle(arg);
    }
    bc->Handle(arg);
}
    
void FadeCanvas::Handle(Display::DeinitializeEventArg arg) {
    bc->Handle(arg);
    list<ICanvas*>::iterator i = inits.begin();
    for (; i != inits.end(); ++i) {
        ((IListener<Display::DeinitializeEventArg>*)*i)->Handle(arg);
    }
}

void FadeCanvas::Handle(Display::ProcessEventArg arg) {
    if (!target) return;
    ((IListener<Display::ProcessEventArg>*)target)->Handle(arg);
    if (!fade) return;
    progress += arg.approx * 1e-6;
    float scale = fmin(progress / duration, 1.0);
    bc->Clear();
    if (source) {
        ((IListener<Display::ProcessEventArg>*)source)->Handle(arg);
        bc->AddTexture(source->GetTexture(), 0, 0, Vector<4,float>(1.0));
    }
    bc->AddTexture(target->GetTexture(), 0, 0, Vector<4,float>(1.0, 1.0, 1.0, scale));
    bc->Handle(arg);
    if (progress > duration) {
        fade = false;
        source = NULL;
    }
}
    
void FadeCanvas::Handle(Display::ResizeEventArg arg) {
    bc->Handle(arg);
}

unsigned int FadeCanvas::GetWidth() const {
    return bc->GetWidth();
}

unsigned int FadeCanvas::GetHeight() const {
    return bc->GetHeight();
}
    
void FadeCanvas::SetWidth(const unsigned int width) {
    bc->SetWidth(width);
}

void FadeCanvas::SetHeight(const unsigned int height) {
    bc->SetHeight(height);
}
    
ITexture2DPtr FadeCanvas::GetTexture() {
    if (fade) 
        return bc->GetTexture();
    else 
        return target->GetTexture();
}

void FadeCanvas::FadeIn(ICanvas* canvas, float duration) {
    progress = 0.0;
    this->duration = duration;
    fade = true;
    source = NULL;
    target = canvas;
    bc->SetBackground(Vector<4,float>(0.0,0.0,0.0,1.0));      
}

void FadeCanvas::FadeTo(ICanvas* canvas, float duration) {
    progress = 0.0;
    this->duration = duration;
    fade = true;
    source = target;
    target = canvas;
}

void FadeCanvas::InitCanvas(ICanvas* canvas) {
    inits.push_back(canvas);
}

}
}
}
