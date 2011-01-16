// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_SHADOW_LIGHT_POST_PROCESS_NODE_H_
#define _OE_SHADOW_LIGHT_POST_PROCESS_NODE_H_

#include <Scene/PostProcessNode.h>
#include <Display/IViewingVolume.h>
#include <Resources/FrameBuffer.h>


namespace OpenEngine {
namespace Scene {

/**
 * Short description.
 *
 * @class ShadowLightPostProcessNode ShadowLightPostProcessNode.h ons/OpenGLRenderer/Scene/ShadowLightPostProcessNode.h
 */
class ShadowLightPostProcessNode : public PostProcessNode {
private:
    class DepthRenderer : public ISceneNodeVisitor {
        ShadowLightPostProcessNode* shadowNode;
    public:
        DepthRenderer(ShadowLightPostProcessNode* n);
        void Render(Renderers::RenderingEventArg arg);

        void VisitTransformationNode(TransformationNode* node);
        void VisitMeshNode(MeshNode* node);
        void ApplyViewingVolume(Display::IViewingVolume& volume);
    };

    DepthRenderer* depthRenderer;
    Display::IViewingVolume* viewingVolume;
    Resources::FrameBuffer* depthFB;
    Vector<2, int> shadowDims;
public:
    ShadowLightPostProcessNode(Resources::IShaderResourcePtr shader,
                               Math::Vector<2, int> dims,
                               Math::Vector<2, int> shadowDims);
    void Handle(Renderers::RenderingEventArg arg);
    void Initialize(Renderers::RenderingEventArg arg);

    void SetViewingVolume(Display::IViewingVolume* v);
};
} // NS Scene
} // NS OpenEngine

#endif // _OE_SHADOW_LIGHT_POST_PROCESS_NODE_H_
