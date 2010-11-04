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

namespace OpenEngine {
namespace Display {
    class ICanvasBackend;
namespace OpenGL {

class SplitScreenCanvas: public ICanvas {
public:
    enum Split {
        VERTICAL, HORIZONTAL
    };
private:
    ICanvas &first, &second;
    bool init;
    Split split;
    float firstPercentage;
    void UpdateChildCanvases();
public:
    SplitScreenCanvas(ICanvasBackend* backend, 
                      ICanvas& first, 
                      ICanvas& second, 
                      Split split = VERTICAL, 
                      float firstPercentage = 0.5);
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
