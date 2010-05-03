// OpenGL split screen canvas implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_SPLIT_SCREEN_CANVAS_H_
#define _OPENGL_SPLIT_SCREEN_CANVAS_H_

#include <Display/ICanvas.h>
#include <Display/OpenGL/TextureCanvasBase.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

class SplitScreenCanvas: public ICanvas, public TextureCanvasBase {
private:
    ICanvas &first, &second;
    bool init;
public:
    SplitScreenCanvas(ICanvas& first, ICanvas& second);
    virtual ~SplitScreenCanvas();

    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);
    void Handle(Display::ProcessEventArg arg);
    void Handle(Display::ResizeEventArg arg);

    void SetFirst(ICanvas& canvas);
    void SetSecond(ICanvas& canvas);
    ICanvas& GetFirst();
    ICanvas& GetSecond();

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    ITexture2DPtr GetTexture();

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OPENGL_SPLIT_SCREEN_CANVAS_H_
