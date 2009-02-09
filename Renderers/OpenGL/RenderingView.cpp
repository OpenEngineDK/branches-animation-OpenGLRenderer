// OpenGL rendering view.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers/OpenGL/RenderingView.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Geometry/FaceSet.h>
#include <Geometry/VertexArray.h>
#include <Scene/GeometryNode.h>
#include <Scene/VertexArrayNode.h>
#include <Scene/TransformationNode.h>
#include <Scene/DisplayListNode.h>
#include <Scene/RenderNode.h>
#include <Resources/IShaderResource.h>
#include <Display/Viewport.h>
#include <Display/IViewingVolume.h>

#include <Meta/OpenGL.h>
#include <Math/Math.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;
using OpenEngine::Geometry::FaceSet;
using OpenEngine::Geometry::VertexArray;
using OpenEngine::Resources::IShaderResource;
using OpenEngine::Display::Viewport;
using OpenEngine::Display::IViewingVolume;

/**
 * Rendering view constructor.
 *
 * @param viewport Viewport in which to render.
 */
RenderingView::RenderingView(Viewport& viewport)
    : IRenderingView(viewport),
      renderer(NULL) {
    renderBinormal=renderTangent=renderSoftNormal=renderHardNormal = false;
    renderTexture = renderShader = true;
    backgroundColor = Vector<4,float>(1.0);
}

/**
 * Rendering view destructor.
 */
RenderingView::~RenderingView() {}

/**
 * Get the renderer that the view is processing for.
 *
 * @return Current renderer, NULL if no renderer processing is active.
 */
IRenderer* RenderingView::GetRenderer() {
    return renderer;
}

void RenderingView::Handle(RenderingEventArg arg) {
    CHECK_FOR_GL_ERROR();

    // setup default render state
    RenderStateNode* renderStateNode = new RenderStateNode();
    renderStateNode->EnableOption(RenderStateNode::TEXTURE);
    renderStateNode->EnableOption(RenderStateNode::SHADER);
    renderStateNode->EnableOption(RenderStateNode::BACKFACE);
    renderStateNode->EnableOption(RenderStateNode::DEPTH_TEST);
    renderStateNode->DisableOption(RenderStateNode::LIGHTING); //@todo
    renderStateNode->DisableOption(RenderStateNode::WIREFRAME);
    ApplyRenderState(renderStateNode);
    delete renderStateNode;

    Vector<4,float> bgc = backgroundColor;
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    Render(&arg.renderer, arg.renderer.GetSceneRoot());
}

/**
 * Renderer the scene.
 *
 * @param renderer a Renderer
 * @param root The scene to be rendered
 */
void RenderingView::Render(IRenderer* renderer, ISceneNode* root) {
    this->renderer = renderer;
    root->Accept(*this);
    this->renderer = NULL;
}

/**
 * Process a rendering node.
 *
 * @param node Rendering node to apply.
 */
void RenderingView::VisitRenderNode(RenderNode* node) {
    node->Apply(this);
}

/**
 * Process a render state node.
 *
 * @param node Render state node to apply.
 */
void RenderingView::VisitRenderStateNode(RenderStateNode* node) {
    ApplyRenderState(node);
    node->VisitSubNodes(*this);
    RenderStateNode* inverse = node->GetInverse();
    ApplyRenderState(inverse);
    delete inverse;
}

