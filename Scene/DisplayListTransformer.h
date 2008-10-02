// Quad transformer.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _DISPLAY_LIST_TRANSFORMER_H_
#define _DISPLAY_LIST_TRANSFORMER_H_

#include <Scene/DisplayListNode.h>
#include <Scene/GeometryNode.h>
#include <Scene/VertexArrayNode.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Renderers/IRenderingView.h>
#include <Core/IListener.h>
#include <Renderers/IRenderer.h>

namespace OpenEngine {
    namespace Scene {
        
using namespace OpenEngine::Geometry;
using namespace OpenEngine::Renderers;

class DisplayListTransformer : public ISceneNodeVisitor, public IListener<RenderingEventArg> {
 private:
    IRenderingView* r;
    IRenderer* renderer;
    
 public:
    DisplayListTransformer(IRenderingView* r);
    ~DisplayListTransformer();
    
    void Transform(ISceneNode& node);
    void Handle(RenderingEventArg arg);
    void VisitGeometryNode(GeometryNode* node);
    void VisitVertexArrayNode(VertexArrayNode* node);
};

} // NS Scene
} // NS OpenEngine

#endif // _DISPLAY_LIST_TRANSFORMER_H_
