// OpenGL texture base class for canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_TEXTURE_CANVAS_BASE_H_
#define _OPENGL_TEXTURE_CANVAS_BASE_H_

#include <Display/ICanvas.h>
#include <Resources/ITexture2D.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

class TextureCanvasBase {
private:
    class CustomTexture : public ITexture2D {
        friend class TextureCanvasBase;
    private:
    public:
        unsigned int GetChannelSize() { return 8; };
        void Load() {}
        void Unload() {}
        void Reverse() {}
        void ReverseVertecally() {}
        void ReverseHorizontally() {}
    private:
    };
    CustomTexture* ctex;
protected:
    ITexture2DPtr tex;
    void CreateTexture();
    void SetupTexture();
    void CopyToTexture();
    void SetTextureWidth(const unsigned int width);
    void SetTextureHeight(const unsigned int height);
    unsigned int GetTextureWidth() const;
    unsigned int GetTextureHeight() const;
public:
    TextureCanvasBase();
    virtual ~TextureCanvasBase();
};

} // NS OpenGL
} // NS Display
} // NS OpenEngine

#endif // _OPENGL_TEXTURE_CANVAS_BASE_H_
