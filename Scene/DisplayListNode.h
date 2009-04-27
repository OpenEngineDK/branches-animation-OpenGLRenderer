// OpenGL display list node.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _OE_DISPLAY_LIST_NODE_H_
#define _OE_DISPLAY_LIST_NODE_H_

#include <Scene/ISceneNode.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
namespace Scene { 

/**
 * OpenGL display list node.
 *
 * @class DisplayListNode DisplayListNode.h Scene/DisplayListNode.h
 */
class DisplayListNode : public ISceneNode {
    OE_SCENE_NODE(DisplayListNode, ISceneNode)

public:
    DisplayListNode();
    DisplayListNode(GLuint id);
    ~DisplayListNode();
    GLuint GetID(); 

private:
    GLuint id;

};

} // NS Scene
} // NS OpenEngine

#endif // _OE_DISPLAY_LIST_NODE_H_
