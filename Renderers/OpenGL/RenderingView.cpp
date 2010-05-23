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
#include <Scene/PostProcessNode.h>
#include <Resources/IShaderResource.h>
#include <Resources/ITexture2D.h>
#include <Display/Viewport.h>
#include <Display/IViewingVolume.h>
#include <Geometry/GeometrySet.h>
#include <Geometry/Mesh.h>
#include <Scene/MeshNode.h>
#include <Resources/DataBlock.h>

#include <Meta/OpenGL.h>
#include <Math/Math.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;
using OpenEngine::Geometry::FaceSet;
using OpenEngine::Geometry::GeometrySet;
using OpenEngine::Geometry::Mesh;
using OpenEngine::Geometry::VertexArray;
using OpenEngine::Resources::IShaderResource;
using OpenEngine::Resources::ITexture2D;
using OpenEngine::Display::Viewport;
using OpenEngine::Display::IViewingVolume;
using OpenEngine::Scene::RenderStateNode;

/**
 * Rendering view constructor.
 *
 * @param viewport Viewport in which to render.
 */
RenderingView::RenderingView() {
    renderBinormal=renderTangent=renderSoftNormal=renderHardNormal = false;
    renderTexture = renderShader = true;
    currentRenderState = new RenderStateNode();
    currentRenderState->EnableOption(RenderStateNode::TEXTURE);
    currentRenderState->EnableOption(RenderStateNode::SHADER);
    currentRenderState->EnableOption(RenderStateNode::BACKFACE);
    currentRenderState->EnableOption(RenderStateNode::DEPTH_TEST);
    currentRenderState->DisableOption(RenderStateNode::LIGHTING); //@todo
    currentRenderState->DisableOption(RenderStateNode::WIREFRAME);
    
    currentGeom = GeometrySetPtr(new GeometrySet());
    indexBuffer = IndicesPtr();
}

/**
 * Rendering view destructor.
 */
RenderingView::~RenderingView() {}

void RenderingView::Handle(RenderingEventArg arg) {
    if (arg.renderer.GetCurrentStage() == IRenderer::RENDERER_PROCESS){
#ifdef OE_SAFE
        if (arg.canvas.GetScene() == NULL) 
            throw Exception("Scene was NULL while rendering.");
#endif
        
        this->arg = &arg;
        currentModelViewMatrix = arg.canvas.GetViewingVolume()->GetViewMatrix();
        
        // setup default render state
        // RenderStateNode* renderStateNode = new RenderStateNode();
        ApplyRenderState(currentRenderState);
        arg.canvas.GetScene()->Accept(*this);
        this->arg = NULL;
        
        // cleanup
        if (currentShader != NULL) {
            currentShader->ReleaseShader();
            currentShader.reset();
        }
        if (currentTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            CHECK_FOR_GL_ERROR();
            currentTexture = 0;
        }
    }else if (arg.renderer.GetCurrentStage() == IRenderer::RENDERER_INITIALIZE){
        copyShader = glCreateProgram();
        
        // Create and compile the vertex program
        GLuint vertexID = glCreateShader(GL_VERTEX_SHADER);
        const GLchar** vertexSource = new const GLchar*[1];
        vertexSource[0] = "varying vec2 texCoord;void main(void){texCoord = (gl_Vertex.xy + 1.0) * 0.5;gl_Position = gl_Vertex;}";
        glShaderSource(vertexID, 1, vertexSource, NULL);
        glCompileShader(vertexID);
        glAttachShader(copyShader, vertexID);
        
        // Create and compile the copy fragment program
        GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar** fragSource = new const GLchar*[1];
        fragSource[0] = "uniform sampler2D image0;uniform sampler2DShadow depth;varying vec2 texCoord;void main(void){gl_FragColor = texture2D(image0, texCoord); gl_FragDepth = shadow2D(depth, vec3(texCoord, 0.0)).x;}";
        glShaderSource(fragID, 1, fragSource, NULL);
        glCompileShader(fragID);
        glAttachShader(copyShader, fragID);
        CHECK_FOR_GL_ERROR();
        
        glLinkProgram(copyShader);
        CHECK_FOR_GL_ERROR();

        glUseProgram(copyShader);
        GLuint loc = glGetUniformLocation(copyShader, "image0");
        glUniform1i(loc, 0);
        loc = glGetUniformLocation(copyShader, "depth");
        glUniform1i(loc, 1);
        glUseProgram(0);
        CHECK_FOR_GL_ERROR();
    }
}
    
