// OpenGL light preprocessor implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _LIGHT_RENDERER_H_
#define _LIGHT_RENDERER_H_

#include <Renderers/IRenderer.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Core/IListener.h>

#include <Meta/OpenGL.h>

namespace OpenEngine {

    //forward declarations
    namespace Scene {
        class TransformationNode;
        class PointLightNode;
        class DirectionalLightNode;
        class SpotLightNode;
    }


namespace Renderers {
namespace OpenGL {

using OpenEngine::Scene::TransformationNode;
using OpenEngine::Scene::PointLightNode;
using OpenEngine::Scene::DirectionalLightNode;
using OpenEngine::Scene::SpotLightNode;
using OpenEngine::Scene::ISceneNodeVisitor;

using OpenEngine::Core::IListener;
using OpenEngine::Renderers::IRenderer;
using OpenEngine::Renderers::RenderingEventArg;


/**
 * Setup OpenGL lighting
 *
 * @class LightRenderer LightRenderer.h Renderers/OpenGL/LightRenderer.h
 */
class LightRenderer: public ISceneNodeVisitor, public IListener<RenderingEventArg> {
private:
    float pos[4], dir[4];
    GLint count;
public:
    GLint lightCount;

 public:

    LightRenderer(); 
    ~LightRenderer();
        
    void Handle(RenderingEventArg arg);
    void VisitTransformationNode(TransformationNode* node);
    void VisitDirectionalLightNode(DirectionalLightNode* node);
    void VisitPointLightNode(PointLightNode* node);
    void VisitSpotLightNode(SpotLightNode* node);
};

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers

#endif
