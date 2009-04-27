// GL tree node.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include <Scene/DisplayListNode.h>
#include <Core/Exceptions.h>

namespace OpenEngine {
    namespace Scene {

/**
 * Default constructor to satisfy scene node requirements.
 * Calling this constructor will throw an exception.
 * @throws Core::Exception
 */
DisplayListNode::DisplayListNode() {
    throw Core::Exception("Unable to create a display list node with no display list id.");
}

/** 
 * Create a display list tree node.
 * @param id An index pointing to a GL display list.
 */
DisplayListNode::DisplayListNode(GLuint id) {
    this->id = id;
}

/**
 * GL node destructor.
 */
DisplayListNode::~DisplayListNode() {
    if (glIsList(id))
        glDeleteLists(id,1);
}

/**
 * Get ID value
 * Use the ID value in a call to glCallList()
 * @return The ID value.
 */
GLuint DisplayListNode::GetID() {
    return id;
}

}
}