/**
 * Process a rendering node.
 *
 * @param node Rendering node to apply.
 */
void RenderingView::VisitRenderNode(RenderNode* node) {
    node->Apply(*arg, *this);
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
    
    if (node->IsOptionEnabled(RenderStateNode::COLOR_MATERIAL)) {
        glEnable(GL_COLOR_MATERIAL);
        CHECK_FOR_GL_ERROR();
    }
    else if (node->IsOptionDisabled(RenderStateNode::COLOR_MATERIAL)) {
        glDisable(GL_COLOR_MATERIAL);
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
 * Process a render state node.
 *
 * @param node Render state node to apply.
 */
    void RenderingView::VisitRenderStateNode(Scene::RenderStateNode* node) {
    // apply differences between current state and node
    RenderStateNode* changes = node->GetDifference(*currentRenderState);
    ApplyRenderState(changes);
    // replace current state
    RenderStateNode* prevCurrent = currentRenderState;
    currentRenderState = node;
    node->VisitSubNodes(*this);
    // undo differences
    changes->Invert();
    ApplyRenderState(changes);
    // restore previous state
    currentRenderState = prevCurrent;
    delete changes;
    CHECK_FOR_GL_ERROR();
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
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
    Matrix<4, 4, float> oldModelView = currentModelViewMatrix;
    currentModelViewMatrix = m * currentModelViewMatrix;
    // traverse sub nodes
    node->VisitSubNodes(*this);
    CHECK_FOR_GL_ERROR();
    // pop transformation matrix
    glPopMatrix();
    currentModelViewMatrix = oldModelView;
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

            mat->shad->ApplyShader();
            // Bind the material textures that are useful to the
            // shader.
            

            // set the current shader
            currentShader = mat->shad;
        }
    }
    
    // if a shader is in use reset the current texture,
    // but dont disable in GL because the shader may use textures. 
    if (currentShader != NULL) currentTexture = 0;
    
    // if the face has no texture reset the current texture 
    else if (mat->Get2DTextures().size() == 0) {
        glBindTexture(GL_TEXTURE_2D, 0); // @todo, remove this if not needed, release texture
        glDisable(GL_TEXTURE_2D);
        CHECK_FOR_GL_ERROR();
        currentTexture = 0;
    }
    
    // check if texture shall be applied
    else if (renderTexture &&
             currentTexture != mat->Get2DTextures().front().second->GetID()) {  // and face texture is different then the current one
        currentTexture = mat->Get2DTextures().front().second->GetID();
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
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->ambient.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular.ToArray());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat->emission.ToArray());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
    CHECK_FOR_GL_ERROR();
}

/**
 * Applies the given mesh. Applying the empty or NULL mesh will
 * disable enabled client states.
 */
