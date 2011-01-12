// Shader loader.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _SHADER_LOADER_H_
#define _SHADER_LOADER_H_

#include <Core/EngineEvents.h>
#include <Core/IListener.h>
#include <Renderers/TextureLoader.h>
#include <Scene/ISceneNode.h>
#include <Scene/ISceneNodeVisitor.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using OpenEngine::Scene::GeometryNode;
using OpenEngine::Scene::MeshNode;
using OpenEngine::Scene::VertexArrayNode;
using OpenEngine::Scene::ISceneNodeVisitor;

/**
 * OpenGL specific shader loader.
 *
 * @class ShaderLoader ShaderLoader.h Renderers/OpenGL/ShaderLoader.h
 */
 class ShaderLoader : public ISceneNodeVisitor, public Core::IListener<Core::InitializeEventArg> {
private:
    TextureLoader& textureLoader;
    Scene::ISceneNode& scene;
public:
    ShaderLoader(TextureLoader& textureLoader, Scene::ISceneNode& scene);
    ~ShaderLoader();

    void Handle(Core::InitializeEventArg event);
    void VisitGeometryNode(GeometryNode* node);
    void VisitVertexArrayNode(VertexArrayNode* node);
    void VisitMeshNode(MeshNode* node);
};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif
