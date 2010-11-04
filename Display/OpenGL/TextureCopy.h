// OpenGL texture copy backend for canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_TEXTURE_COPY_BACKEND_H_
#define _OPENGL_TEXTURE_COPY_BACKEND_H_

#include <Display/ICanvasBackend.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

class TextureCopy: public ICanvasBackend {
private:
    class CustomTexture : public Resources::ITexture2D {
        friend class TextureCopy;
    private:
    public:
        unsigned int GetChannelSize() { return 8; };
        ITexture2D* Clone() { return NULL; }
        void Load() {}
        void Unload() {}
        void Reverse() {}
        void ReverseVertecally() {}
        void ReverseHorizontally() {}
    private:
    };
    CustomTexture* ctex;
    Resources::ITexture2DPtr tex;
public:
    TextureCopy();
    virtual ~TextureCopy();
    void Init(unsigned int width, unsigned int height);
    void Deinit();
    void Pre();
    void Post();
    void SetDimensions(unsigned int width, unsigned int height);
    ICanvasBackend* Clone();
    Resources::ITexture2DPtr GetTexture();
};

} // NS OpenGL
} // NS Display
} // NS OpenEngine

#endif // _OPENGL_TEXTURE_COPY_BACKEND_H_
