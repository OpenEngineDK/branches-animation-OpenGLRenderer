// OpenGL color stereo canvas
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_COLOR_STEREO_CANVAS_H_
#define _OPENGL_COLOR_STEREO_CANVAS_H_

#include <Display/ICanvas.h>
#include <Display/RenderCanvas.h>

namespace OpenEngine {
    namespace Display {
        class StereoCamera;
        class IViewingVolume;
    }
namespace Display {

class ICanvasBackend;

    namespace OpenGL {


class ColorStereoCanvas : public IRenderCanvas {
private:
    IViewingVolume* dummyCam;
    StereoCamera* stereoCam;
    RenderCanvas *left, *right;
    bool init;
public:
    ColorStereoCanvas(ICanvasBackend* backend);
    virtual ~ColorStereoCanvas();

    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::ProcessEventArg arg);
    void Handle(Display::ResizeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    ITexture2DPtr GetTexture();

    void SetRenderer(IRenderer* renderer);
    void SetViewingVolume(IViewingVolume* vv);
    void SetScene(ISceneNode* scene);
};

} // NS OpenGL
} // NS Display
} // NS OpenEngine

#endif // #define _OPENGL_SPLIT_STEREO_CANVAS_H_
