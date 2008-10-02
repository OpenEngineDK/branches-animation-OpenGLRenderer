// OpenGL rendering view.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_RENDERING_VIEW_H_
#define _OPENGL_RENDERING_VIEW_H_

#include <Meta/OpenGL.h>
#include <Renderers/IRenderingView.h>
#include <Renderers/RenderStateNode.h>
#include <Scene/BlendingNode.h>
#include <vector>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace OpenEngine::Renderers;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Geometry;
using namespace std;

/**
 * Concrete RenderingView using OpenGL.
 */
class RenderingView : virtual public IRenderingView {
    IRenderer* renderer;
    vector<RenderStateNode*> stateStack;

    void RenderBinormals(FacePtr face);
    void RenderTangents(FacePtr face);
    void RenderNormals(FacePtr face);
    void RenderHardNormal(FacePtr face);
    void RenderLine(Vector<3,float> vert, Vector<3,float> norm, Vector<3,float> color);
    bool IsOptionSet(RenderStateNode::RenderStateOption o);
    void ApplyMaterial(MaterialPtr mat);
    void ApplyRenderState();
public:
    RenderingView(Viewport& viewport);
    virtual ~RenderingView();
    void VisitGeometryNode(GeometryNode* node);
    void VisitVertexArrayNode(VertexArrayNode* node);
    void VisitTransformationNode(TransformationNode* node);
    void VisitRenderStateNode(RenderStateNode* node);
    void VisitRenderNode(IRenderNode* node);
    void VisitDisplayListNode(DisplayListNode* node);
    void VisitBlendingNode(BlendingNode* node);
    void Render(IRenderer* renderer, ISceneNode* root);
    void Handle(RenderingEventArg arg);
    IRenderer* GetRenderer();
    virtual void SetBackgroundColor(Vector<4,float> color);
    virtual Vector<4,float> GetBackgroundColor();

private:
    int currentTexture;
    IShaderResourcePtr currentShader;
    int binormalid;
    int tangentid;
    Vector<4,float> backgroundColor;

    void EnableBlending(BlendingNode::BlendingFactor source, 
                        BlendingNode::BlendingFactor destination,
                        BlendingNode::BlendingEquation equation);
    void DisableBlending();

    inline void EnableBlending(GLenum source, GLenum destination,
                               GLenum eqation);
    inline GLenum ConvertBlendingFactor(BlendingNode::BlendingFactor factor);
    inline GLenum ConvertBlendingEquation(BlendingNode::BlendingEquation equation);

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OPENGL_RENDERING_VIEW_H_