void RenderingView::ApplyRenderState(RenderStateNode* node) {
    if (node->IsOptionEnabled(RenderStateNode::WIREFRAME)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::WIREFRAME)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        CHECK_FOR_GL_ERROR();
    }

    if (node->IsOptionEnabled(RenderStateNode::BACKFACE)) {
        glDisable(GL_CULL_FACE);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::BACKFACE)) {
        glEnable(GL_CULL_FACE);
        CHECK_FOR_GL_ERROR();
    }

    if (node->IsOptionEnabled(RenderStateNode::LIGHTING)) {
        glEnable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::LIGHTING)) {
        glDisable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
    }

    if (node->IsOptionEnabled(RenderStateNode::DEPTH_TEST)) {
        glEnable(GL_DEPTH_TEST);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::DEPTH_TEST)) {
        glDisable(GL_DEPTH_TEST);
        CHECK_FOR_GL_ERROR();
    }

    if (node->IsOptionEnabled(RenderStateNode::BINORMAL))
        renderBinormal = true;
    else if (node->IsOptionDisabled(RenderStateNode::BINORMAL))
        renderBinormal = false;

    if (node->IsOptionEnabled(RenderStateNode::TANGENT))
        renderTangent = true;
    else if (node->IsOptionDisabled(RenderStateNode::TANGENT))
        renderTangent = false;

    if (node->IsOptionEnabled(RenderStateNode::SOFT_NORMAL))
        renderSoftNormal = true;
    else if (node->IsOptionDisabled(RenderStateNode::SOFT_NORMAL))
        renderSoftNormal = false;

    if (node->IsOptionEnabled(RenderStateNode::HARD_NORMAL))
        renderHardNormal = true;
    else if (node->IsOptionDisabled(RenderStateNode::HARD_NORMAL))
        renderHardNormal = false;

    if (node->IsOptionEnabled(RenderStateNode::TEXTURE))
        renderTexture = true;
    else if (node->IsOptionDisabled(RenderStateNode::TEXTURE))
        renderTexture = false;

    if (node->IsOptionEnabled(RenderStateNode::SHADER))
        renderShader = true;
    else if (node->IsOptionDisabled(RenderStateNode::SHADER))
        renderShader = false;
}

/**
 * Process a transformation node.
 *
 * @param node Transformation node to apply.
 */
void RenderingView::VisitTransformationNode(TransformationNode* node) {
    // push transformation matrix
    Matrix<4,4,float> m = node->GetTransformationMatrix();
    float f[16];
    m.ToArray(f);
    glPushMatrix();
    CHECK_FOR_GL_ERROR();
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
    // traverse sub nodes
    node->VisitSubNodes(*this);
    // pop transformation matrix
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
}

void RenderingView::ApplyMaterial(MaterialPtr mat) {
    // check if shaders should be applied
    if (Renderer::IsGLSLSupported()) {

        // if the shader changes release the old shader
        if (currentShader != NULL && currentShader != mat->shad) {
            currentShader->ReleaseShader();
            currentShader.reset();
        }
        
        // check if a shader shall be applied
        if (renderShader &&
            mat->shad != NULL &&              // and the shader is not null
            currentShader != mat->shad) {     // and the shader is different from the current
            // get the bi-normal and tangent ids
            binormalid = mat->shad->GetAttributeID("binormal");
            tangentid = mat->shad->GetAttributeID("tangent");
            mat->shad->ApplyShader();
            // set the current shader
            currentShader = mat->shad;
        }
    }
    
    // if a shader is in use reset the current texture,
    // but dont disable in GL because the shader may use textures. 
    if (currentShader != NULL) currentTexture = 0;
    
    // if the face has no texture reset the current texture 
    else if (mat->texr == NULL) {
        glBindTexture(GL_TEXTURE_2D, 0); // @todo, remove this if not needed, release texture
        glDisable(GL_TEXTURE_2D);
        CHECK_FOR_GL_ERROR();
        currentTexture = 0;
    }
    
    // check if texture shall be applied
    else if (renderTexture &&
             currentTexture != mat->texr->GetID()) {  // and face texture is different then the current one
        currentTexture = mat->texr->GetID();
        glEnable(GL_TEXTURE_2D);
#ifdef DEBUG
        if (!glIsTexture(currentTexture)) //@todo: ifdef to debug
            throw Exception("texture not bound, id: " + currentTexture);
#endif
        glBindTexture(GL_TEXTURE_2D, currentTexture);
        CHECK_FOR_GL_ERROR();
    }
    
    // Apply materials
    // TODO: Decide whether we want both front and back
    //       materials (maybe a material property).
    float col[4];
    
    mat->diffuse.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
    CHECK_FOR_GL_ERROR();
    
    mat->ambient.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);
    CHECK_FOR_GL_ERROR();
    
    mat->specular.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
    CHECK_FOR_GL_ERROR();
    
    mat->emission.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, col);
    CHECK_FOR_GL_ERROR();
    
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
    CHECK_FOR_GL_ERROR();
}

