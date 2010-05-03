// OpenGL split screen stero rendering link
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Display/OpenGL/SplitStereoCanvas.h>

#include <Display/StereoCamera.h>
#include <Display/ViewingVolume.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

SplitStereoCanvas::SplitStereoCanvas()
    : IRenderCanvas()
    , dummyCam(new ViewingVolume())
    , stereoCam(new StereoCamera(*dummyCam))
    , split(SplitScreenCanvas(left, right))
{
}

SplitStereoCanvas::~SplitStereoCanvas() {
    delete stereoCam;
    delete dummyCam;
}

void SplitStereoCanvas::Handle(Display::InitializeEventArg arg) {
    ((IListener<Display::InitializeEventArg>&)split).Handle(arg);
}

void SplitStereoCanvas::Handle(Display::ProcessEventArg arg) {
    stereoCam->SignalRendering(arg.approx);
    ((IListener<Display::ProcessEventArg>&)split).Handle(arg);
}

void SplitStereoCanvas::Handle(Display::ResizeEventArg arg) {
    ((IListener<Display::ResizeEventArg>&)split).Handle(arg);
}

void SplitStereoCanvas::Handle(Display::DeinitializeEventArg arg) {
    ((IListener<Display::DeinitializeEventArg>&)split)
        .Handle(Display::DeinitializeEventArg(arg));
}

unsigned int SplitStereoCanvas::GetWidth() const {
    return split.GetWidth();
}

unsigned int SplitStereoCanvas::GetHeight() const {
    return split.GetHeight();
}
    
void SplitStereoCanvas::SetWidth(const unsigned int width) {
    split.SetWidth(width);
}

void SplitStereoCanvas::SetHeight(const unsigned int height) {
    split.SetHeight(height);
}
    
ITexture2DPtr SplitStereoCanvas::GetTexture() {
    return split.GetTexture();
}

void SplitStereoCanvas::SetRenderer(IRenderer* renderer) {
    this->renderer = renderer;
    left.SetRenderer(renderer);
    right.SetRenderer(renderer);
}

void SplitStereoCanvas::SetViewingVolume(IViewingVolume* vv) {
    this->vv = vv;
    delete stereoCam;
    //stereoCam->SetViewingVolume(*vv);
    stereoCam = new StereoCamera(*vv);
    left.SetViewingVolume(stereoCam->GetLeft());
    right.SetViewingVolume(stereoCam->GetRight());
}

void SplitStereoCanvas::SetScene(ISceneNode* scene) {
    this->scene = scene;
    left.SetScene(scene);
    right.SetScene(scene);
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
