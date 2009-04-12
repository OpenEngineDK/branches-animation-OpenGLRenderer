// OpenGL buffered renderer implementation.
// renderers a scene to a 2d texture using a GLCopy after renderering
//
// initial code based on the FBOBufferedRenderer and:
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=36
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_GLCOPY_BUFFERED_RENDERER_H_
#define _OE_GLCOPY_BUFFERED_RENDERER_H_

#include <Meta/OpenGL.h>
#include <Renderers/IBufferedRenderer.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Resources/ITextureResource.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

/**
 * GLCopyBufferedRenderer using OpenGL
 *
 * @class GLCopyBufferedRenderer GLCopyBufferedRenderer.h Renderers/OpenGL/Renderer.h
 */
class GLCopyBufferedRenderer : public IBufferedRenderer {
public:
    GLCopyBufferedRenderer(Viewport* viewport);
    virtual ~GLCopyBufferedRenderer();

    virtual void Handle(InitializeEventArg arg);
    virtual void Handle(ProcessEventArg arg);
    virtual void Handle(DeinitializeEventArg arg);

    virtual ITextureResourcePtr GetColorBuffer() const;

    // forward calls to the un-buffered renderer
    Core::IEvent<RenderingEventArg>& InitializeEvent()   { return peer.InitializeEvent(); }
    Core::IEvent<RenderingEventArg>& PreProcessEvent()   { return peer.PreProcessEvent(); }
    Core::IEvent<RenderingEventArg>& ProcessEvent()      { return peer.ProcessEvent(); }
    Core::IEvent<RenderingEventArg>& PostProcessEvent()  { return peer.PostProcessEvent(); }
    Core::IEvent<RenderingEventArg>& DeinitializeEvent() { return peer.DeinitializeEvent(); }
    Display::Viewport& GetViewport() const               { return peer.GetViewport(); }
    Scene::ISceneNode* GetSceneRoot() const              { return peer.GetSceneRoot(); }
    void SetSceneRoot(Scene::ISceneNode* scene)          { peer.SetSceneRoot(scene); }
    void ApplyViewingVolume(Display::IViewingVolume& v)  { peer.ApplyViewingVolume(v); }
    void LoadTexture(Resources::ITextureResourcePtr t)   { peer.LoadTexture(t); }
    void RebindTexture(Resources::ITextureResourcePtr t) { peer.RebindTexture(t); }
    void DrawFace(FacePtr f)                                      { peer.DrawFace(f); }
    void DrawFace(FacePtr f, Vector<3,float> c, float w)          { peer.DrawFace(f, c, w); }
    void DrawLine(Line l, Vector<3,float> c, float w)             { peer.DrawLine(l, c, w); }
    void DrawPoint(Vector<3,float> p, Vector<3,float> c, float w) { peer.DrawPoint(p, c, w); }
    void DrawSphere(Vector<3,float> c, float r, Vector<3,float> col) { peer.DrawSphere(c, r, col); }
    void SetBackgroundColor(Vector<4,float> color) { peer.SetBackgroundColor(color); }
    Vector<4,float> GetBackgroundColor() { return peer.GetBackgroundColor(); }

private:
 
    Renderer peer;
    GLuint fbo, depthbuffer, img;
    unsigned int width, height;
    // color buffer wrapper
    ITextureResourcePtr colorbuf;
    class ColorBuffer : public Resources::ITextureResource {
    public:
        ColorBuffer(GLCopyBufferedRenderer& r) : r(r) {}
        void Load() {}
        void Unload() {}
        int GetID() { return r.img; }
        void SetID(int id) { throw Exception("Buffered textures can not change identifiers."); }
        unsigned int GetWidth() { return r.width; }
        unsigned int GetHeight() { return r.height; }
        unsigned int GetDepth() { return 32; }
        unsigned char* GetData() { throw Exception("Buffered textures can not supply data information."); }
        Resources::ColorFormat GetColorFormat() { return Resources::RGBA; }
    private:
        GLCopyBufferedRenderer& r;
    };
    
};

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers

#endif // _GLCOPY_BUFFERED_RENDERER_H_
