// OpenGL color stero rendering link
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_COLOR_STEREO_RENDERER_H_
#define _OPENGL_COLOR_STEREO_RENDERER_H_

#include <Display/ICanvas.h>
#include <Meta/OpenGL.h>

namespace OpenEngine {
    namespace Display {
        class StereoCamera;
    }
namespace Renderers {
    namespace OpenGL {

using Display::RedrawEventArg;
using Display::ResizeEventArg;
using Display::StereoCamera;
using Display::ICanvas;
using Display::ICanvasListener;

class ColorStereoRenderer : public ICanvas, public ICanvasListener {
private:
    StereoCamera* stereo;
    unsigned int width, height, depth;
    GLuint leftID, rightID;
public:
    ColorStereoRenderer();
    virtual ~ColorStereoRenderer();

    void Handle(Display::InitializeEventArg arg);
    void Handle(RedrawEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);
    void Handle(ResizeEventArg arg) { resizeEvent.Notify(arg); }

    unsigned int GetWidth() const { return width; }
    unsigned int GetHeight() const { return height; } 
    unsigned int GetDepth()  const { return depth; }

    void SetWidth(const unsigned int width) { this->width = width; }
    void SetHeight(const unsigned int height) { this->height = height; } 
    void SetDepth(const unsigned int depth)  { this->depth = depth; }

};

} // NS OpenGL
} // NS Display
} // NS OpenEngine

#endif // _OPENGL_COLOR_STEREO_RENDERER_H_
