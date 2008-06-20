// GL tree node.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _DISPLAY_LIST_NODE_H_
#define _DISPLAY_LIST_NODE_H_

#include <Scene/SceneNode.h>
#include <Meta/OpenGL.h>


namespace OpenEngine {
    namespace Scene { 

// forward declarations   
class ISceneNodeVisitor; 


using namespace OpenEngine::Scene;

/**
 * GL display list tree node.
 * To build a tree please refer to -.
 *
 * @see GLTreeBuilder
 *
 * @class DisplayListNode DisplayListNode.h Scene/DisplayListNode.h
 */
class DisplayListNode : public SceneNode {
private:
    GLuint id;
public:
    DisplayListNode(GLuint id);
    ~DisplayListNode();

    void Accept(ISceneNodeVisitor& visitor);
    GLuint GetID(); 
};

}
}
#endif // _DISPLAY_LIST_NODE_H_
