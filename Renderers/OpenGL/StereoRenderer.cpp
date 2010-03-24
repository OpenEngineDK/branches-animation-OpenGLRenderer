// OpenGL Stereoscopic Renderer Implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Renderers/OpenGL/StereoRenderer.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace Display;

StereoRenderer::StereoRenderer(Viewport* viewport)
    : Renderer(viewport)
{
    
}

StereoRenderer::~StereoRenderer() {
    
}

void StereoRenderer::SetStereoCamera(StereoCamera* sc) {
    stereoCam = sc;
}

void StereoRenderer::Handle(InitializeEventArg arg) {
    Renderer::Handle(arg);
}

void StereoRenderer::Handle(ProcessEventArg arg) {
    const bool rbstereo = true;
    stereoCam->SignalRendering(arg.approx);
    
    GetViewport().SetViewingVolume(stereoCam->GetLeft());

    glDrawBuffer(GL_BACK_LEFT);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rbstereo)
        glColorMask (GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);

    Renderer::Handle(arg);

    GetViewport().SetViewingVolume(stereoCam->GetRight());

    glDrawBuffer(GL_BACK_RIGHT);
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rbstereo)
        glColorMask (GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);

    Renderer::Handle(arg);
}

void StereoRenderer::Handle(DeinitializeEventArg arg) {
    Renderer::Handle(arg);

}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
