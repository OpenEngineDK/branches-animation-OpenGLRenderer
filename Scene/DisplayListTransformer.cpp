// Display list transformer.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//---------------------------------------------------------------------

#include <Scene/DisplayListTransformer.h>

#include <Renderers/OpenGL/Renderer.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Scene {

    using namespace OpenEngine::Resources;
    using namespace OpenEngine::Renderers::OpenGL;

    /**
     * Construct a GL transformor, that transforms geometry nodes to
     * GL nodes.
     */
    DisplayListTransformer::DisplayListTransformer(IRenderingView* r): r(r) {
        
    }

    /**
     * Destructor.
     */
    DisplayListTransformer::~DisplayListTransformer(){

    }
    
    /**
     * Transforms the geometry nodes of a tree into GL nodes.
     * Note: OpenGL must be initialized before calling this method.
     * 
     * @param node Root node of a scene to build from.
     */
    void DisplayListTransformer::Transform(ISceneNode& node){
        node.Accept(*this);
    }

    /**
     * Transform the encountered geometry node into a GL node.
     *
     * @param node Geometry node.
     */
    void DisplayListTransformer::VisitGeometryNode(GeometryNode *node){
        GLuint id = glGenLists(1);
        if (id == 0) {
            logger.info << "DisplayListTransformer: Error creating display list element" << logger.end;
            return;
        } 
        glNewList(id, GL_COMPILE);        
        r->VisitGeometryNode(node);
        glEndList();

        DisplayListNode* glnode = new DisplayListNode(id);
        node->GetParent()->ReplaceNode(node, glnode);
    }

    void DisplayListTransformer::VisitVertexArrayNode(VertexArrayNode *node){
        GLuint id = glGenLists(1);
        if (id == 0) {
            logger.info << "DisplayListTransformer: Error creating display list element" << logger.end;
            return;
        } 
        glNewList(id, GL_COMPILE);        
        r->VisitVertexArrayNode(node);
        glEndList();

        DisplayListNode* glnode = new DisplayListNode(id);
        node->GetParent()->ReplaceNode(node, glnode);
    }

    void DisplayListTransformer::Handle(RenderingEventArg arg) {
        Transform(*arg.renderer.GetSceneRoot());
    }

} // NS Scene
} // NS OpenEngine
