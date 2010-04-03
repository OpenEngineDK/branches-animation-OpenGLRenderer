// OpenGL buffered renderer implementation.
// renderers a scene to a 2d texture using a Frame Buffer Object (FBO)
//
// initial code based on:
// http://www.gamedev.net/reference/articles/article2331.asp
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OE_FBO_BUFFERED_RENDERER_H_
#define _OE_FBO_BUFFERED_RENDERER_H_

#include <Meta/OpenGL.h>
#include <Renderers/IBufferedRenderer.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Resources/Texture2D.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

/**
 * FBOBufferedRenderer using OpenGL
 *
 * @class FBOBufferedRenderer FBOBufferedRenderer.h Renderers/OpenGL/Renderer.h
 */
class FBOBufferedRenderer : public IBufferedRenderer {
public:
    FBOBufferedRenderer(Viewport* viewport);
    virtual ~FBOBufferedRenderer();

    virtual void Handle(InitializeEventArg arg);
    virtual void Handle(ProcessEventArg arg);
    virtual void Handle(DeinitializeEventArg arg);

    virtual ITexture2DPtr GetColorBuffer() const;

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
    void LoadTexture(Resources::ITexture2DPtr t)   { peer.LoadTexture(t.get()); }
    void LoadTexture(Resources::ITexture2D* t)     { peer.LoadTexture(t); }
    void LoadTexture(Resources::ITexture3DPtr t)   { peer.LoadTexture(t.get()); }
    void LoadTexture(Resources::ITexture3D* t)     { peer.LoadTexture(t); }
    void RebindTexture(Resources::ITexture2DPtr t, unsigned int x, unsigned int y, unsigned int w, unsigned int h) { peer.RebindTexture(t.get(), x, y, w, h); }
    void RebindTexture(Resources::ITexture2D* t, unsigned int x, unsigned int y, unsigned int w, unsigned int h)   { peer.RebindTexture(t, x, y, w, h); }
    void RebindTexture(Resources::ITexture3DPtr t, unsigned int x, unsigned int y, unsigned int z, unsigned int w, unsigned int h, unsigned int d) { peer.RebindTexture(t.get(), x, y, z, w, h, d); }
    void RebindTexture(Resources::ITexture3D* t, unsigned int x, unsigned int y, unsigned int z, unsigned int w, unsigned int h, unsigned int d)   { peer.RebindTexture(t, x, y, z, w, h, d); }
    void BindFrameBuffer(FrameBuffer* fb)         { peer.BindFrameBuffer(fb); }
    void BindDataBlock(Resources::IDataBlock* bo)     { peer.BindDataBlock(bo); }
    void DrawFace(FacePtr f)                                      { peer.DrawFace(f); }
    void DrawFace(FacePtr f, Vector<3,float> c, float w)          { peer.DrawFace(f, c, w); }
    void DrawLine(Line l, Vector<3,float> c, float w)             { peer.DrawLine(l, c, w); }
    void DrawPoint(Vector<3,float> p, Vector<3,float> c, float w) { peer.DrawPoint(p, c, w); }
    void DrawSphere(Vector<3,float> c, float r, Vector<3,float> col) { peer.DrawSphere(c, r, col); }
    void SetBackgroundColor(Vector<4,float> color) { peer.SetBackgroundColor(color); }
    Vector<4,float> GetBackgroundColor() { return peer.GetBackgroundColor(); }
    bool BufferSupport() { return peer.BufferSupport(); }

private:
 
    Renderer peer;
    GLuint fbo, depthbuffer, img;
    unsigned int width, height;
    std::string EnumToString(GLenum status);
    void RenderTextureInOrtho();
    // color buffer wrapper
    ITexture2DPtr colorbuf;
    class ColorBuffer : public Resources::Texture2D<unsigned char> {
    public:
        ColorBuffer(FBOBufferedRenderer& r) 
            : Resources::Texture2D<unsigned char>(), r(r) {
            Load();
        }
        void Load() {
            this->id = 0;
            this->width = r.width;
            this->height = r.height;
            this->channels = 4;
            this->format = Resources::RGBA;
            this->mipmapping = false;
        }
        void Unload() {}
    private:
        FBOBufferedRenderer& r;
    };
    
};

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers

#endif // _FBO_BUFFERED_RENDERER_H_