void RenderingView::ApplyGeometrySet(GeometrySetPtr geom){
    if (geom == NULL){
        // Disable client states enabled by previous geom.
        if (currentGeom->GetVertices() != NULL) {
            glDisableClientState(GL_VERTEX_ARRAY);
        }
        if (currentGeom->GetNormals() != NULL){ 
            glDisableClientState(GL_NORMAL_ARRAY);
        }
        if (currentGeom->GetColors() != NULL){ 
            glDisableClientState(GL_COLOR_ARRAY);
        }
        for (int count = currentGeom->GetTexCoords().size()-1; count >= 0 ; --count){
            glClientActiveTexture(GL_TEXTURE0 + count);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        currentGeom = GeometrySetPtr(new GeometrySet());

    }else if (currentGeom != geom){

        bool bufferSupport = arg->renderer.BufferSupport();
        
        IDataBlockPtr v = geom->GetVertices();
        if (v == NULL){
            // No vertices, disable them.
            glDisableClientState(GL_VERTEX_ARRAY);
        }else if (v != currentGeom->GetVertices()){
            // new vertices, bind them
            glEnableClientState(GL_VERTEX_ARRAY);
            // Only bind the buffer if it is supported
            if (bufferSupport) glBindBuffer(GL_ARRAY_BUFFER, v->GetID());
            if (v->GetID() != 0)
                glVertexPointer(v->GetDimension(), GL_FLOAT, 0, 0);
            else
                glVertexPointer(v->GetDimension(), GL_FLOAT, 0, v->GetVoidDataPtr());
        }else{
            glEnableClientState(GL_VERTEX_ARRAY);
        }
        CHECK_FOR_GL_ERROR();

        IDataBlockPtr n = geom->GetNormals();
        if (n == NULL){
            glDisableClientState(GL_NORMAL_ARRAY);
        }else if (n != currentGeom->GetNormals()){
            glEnableClientState(GL_NORMAL_ARRAY);
            if (bufferSupport) glBindBuffer(GL_ARRAY_BUFFER, n->GetID());
            if (n->GetID() != 0)
                glNormalPointer(GL_FLOAT, 0, 0);
            else
                glNormalPointer(GL_FLOAT, 0, n->GetVoidDataPtr());
        }
        CHECK_FOR_GL_ERROR();

        IDataBlockPtr c = geom->GetColors();
        if (c == NULL){
            glDisableClientState(GL_COLOR_ARRAY);
        }else if (c != currentGeom->GetColors()){
            glEnableClientState(GL_COLOR_ARRAY);
            if (bufferSupport) glBindBuffer(GL_ARRAY_BUFFER, c->GetID());
            if (c->GetID() != 0)
                glColorPointer(c->GetDimension(), GL_FLOAT, 0, 0);
            else
                glColorPointer(c->GetDimension(), GL_FLOAT, 0, c->GetVoidDataPtr());
        }
        CHECK_FOR_GL_ERROR();

        IDataBlockList tcs = geom->GetTexCoords();
        IDataBlockList texCoords = currentGeom->GetTexCoords();
        IDataBlockList::iterator newItr = tcs.begin();
        IDataBlockList::iterator oldItr = texCoords.begin();
        unsigned int maxCount = max(texCoords.size(), tcs.size());
        for (unsigned int count = 0; count < maxCount; ++count){
            glClientActiveTexture(GL_TEXTURE0 + count);
            IDataBlockPtr newTc = (*newItr);
            IDataBlockPtr oldTc = (*oldItr);

            if (tcs.size() <= count){
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }else if (newTc != oldTc || texCoords.size() <= count+1){
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                if (bufferSupport) glBindBuffer(GL_ARRAY_BUFFER, newTc->GetID());
                if (newTc->GetID() != 0){
                    glTexCoordPointer(newTc->GetDimension(), GL_FLOAT, 0, 0);
                }else
                    glTexCoordPointer(newTc->GetDimension(), GL_FLOAT, 0, newTc->GetVoidDataPtr());
            }

            if (newItr != tcs.end()) ++newItr;
            if (oldItr != texCoords.end()) ++oldItr;
        }
        CHECK_FOR_GL_ERROR();

        if (bufferSupport) glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_FOR_GL_ERROR();

        currentGeom = geom;
    }
}

void RenderingView::ApplyMesh(Mesh* prim){
    if (prim == NULL){
        ApplyGeometrySet(GeometrySetPtr());

    } else {
        // Apply the geometry set.
        ApplyGeometrySet(prim->GetGeometrySet());
        
        // Apply the material.
        ApplyMaterial(prim->GetMaterial());

        bool bufferSupport = arg->renderer.BufferSupport();
        
        // Apply the index buffer and draw
        indexBuffer = prim->GetIndices();
        GLsizei count = prim->GetDrawingRange();
        unsigned int offset = prim->GetIndexOffset();
        Geometry::Type type = prim->GetType();
        if (bufferSupport) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->GetID());
        if (indexBuffer->GetID() != 0){
            glDrawElements(type, count, GL_UNSIGNED_INT, (GLvoid*)(offset * sizeof(GLuint)));
        }else{
            glDrawElements(type, count, GL_UNSIGNED_INT, indexBuffer->GetData() + offset);
        }

        if (bufferSupport) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    CHECK_FOR_GL_ERROR();
}

/**
 * Process a mesh node.
 *
 * @param node Mesh node to render
 */
void RenderingView::VisitMeshNode(MeshNode* node) {
    ApplyMesh(node->GetMesh().get());
    node->VisitSubNodes(*this);
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

    // Reset geometry state
    ApplyGeometrySet(GeometrySetPtr());

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

    // Reset geometry state
    ApplyGeometrySet(GeometrySetPtr());

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

void RenderingView::VisitPostProcessNode(PostProcessNode* node) {
    // if the node isn't enabled or there is no fbo
    // support then just proceed as usual.
    if (arg->renderer.FrameBufferSupport() == false ||
        node->enabled == false) {
        node->VisitSubNodes(*this);
        return;
    }
    
    // Save the previous state
    GLint prevFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &prevFbo);
    Vector<4, GLint> prevDims;
    glGetIntegerv(GL_VIEWPORT, prevDims.ToArray());
    CHECK_FOR_GL_ERROR();
    
    // Setup the new framebuffer
    Vector<2, int> dims = node->GetDimension();
    glViewport(0, 0, dims[0], dims[1]);
    CHECK_FOR_GL_ERROR();

    // Use the new framebuffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, node->GetSceneFrameBuffer()->GetID());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_FOR_GL_ERROR();
    
    node->VisitSubNodes(*this);
    
    node->PreEffect(arg->renderer, currentModelViewMatrix);
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, node->GetEffectFrameBuffer()->GetID());
    glDepthFunc(GL_ALWAYS);
    CHECK_FOR_GL_ERROR();
    
    node->GetEffect()->ApplyShader();
    glRecti(-1,-1,1,1);
    node->GetEffect()->ReleaseShader();
    
    // Copy the final image to the final textures
    if (node->GetFinalTexs().size() != 0){
        if (node->GetFinalTexs()[0]->GetID() == 0){
            // Initialize the final texs and setup the
            // effect shader to use them.
            for (unsigned int i = 0; i < node->GetFinalTexs().size(); ++i){
                ITexture2DPtr tex = node->GetFinalTexs()[i];
                arg->renderer.LoadTexture(tex);
                string colorid = "finalColor" + Utils::Convert::ToString<unsigned int>(i);
                if (node->GetEffect()->GetUniformID(colorid) >= 0)
                    node->GetEffect()->SetTexture(colorid, tex);
                CHECK_FOR_GL_ERROR();
            }
        }
        for (unsigned int i = 0; i < node->GetFinalTexs().size(); ++i){
            glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
            glBindTexture(GL_TEXTURE_2D, node->GetFinalTexs()[i]->GetID());
            GLsizei width = node->GetFinalTexs()[i]->GetWidth();
            GLsizei height = node->GetFinalTexs()[i]->GetHeight();
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
        }
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        CHECK_FOR_GL_ERROR();
    }
    
    // copy the picture onto the original framebuffer.
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prevFbo);
    glViewport(prevDims[0], prevDims[1], prevDims[2], prevDims[3]);
    CHECK_FOR_GL_ERROR();
    
    if (!(node->offscreenRendering)){
        glUseProgram(copyShader);
        glBindTexture(GL_TEXTURE_2D, node->GetEffectFrameBuffer()->GetTexAttachment(0)->GetID());
        glActiveTexture(GL_TEXTURE1);        
        glBindTexture(GL_TEXTURE_2D, node->GetEffectFrameBuffer()->GetDepthTexture()->GetID());
        glRecti(-1,-1,1,1);
        CHECK_FOR_GL_ERROR();
    }
    // Clean up
    // @TODO reset to previous depth func, not just less
    glDepthFunc(GL_LESS);
    if (currentShader)
        currentShader->ApplyShader();
    else
        glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
}
    
