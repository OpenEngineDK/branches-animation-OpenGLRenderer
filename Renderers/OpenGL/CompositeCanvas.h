// OpenGL split screen renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_TEXTURE_RENDERER_H_
#define _OPENGL_TEXTURE_RENDERER_H_

#include <Display/ICanvas.h>
#include <Display/OpenGL/TextureCanvas.h>
#include <vector>
#include <set>

namespace OpenEngine {
namespace Renderers {

/**
 * OpenGL Rendering implementation.
 *
 * @namespace OpenEngine::Renderers::OpenGL
 */
namespace OpenGL {
    using Display::OpenGL::TextureCanvas;
    using Display::RedrawEventArg;
    using Display::ResizeEventArg;
    using Core::IListener;
    using Display::ICanvasListener;
    using Display::ICanvas;

    using std::vector;
    using std::set;

/**
 * Draw multiple 2D canvases onto a single target canvas.
 *
 * @class CompositeCanvas CompositeCanvas.h Renderers/OpenGL/CompositeCanvas.h
 */
class CompositeCanvas 
    : public ICanvasListener
{
private:
    class Layout {
    public:
        TextureCanvas* canvas;
        unsigned int x, y;
        Layout(TextureCanvas* canvas, unsigned int x, unsigned int y) 
            : canvas(canvas), x(x), y(y) {}
        Layout(const Layout& l): canvas(l.canvas), x(l.x), y(l.y) {
            
        }
        virtual ~Layout() {}
    };

    vector<Layout*> layout;   // ordered by back to front
    vector<Layout*> frames;   // ordered by canvas number
    set<ICanvas*> processSet; // process each canvas once per frame only
    bool init;
    Layout *maxframe;
    unsigned int maxid;
    unsigned int origX, origY, origWidth, origHeight;
    bool animate;
    bool updateFrames;
    unsigned int cols;
    float elapsedTime, maxTime;
    
    inline void UpdateFrames(ICanvas& canvas);
    inline void ToggleFrame(unsigned int frame);
    void MoveToFront(Layout* l);
    void Animate();

public:
    CompositeCanvas();
    virtual ~CompositeCanvas();

    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);
    void Handle(RedrawEventArg arg);
    void Handle(ResizeEventArg arg);

    void AddCanvas(TextureCanvas* canvas);
    void ToggleMaximize(unsigned int canvas);

    void SetColumns(unsigned int cols);
    unsigned int GetColumns();
};

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

#endif // _RENDERER_H_
