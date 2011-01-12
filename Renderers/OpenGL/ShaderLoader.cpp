// Shader loader.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers/OpenGL/ShaderLoader.h>
#include <Scene/GeometryNode.h>
#include <Scene/MeshNode.h>
#include <Geometry/Mesh.h>
#include <Scene/VertexArrayNode.h>
#include <Geometry/FaceSet.h>
#include <Geometry/Face.h>
#include <Meta/OpenGL.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Resources/IShaderResource.h>
#include <list>

#include <Resources/PhongShader.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace Geometry;
using namespace Resources;
using namespace Scene;
using namespace std;

// using OpenEngine::Geometry::FaceSet;
// using OpenEngine::Geometry::FaceList;
// using OpenEngine::Resources::IShaderResourcePtr;
// using OpenEngine::Resources::ShaderTextureMap;
// using OpenEngine::Resources::TextureList;

    ShaderLoader::ShaderLoader(TextureLoader& textureLoader, Scene::ISceneNode& scene)
    : textureLoader(textureLoader), scene(scene) {}

ShaderLoader::~ShaderLoader() {}

void ShaderLoader::Handle(Core::InitializeEventArg event) {
    scene.Accept(*this);
}

/**
 * The Geometry nodes shaders are loaded on visit
 * 
 * @param node Geometry node 
 */
void ShaderLoader::VisitGeometryNode(GeometryNode* node) {
    FaceList::iterator face;
    FaceSet* faces = node->GetFaceSet();
    if (faces == NULL) return;
    IShaderResourcePtr currentShader;
    for (face = faces->begin(); face != faces->end(); face++) {
        IShaderResourcePtr shad = (*face)->mat->shad;
        if (currentShader == shad) continue;

        if (shad != NULL) {
            // load shader and its textures
            shad->Load();
            TextureList texs = shad->GetTextures();
            for (unsigned int i = 0; i < texs.size(); ++i)
                textureLoader.Load(texs[i]);
            currentShader = shad;
        }
    }
}

/**
 * The Vertex Array nodes shaders are loaded on visit
 * 
 * @param node VertexArray node 
 */
void ShaderLoader::VisitVertexArrayNode(VertexArrayNode* node) {
    std::list<Geometry::VertexArray*>::iterator array;
    std::list<Geometry::VertexArray*> arrays = node->GetVertexArrays();

    if (arrays.size() == 0) return;

    IShaderResourcePtr currentShader;
    for (array = arrays.begin(); array != arrays.end(); array++) {
        IShaderResourcePtr shad = (*array)->mat->shad;
        if (currentShader == shad) continue;

        if (shad != NULL) {
            // load shader and its textures
            shad->Load();
            TextureList texs = shad->GetTextures();
            for (unsigned int i = 0; i < texs.size(); ++i)
                textureLoader.Load(texs[i]);
        }
    }
}

void ShaderLoader::VisitMeshNode(MeshNode* node) { 
    //@todo optimize by reusing the shader.
    MaterialPtr m = node->GetMesh()->GetMaterial();
    if (m->shading == Material::PHONG || m->shading == Material::BLINN) {
        logger.info << "loading phong shader" << logger.end;
        m->shad = IShaderResourcePtr(new PhongShader(m));
        m->shad->Load();
        TextureList texs = m->shad->GetTextures();
        for (unsigned int i = 0; i < texs.size(); ++i)
            textureLoader.Load(texs[i]);
    }
}


} // NS OpenGL
} // NS Renderers
} // NS OpenEngine
