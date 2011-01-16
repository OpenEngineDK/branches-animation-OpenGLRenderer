// OpenGL based blending canvas implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_BLEND_CANVAS_H_
#define _OPENGL_BLEND_CANVAS_H_

#include <Display/ICanvas.h>
#include <Math/Vector.h>
#include <list>

namespace OpenEngine {
namespace Display {
    class ICanvasBackend;
namespace OpenGL {

using Math::Vector;
using std::list;

class BlendCanvas: public ICanvas {
private:
    struct Element {
        int x, y;
        ITexture2DPtr tex;
        Vector<4,float> color;
    };
    bool init;
    list<Element> elements;
    Vector<4,float> bg;
    list<ICanvas*> inits;
public:
    BlendCanvas(ICanvasBackend* backend);
    virtual ~BlendCanvas();

    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);
    void Handle(Display::ProcessEventArg arg);
    void Handle(Display::ResizeEventArg arg);

    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    ITexture2DPtr GetTexture();

    void AddTexture(ITexture2DPtr tex, int x, int y, Vector<4,float> color = (Vector<4,float>(1.0f)));
    void SetBackground(Vector<4,float> bg); 
    void Clear();

    void InitCanvas(ICanvas* canvas);

};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _OPENGL_HUD_CANVAS_H_
