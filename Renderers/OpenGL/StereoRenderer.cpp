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

StereoRenderer::StereoRenderer(Viewport* viewport)
    : root(NULL) 
    , viewport(viewport)
{
    backgroundColor = Vector<4,float>(1.0);
}

StereoRenderer::~StereoRenderer() {
    delete viewport;
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void StereoRenderer::Handle(ProcessEventArg arg) {
    // @todo: assert we are in preprocess stage

    Vector<4,float> bgc = backgroundColor;
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // run the processing phases
    RenderingEventArg rarg(*this, arg.start, arg.approx);
    this->preProcess.Notify(rarg);
    this->stage = RENDERER_PROCESS;
    this->process.Notify(rarg);
    this->stage = RENDERER_POSTPROCESS;
    this->postProcess.Notify(rarg);
    this->stage = RENDERER_PREPROCESS;
}

void StereoRenderer::Handle(DeinitializeEventArg arg) {
    this->stage = RENDERER_DEINITIALIZE;
    this->deinitialize.Notify(RenderingEventArg(*this));
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
