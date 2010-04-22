// OpenGL color stero rendering link
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Renderers/OpenGL/ColorStereoRenderer.h>

#include <Display/StereoCamera.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace Display;

ColorStereoRenderer::ColorStereoRenderer() {
    
}

ColorStereoRenderer::~ColorStereoRenderer() {
    
}

void ColorStereoRenderer::Handle(InitializeEventArg arg) {
    stereo = new StereoCamera(*arg.canvas.GetViewingVolume());
    initEvent.Notify(arg);
}

void ColorStereoRenderer::Handle(RedrawEventArg arg) {
    this->width = arg.canvas.GetWidth();
    height = arg.canvas.GetHeight();
    scene = arg.canvas.GetScene();
    depth = arg.canvas.GetDepth();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vv = stereo->GetLeft();
    glColorMask (GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);

    redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));

    vv = stereo->GetRight();
    glColorMask (GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
    redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));

    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void ColorStereoRenderer::Handle(DeinitializeEventArg arg) {
    deinitEvent.Notify(arg);
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
