//
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include <Scene/ShadowLightPostProcessNode.h>
#include <Scene/TransformationNode.h>
#include <Scene/MeshNode.h>
#include <Logging/Logger.h>
#include <Meta/OpenGL.h>
#include <Geometry/Mesh.h>
#include <Geometry/GeometrySet.h>
#include <Resources/IShaderResource.h>

namespace OpenEngine {
namespace Scene {

using namespace Resources;
using namespace Math;
using namespace Display;
using namespace Geometry;

ShadowLightPostProcessNode::DepthRenderer::DepthRenderer(ShadowLightPostProcessNode* n)
    : shadowNode(n) {

}

void ShadowLightPostProcessNode::DepthRenderer::ApplyViewingVolume(IViewingVolume& volume) {

    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    CHECK_FOR_GL_ERROR();

    // Reset The Projection Matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Setup OpenGL with the volumes projection matrix
    Matrix<4,4,float> projMatrix = volume.GetProjectionMatrix();
    float arr[16] = {0};
    projMatrix.ToArray(arr);
    glMultMatrixf(arr);
    CHECK_FOR_GL_ERROR();

    // Select the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    CHECK_FOR_GL_ERROR();

    // Reset the modelview matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Get the view matrix and apply it
    Matrix<4,4,float> matrix = volume.GetViewMatrix();
    float f[16] = {0};
    matrix.ToArray(f);
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
}


void ShadowLightPostProcessNode::DepthRenderer::Render(Renderers::RenderingEventArg arg) {
    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &prevFbo);
    Vector<4, GLint> prevDims;
    glGetIntegerv(GL_VIEWPORT, prevDims.ToArray());
    CHECK_FOR_GL_ERROR();

    // Setup the new frame buffer
    Vector<2, int> dims = shadowNode->GetDimension();
    glViewport(0, 0, dims[0], dims[1]);
    CHECK_FOR_GL_ERROR();
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, shadowNode->depthFB->GetID());
    // Blit the previous framebuffer depth for merging instead of sorting.
    glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_FOR_GL_ERROR();

    // We need to setup the texture for depth compare
    // glBindTexture(GL_TEXTURE_2D,shadowNode->depthFB->GetDepthTexture()->GetID());
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);

    //ApplyViewingVolume(*(arg.canvas.GetViewingVolume()));

    // Apply VV
    ApplyViewingVolume(*(shadowNode->viewingVolume));

    // Turn of unneeded stuff!
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.1, 4.0);


    shadowNode->Accept(*this);

    // glBindTexture(GL_TEXTURE_2D,shadowNode->depthFB->GetDepthTexture()->GetID());
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prevFbo);
    glViewport(prevDims[0], prevDims[1], prevDims[2], prevDims[3]);
    CHECK_FOR_GL_ERROR();

    // Reset viewing volume
    //ApplyViewingVolume(*(arg.canvas.GetViewingVolume()));

}

void ShadowLightPostProcessNode::DepthRenderer::VisitTransformationNode(TransformationNode* node) {
    // push transformation matrix
    Matrix<4,4,float> m = node->GetTransformationMatrix();
    float f[16];
    m.ToArray(f);
    glPushMatrix();
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
    //Matrix<4, 4, float> oldModelView = currentModelViewMatrix;
    //currentModelViewMatrix = m * currentModelViewMatrix;
    // traverse sub nodes
    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();
    // pop transformation matrix
    glPopMatrix();
    //currentModelViewMatrix = oldModelView;
    CHECK_FOR_GL_ERROR();
}

void ShadowLightPostProcessNode::DepthRenderer::VisitMeshNode(MeshNode* node) {
    MeshPtr mesh = node->GetMesh();
    GeometrySetPtr geom = mesh->GetGeometrySet();

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    IDataBlockPtr v = geom->GetVertices();

    glBindBuffer(GL_ARRAY_BUFFER, v->GetID());
    if (v->GetID() != 0)
        glVertexPointer(v->GetDimension(), GL_FLOAT, 0, 0);
    else
        glVertexPointer(v->GetDimension(), GL_FLOAT, 0, v->GetVoidDataPtr());


    CHECK_FOR_GL_ERROR();

    IndicesPtr indexBuffer = mesh->GetIndices();
    GLsizei count = mesh->GetDrawingRange();
    unsigned int offset = mesh->GetIndexOffset();
    Geometry::Type type = mesh->GetType();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->GetID());
    if (indexBuffer->GetID() != 0){
        glDrawElements(type, count, GL_UNSIGNED_INT, (GLvoid*)(offset * sizeof(GLuint)));
    }else{
        glDrawElements(type, count, GL_UNSIGNED_INT, indexBuffer->GetData() + offset);
    }



    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();
}



ShadowLightPostProcessNode::ShadowLightPostProcessNode(IShaderResourcePtr s,
                                                       Vector<2,int> dims,
                                                       Vector<2,int> shadowDims)
: PostProcessNode(s, dims, 1, true),viewingVolume(NULL) {
    depthFB = new FrameBuffer(shadowDims,0,true);
    depthRenderer = new DepthRenderer(this);

    GetEffect()->SetTexture("shadow", depthFB->GetDepthTexture());
}

void ShadowLightPostProcessNode::SetViewingVolume(IViewingVolume* v) {
    viewingVolume = v;
}

void ShadowLightPostProcessNode::Initialize(Renderers::RenderingEventArg arg) {
    arg.renderer.BindFrameBuffer(depthFB);

    glBindTexture(GL_TEXTURE_2D,depthFB->GetDepthTexture()->GetID());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    // GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Remove artifact on the edges of the shadowmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

}

void ShadowLightPostProcessNode::Handle(Renderers::RenderingEventArg arg) {
    if (arg.renderer.GetCurrentStage() == Renderers::IRenderer::RENDERER_PREPROCESS) {
        depthRenderer->Render(arg);

        Matrix<4,4,float> bias(.5, .0, .0,  .0,
                               .0, .5, .0,  .0,
                               .0, .0, .5,  .0,
                               .5, .5, .5, 1.0);


        GetEffect()->SetUniform("lightMat",
                                viewingVolume->GetViewMatrix() *
                                viewingVolume->GetProjectionMatrix() *
                                bias);
    }
    PostProcessNode::Handle(arg);
}



} // NS Scene
} // NS OpenEngine

