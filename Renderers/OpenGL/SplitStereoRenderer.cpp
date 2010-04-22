// OpenGL split screen stero rendering link
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Renderers/OpenGL/SplitStereoRenderer.h>

#include <Display/StereoCamera.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace Display;

SplitStereoRenderer::SplitStereoRenderer() {
    
}

SplitStereoRenderer::~SplitStereoRenderer() {
    
}

void SplitStereoRenderer::Handle(InitializeEventArg arg) {
    stereo = new StereoCamera(*arg.canvas.GetViewingVolume());
    initEvent.Notify(arg);
}

void SplitStereoRenderer::Handle(RedrawEventArg arg) {
    this->width = arg.canvas.GetWidth();
    height = arg.canvas.GetHeight();
    scene = arg.canvas.GetScene();
    depth = arg.canvas.GetDepth();

    stereo->SignalRendering(arg.approx);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int halfw = arg.canvas.GetWidth() * 0.5;
    unsigned int height = arg.canvas.GetHeight();
    vv = stereo->GetLeft();

    Vector<4,int> d(0, 0, halfw, height);
    glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
    CHECK_FOR_GL_ERROR();
    redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));

    vv = stereo->GetRight();
    d = Vector<4,int>(halfw, 0, halfw, height);
    glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
    redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));
}

void SplitStereoRenderer::Handle(DeinitializeEventArg arg) {
    deinitEvent.Notify(arg);
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