/**
 * Process a geometry node.
 *
 * @param node Geometry node to render
 */
void RenderingView::VisitGeometryNode(GeometryNode* node) {
    // reset last state for matrial applying
    currentTexture = 0;
    currentShader.reset();
    binormalid = -1; 
    tangentid = -1;

    // Remember last bound texture and shader
    FaceList::iterator itr;
    FaceSet* faces = node->GetFaceSet();
    if (faces == NULL) return;

    // for each face ...
    for (itr = faces->begin(); itr != faces->end(); itr++) {
        FacePtr f = (*itr);

        ApplyMaterial(f->mat);

        glBegin(GL_TRIANGLES);
        // for each vertex ...
        for (int i=0; i<3; i++) {
            Vector<3,float> v = f->vert[i];
            Vector<2,float> t = f->texc[i];
            Vector<3,float> n = f->norm[i];
            Vector<4,float> c = f->colr[i];
            glTexCoord2f(t[0],t[1]);
            glColor4f (c[0],c[1],c[2],c[3]);
            glNormal3f(n[0],n[1],n[2]);
            // apply tangent and binormal per vertex for the shader to use
            if (currentShader != NULL) {
                if (binormalid != -1)
                    currentShader->VertexAttribute(binormalid, f->bino[i]);
                if (tangentid != -1)
                    currentShader->VertexAttribute(tangentid, f->tang[i]);
            }
			glVertex3f(v[0],v[1],v[2]);
        }
        glEnd();
        CHECK_FOR_GL_ERROR();

        RenderDebugGeometry(f);
    }

    // last we release the final shader
    if (currentShader != NULL)
        currentShader->ReleaseShader();

    // disable textures if it has been enabled
    glBindTexture(GL_TEXTURE_2D, 0); // @todo, remove this if not needed, release texture
    glDisable(GL_TEXTURE_2D);
    CHECK_FOR_GL_ERROR();
}

/**
 *   Process a Vertex Array Node which may contain a list of vertex arrays
 *   sorted by texture id.
 */
void RenderingView::VisitVertexArrayNode(VertexArrayNode* node){
    // reset last state for matrial applying
    currentTexture = 0;
    currentShader.reset();
    binormalid = -1; 
    tangentid = -1;

    CHECK_FOR_GL_ERROR();

    // Enable all client states
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    CHECK_FOR_GL_ERROR();

    // Get vertex array from the vertex array node
    list<VertexArray*> vaList = node->GetVertexArrays();
    for(list<VertexArray*>::iterator itr = vaList.begin(); itr!=vaList.end(); itr++) {
        VertexArray* va = (*itr);

        ApplyMaterial(va->mat);
        
        // Setup pointers to arrays
        glNormalPointer(GL_FLOAT, 0, va->GetNormals());
        glColorPointer(4, GL_FLOAT, 0, va->GetColors());
        glTexCoordPointer(2, GL_FLOAT, 0, va->GetTexCoords());
        glVertexPointer(3, GL_FLOAT, 0, va->GetVertices());
        glDrawArrays(GL_TRIANGLES, 0, va->GetNumFaces()*3);
    }
    CHECK_FOR_GL_ERROR();

    /* @todo: added debug rendering of normals and other things:
       RenderDebugGeometry(face); */

    // last we release the final shader
    if (currentShader != NULL)
        currentShader->ReleaseShader();

    // Disable all state changes
    glBindTexture(GL_TEXTURE_2D, 0); // @todo, remove this if not needed, release texture
    glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    CHECK_FOR_GL_ERROR();
}

void RenderingView::VisitDisplayListNode(DisplayListNode* node) {
    glCallList(node->GetID());
    CHECK_FOR_GL_ERROR();
}

void RenderingView::VisitBlendingNode(BlendingNode* node) {
    EnableBlending(node->GetSource(),
                   node->GetDestination(),
                   node->GetEquation());
    node->VisitSubNodes(*this);
    DisableBlending();
}

void RenderingView::EnableBlending(BlendingNode::BlendingFactor source, 
                                   BlendingNode::BlendingFactor destination,
                                   BlendingNode::BlendingEquation equation) {
    EnableBlending( ConvertBlendingFactor(source),
                    ConvertBlendingFactor(destination),
                    ConvertBlendingEquation(equation));
}