void RenderingView::VisitBlendingNode(BlendingNode* node) {
    // save original blend state
    GLboolean blending = glIsEnabled(GL_BLEND);
    GLenum source, destination, equation;
    glGetIntegerv(GL_BLEND_SRC, (GLint*) &source);
    glGetIntegerv(GL_BLEND_DST, (GLint*) &destination);
    glGetIntegerv(GL_BLEND_EQUATION, (GLint*) &equation);

    glEnable(GL_BLEND);
    SwitchBlending(node->GetSource(),
                   node->GetDestination(),
                   node->GetEquation());
    node->VisitSubNodes(*this);

    // apply original blend state
    SwitchBlending(source, destination, equation);
    if (!blending) glDisable(GL_BLEND);
    CHECK_FOR_GL_ERROR();
}

void RenderingView::SwitchBlending(BlendingNode::BlendingFactor source, 
                                   BlendingNode::BlendingFactor destination,
                                   BlendingNode::BlendingEquation equation) {
    SwitchBlending( ConvertBlendingFactor(source),
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

void RenderingView::SwitchBlending(GLenum source, GLenum destination,
                                     GLenum equation) {
    glBlendFunc(source, destination);
    glBlendEquationEXT(equation);
    CHECK_FOR_GL_ERROR();
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
    arg->renderer.DrawLine(Line(vert,vert+norm),color,1);
}

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
