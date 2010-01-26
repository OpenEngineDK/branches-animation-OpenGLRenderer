// OpenGL texture loader.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers/OpenGL/TextureLoader.h>
#include <Scene/GeometryNode.h>
#include <Scene/VertexArrayNode.h>
#include <Geometry/FaceSet.h>
#include <Geometry/Face.h>
#include <Geometry/VertexArray.h>
#include <Meta/OpenGL.h>
#include <Resources/ITextureResource.h>
#include <Logging/Logger.h>
#include <list>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using std::list;
using OpenEngine::Geometry::FaceSet;
using OpenEngine::Geometry::FaceList;
using OpenEngine::Geometry::VertexArray;
using namespace OpenEngine::Resources;

TextureLoader::TextureLoader() {}

TextureLoader::~TextureLoader() {}

void TextureLoader::Handle(RenderingEventArg arg) {
    arg.renderer.GetSceneRoot()->Accept(*this);
}

/**
 * The Geometry nodes textures are loaded on visit
 * 
 * @param node Geometry node 
 */
void TextureLoader::VisitGeometryNode(GeometryNode* node) {
    FaceSet* faces = node->GetFaceSet();
    if (faces == NULL) return;
    for (FaceList::iterator face = faces->begin(); face != faces->end(); face++) {
        // load face textures
        LoadTextureResource((*face)->mat->texr);
    }
}

/**
 * The textures for each vertex array in the nodes list are loaded on visit.
 * 
 * @param node Vertex Array Node
 */
void TextureLoader::VisitVertexArrayNode(VertexArrayNode* node) {
    list<VertexArray*> vaList = node->GetVertexArrays();
    // Iterate through list of Vertex Arrays
    for (list<VertexArray*>::iterator itr = vaList.begin(); itr!=vaList.end(); itr++) {
        // Load vertex array texture
        LoadTextureResource((*itr)->mat->texr);
    }
}

/**
 * Load a texture resource.
 *
 * @param tex Texture resource pointer.
 */
    void TextureLoader::LoadTextureResource(ITextureResourcePtr& tex, bool linearInterpolation, bool mipmapping) {
    if (tex == NULL) return;
    if(tex->GetID() == 0) {
        tex->Load();

        GLuint texid;
        glGenTextures(1, &texid);
        tex->SetID(texid);
            
        glBindTexture(GL_TEXTURE_2D, texid);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
        if (mipmapping){
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            if (linearInterpolation){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }else{
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
        }else{
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
            if (linearInterpolation){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }else{
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
        }
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        GLuint depth = 0;
        switch (tex->GetColorFormat()) {
        case LUMINANCE:  depth = GL_LUMINANCE; break;
        case RGB: depth = GL_RGB;   break;
        case BGR: depth = GL_BGR;   break;
        case RGBA: depth = GL_RGBA;  break;
        case BGRA: depth = GL_BGRA;  break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, depth, tex->GetWidth(), tex->GetHeight(), 0,
                     depth, GL_UNSIGNED_BYTE, tex->GetData());
        tex->Unload();
    }
}


} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
