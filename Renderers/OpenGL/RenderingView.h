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
#include <Scene/RenderStateNode.h>
#include <Scene/BlendingNode.h>
#include <list>

namespace OpenEngine {
    // Forward declarations.
    namespace Geometry {
        class GeometrySet;
        typedef boost::shared_ptr<GeometrySet> GeometrySetPtr;
        class Mesh;
        class Model;
    }
    namespace Resources {
        class IDataBlock;
        typedef boost::shared_ptr<IDataBlock> IDataBlockPtr;
        typedef std::list<IDataBlockPtr > IDataBlockList;
        class Indices;
        typedef boost::shared_ptr<Indices > IndicesPtr;
    }
namespace Renderers {
namespace OpenGL {

using namespace OpenEngine::Renderers;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Geometry;
using namespace std;

/**
 * Concrete RenderingView using OpenGL.
 */
class RenderingView : virtual public IRenderingView {    
    void RenderLine(Vector<3,float> vert,
                    Vector<3,float> norm,
                    Vector<3,float> color);

public:
    RenderingView(Viewport& viewport);
    virtual ~RenderingView();
    void VisitMeshNode(MeshNode* node);
    void VisitGeometryNode(GeometryNode* node);
    void VisitVertexArrayNode(VertexArrayNode* node);
    void VisitTransformationNode(TransformationNode* node);
    void VisitRenderStateNode(RenderStateNode* node);
    void VisitRenderNode(RenderNode* node);
    void VisitDisplayListNode(DisplayListNode* node);
    void VisitBlendingNode(BlendingNode* node);
    void Render(IRenderer* renderer, ISceneNode* root);
    void Handle(RenderingEventArg arg);
    IRenderer* GetRenderer();
    
protected:
    IRenderer* renderer;

    Matrix<4, 4, float> modelViewMatrix[16];
    unsigned int mvIndex;
    void ApplyViewingVolume(Display::IViewingVolume& volume);

    bool renderBinormal, renderTangent, renderSoftNormal, renderHardNormal;
    bool renderTexture, renderShader;
    unsigned int currentTexture;
    IShaderResourcePtr currentShader;
    IDataBlockPtr vertices;
    IDataBlockPtr normals;
    IDataBlockPtr colors;
    IDataBlockList texCoords;
    IndicesPtr indexBuffer;

    RenderStateNode* currentRenderState;

    void SwitchBlending(BlendingNode::BlendingFactor source, 
                        BlendingNode::BlendingFactor destination,
                        BlendingNode::BlendingEquation equation);
    inline void SwitchBlending(GLenum source, GLenum destination,
                               GLenum eqation);
    inline GLenum ConvertBlendingFactor(BlendingNode::BlendingFactor factor);
    inline GLenum ConvertBlendingEquation(BlendingNode::BlendingEquation equation);
    inline void RenderDebugGeometry(FacePtr face);
    inline void RenderBinormals(FacePtr face);
    inline void RenderTangents(FacePtr face);
    inline void RenderNormals(FacePtr face);
    inline void RenderHardNormal(FacePtr face);
    inline void ApplyMaterial(Geometry::MaterialPtr mat);
    void ApplyGeometrySet(GeometrySetPtr geom);
    inline void ApplyMesh(Mesh* prim);
    inline void ApplyModel(Model* model);
    inline void ApplyRenderState(RenderStateNode* node);
};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OPENGL_RENDERING_VIEW_H_