GLenum RenderingView::ConvertBlendingFactor(BlendingNode::BlendingFactor factor) {
    switch(factor) {
    case BlendingNode::ZERO: return GL_ZERO;
    case BlendingNode::ONE: return GL_ONE;
    case BlendingNode::BlendingNode::SRC_COLOR: return GL_SRC_COLOR;
    case BlendingNode::ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
    case BlendingNode::DST_COLOR: return GL_DST_COLOR;
    case BlendingNode::ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
    case BlendingNode::SRC_ALPHA: return GL_SRC_ALPHA;
    case BlendingNode::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
    case BlendingNode::DST_ALPHA: return GL_DST_ALPHA;
    case BlendingNode::ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
    default:
        throw Exception("unsupported blending factor");
    }
}

GLenum RenderingView::ConvertBlendingEquation(BlendingNode::BlendingEquation equation) {
    switch(equation) {
    case BlendingNode::ADD: return GL_FUNC_ADD;
    case BlendingNode::SUBTRACT: return GL_FUNC_SUBTRACT;
    case BlendingNode::REVERSE_SUBTRACT:
        return GL_FUNC_REVERSE_SUBTRACT_EXT; //@todo ?!?
    case BlendingNode::MIN: return GL_MIN;
    case BlendingNode::MAX: return GL_MAX;
    default:
        throw Exception("unsupported blending equation");
    }
}

void RenderingView::EnableBlending(GLenum source, GLenum destination,
                                     GLenum equation) {
    //@todo default values
    glEnable(GL_BLEND);
    glBlendFunc (source, destination);
    glBlendEquationEXT(equation);
    CHECK_FOR_GL_ERROR();
}

void RenderingView::DisableBlending() {
    glDisable(GL_BLEND);
    CHECK_FOR_GL_ERROR();
}

void RenderingView::SetBackgroundColor(Vector<4,float> color) {
    backgroundColor = color;
}

Vector<4,float> RenderingView::GetBackgroundColor() {
    return backgroundColor;
}

void RenderingView::RenderDebugGeometry(FacePtr f) {
        // Render normal if enabled
        GLboolean l = glIsEnabled(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
        glDisable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();

        if (renderBinormal)
            RenderBinormals(f);
        if (renderTangent)
            RenderTangents(f);
        if (renderSoftNormal)
            RenderNormals(f);
        if (renderHardNormal)
            RenderHardNormal(f);
        if (l) glEnable(GL_LIGHTING);
        CHECK_FOR_GL_ERROR();
}

void RenderingView::RenderNormals(FacePtr face) {
    for (int i=0; i<3; i++) {
        Vector<3,float> v = face->vert[i];
        Vector<3,float> n = face->norm[i];
		Vector<3,float> c (0,1,0);

        // if not unit length, make it red
        float length = n.GetLength();
        if (length > 1 + Math::EPS ||
            length < 1 - Math::EPS)
            c = Vector<3,float>(1,0,0);
        RenderLine(v,n,c);
    }
} 	

void RenderingView::RenderHardNormal(FacePtr face) {
    Vector<3,float> v = (face->vert[0]+face->vert[1]+face->vert[2])/3;
    Vector<3,float> n = face->hardNorm;
    Vector<3,float> c(1,0,1);
    RenderLine(v,n,c);
}

void RenderingView::RenderBinormals(FacePtr face) {
    for (int i=0; i<3; i++) {
        Vector<3,float> v = face->vert[i];
        Vector<3,float> n = face->bino[i];
		Vector<3,float> c(0,1,1);
        RenderLine(v,n,c);
    }
} 	

void RenderingView::RenderTangents(FacePtr face) {
    for (int i=0; i<3; i++) {
        Vector<3,float> v = face->vert[i];
		Vector<3,float> n = face->tang[i];
		Vector<3,float> c(1,0,0);
        RenderLine(v,n,c);
    }
}

void RenderingView::RenderLine(Vector<3,float> vert, Vector<3,float> norm, Vector<3,float> color) {
    renderer->DrawLine(Line(vert,vert+norm),color,1);
}

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
