// OpenGL Stereoscopic Renderer Implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _STEREO_RENDERER_H_
#define _STEREO_RENDERER_H_

#include <Renderers/OpenGL/Renderer.h>
#include <Core/Event.h>
#include <Scene/ISceneNode.h>
#include <Math/Matrix.h>
#include <vector>
#include <Display/StereoCamera.h>

namespace OpenEngine {


namespace Renderers {

/**
 * OpenGL Rendering implementation.
 *
 * @namespace OpenEngine::Renderers::OpenGL
 */
namespace OpenGL {


using OpenEngine::Math::Matrix;
using OpenEngine::Geometry::FacePtr;
using OpenEngine::Core::InitializeEventArg;
using OpenEngine::Core::ProcessEventArg;
using OpenEngine::Core::DeinitializeEventArg;
using OpenEngine::Core::Event;


/**
 * StereoScopic Renderer using OpenGL
 *
 * @class StereoRenderer Renderer.h Renderers/OpenGL/StereoRenderer.h
 */
class StereoRenderer : public Renderer {
private:
    Display::StereoCamera* stereoCam;
public:
    StereoRenderer(Viewport* viewport);
    virtual ~StereoRenderer();

    void SetStereoCamera(Display::StereoCamera* sc);

    void Handle(InitializeEventArg arg);
    void Handle(ProcessEventArg arg);
    void Handle(DeinitializeEventArg arg);
};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _RENDERER_H_
