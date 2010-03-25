// OpenGL renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <Renderers/IRenderer.h>
#include <Renderers/IRenderingView.h>
#include <Core/Event.h>
#include <Scene/ISceneNode.h>
#include <Math/Matrix.h>
#include <Geometry/Face.h>
#include <vector>
#include <Resources/ITexture.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {

    //forward declarations
    namespace Scene {
        class TransformationNode;
        class PointLightNode;
        class DirectionalLightNode;
        class SpotLightNode;
    }

namespace Renderers {

/**
 * OpenGL Rendering implementation.
 *
 * @namespace OpenEngine::Renderers::OpenGL
 */
namespace OpenGL {


using OpenEngine::Math::Matrix;
using OpenEngine::Geometry::FacePtr;
using OpenEngine::Core::InitializeEventArg;
using OpenEngine::Core::ProcessEventArg;
using OpenEngine::Core::DeinitializeEventArg;
using OpenEngine::Core::Event;
using OpenEngine::Resources::Types::Type;
using OpenEngine::Resources::ColorFormat;
using OpenEngine::Resources::ITexture;


/**
 * OpenGL Shader Language versions
 */
enum GLSLVersion { GLSL_UNKNOWN, GLSL_NONE, GLSL_14, GLSL_20 };

/**
 * Renderer using OpenGL
 *
 * @class Renderer Renderer.h Renderers/OpenGL/IRenderer.h
 */
class Renderer : public IRenderer {
private:
    static GLSLVersion glslversion;
    bool compressionSupport;
    Scene::ISceneNode* root;
    Viewport* viewport;
    Vector<4,float> backgroundColor;

    // Event lists for the rendering phases.
    Event<RenderingEventArg> initialize;
    Event<RenderingEventArg> preProcess;
    Event<RenderingEventArg> process;
    Event<RenderingEventArg> postProcess;
    Event<RenderingEventArg> deinitialize;

    void InitializeGLSLVersion();
    inline void SetupTexParameters(ITexture2D* tex);
    inline void SetupTexParameters(ITexture3D* tex);
    inline void SetTextureCompression(ITexture* tex);
    inline GLint GLInternalColorFormat(ColorFormat f);
    inline GLenum GLColorFormat(ColorFormat f);

public:
    Renderer(Viewport* viewport);
    virtual ~Renderer();

    void Handle(InitializeEventArg arg);
    void Handle(ProcessEventArg arg);
    void Handle(DeinitializeEventArg arg);

    virtual Viewport& GetViewport() const;
    virtual void SetSceneRoot(Scene::ISceneNode* root);
    virtual Scene::ISceneNode* GetSceneRoot() const;

    /**
     * Event lists for the rendering phases.
     */
    virtual IEvent<RenderingEventArg>& InitializeEvent();
    virtual IEvent<RenderingEventArg>& PreProcessEvent();
    virtual IEvent<RenderingEventArg>& ProcessEvent();
    virtual IEvent<RenderingEventArg>& PostProcessEvent();
    virtual IEvent<RenderingEventArg>& DeinitializeEvent();

    /**
     * Test if OpenGL Shader Language is supported.
     *
     * @return True if support is found.
     */
    static bool IsGLSLSupported();

    /**
     * Get the supported version of OpenGL Shader Language.
     *
     * @return Version
     */
    static GLSLVersion GetGLSLVersion();

    virtual void SetBackgroundColor(Vector<4,float> color);
    virtual Vector<4,float> GetBackgroundColor();

    virtual void ApplyViewingVolume(Display::IViewingVolume& volume);
    virtual void LoadTexture(ITexture2DPtr texr);
    virtual void LoadTexture(ITexture2D* texr);
    virtual void LoadTexture(ITexture3DPtr texr);
    virtual void LoadTexture(ITexture3D* texr);
    virtual void RebindTexture(ITexture2DPtr texr, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
    virtual void RebindTexture(ITexture2D* texr, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
    virtual void RebindTexture(ITexture3DPtr texr, unsigned int x, unsigned int y, unsigned int z, unsigned int w, unsigned int h, unsigned int d);
    virtual void RebindTexture(ITexture3D* texr, unsigned int x, unsigned int y, unsigned int z, unsigned int w, unsigned int h, unsigned int d);
    virtual void DrawFace(FacePtr face);
    virtual void DrawFace(FacePtr face, Vector<3,float> color, float width = 1);
    virtual void DrawLine(Line line, Vector<3,float> color, float width = 1);
    virtual void DrawPoint(Vector<3,float> point, Vector<3,float> color , float size);
    virtual void DrawSphere(Vector<3,float> center, float radius, Vector<3,float> color);
};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _RENDERER_H_
