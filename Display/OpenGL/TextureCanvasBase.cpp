// OpenGL texture base class for canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Display/OpenGL/TextureCanvasBase.h>
#include <Meta/OpenGL.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

    TextureCanvasBase::TextureCanvasBase()
        : ctex(new CustomTexture())
        , tex(ITexture2DPtr(ctex))
    {
        // ctex->width = width;
        // ctex->height = height;
        ctex->id = -1; // ugly hack: make sure that a texture loader wont try to bind this texture
        ctex->channels = 4;
        ctex->format = Resources::RGBA;
        ctex->type = Types::UBYTE;
    }
    
    TextureCanvasBase::~TextureCanvasBase() {
        
    }
    
    GLint GLInternalColorFormat(ColorFormat f){
        switch (f) {
        case ALPHA: 
        case LUMINANCE: 
            return 1; 
        case LUMINANCE_ALPHA: 
            return 2;
        case BGR:
        case RGB: 
            return 3;
        case BGRA: 
            return 4;  
        case RGBA: 
            return GL_RGBA;
        case ALPHA_COMPRESSED: return GL_COMPRESSED_ALPHA;
        case LUMINANCE_COMPRESSED: return GL_COMPRESSED_LUMINANCE;
        case LUMINANCE_ALPHA_COMPRESSED: return GL_COMPRESSED_LUMINANCE_ALPHA;
        case RGB_COMPRESSED: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case RGBA_COMPRESSED: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case RGB32F: return GL_RGB32F;
        case RGBA32F: return GL_RGBA32F;
        case DEPTH: return GL_DEPTH_COMPONENT;
        default: 
            logger.warning << "Unsupported color format: " << f << logger.end;
            logger.warning << "Defaulting to RGBA." << logger.end;
        }
        return GL_RGBA;
    }

    GLenum GLColorFormat(ColorFormat f){
        switch (f) {
        case ALPHA:
        case ALPHA_COMPRESSED:
            return GL_ALPHA;
        case LUMINANCE: 
        case LUMINANCE_COMPRESSED: 
            return GL_LUMINANCE;
        case LUMINANCE_ALPHA: 
        case LUMINANCE_ALPHA_COMPRESSED: 
            return GL_LUMINANCE_ALPHA;
        case RGB: 
        case RGB32F: 
        case RGB_COMPRESSED: 
            return GL_RGB;
        case RGBA: 
        case RGBA_COMPRESSED: 
        case RGBA32F: 
            return GL_RGBA;
        case BGR: 
            return GL_BGR;
        case BGRA: 
            return GL_BGRA;
        case DEPTH: 
            return GL_DEPTH_COMPONENT;
        default: 
            logger.warning << "Unsupported color format: " << f << logger.end;
            logger.warning << "Defaulting to RGBA." << logger.end;
        }
        return GL_RGBA;
    }
    
    void TextureCanvasBase::CreateTexture() {
        glGenTextures(1, &ctex->id);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, ctex->id);
        CHECK_FOR_GL_ERROR();
    }

    void TextureCanvasBase::SetupTexture() {
        GLenum colorFormat = GLColorFormat(ctex->GetColorFormat());
        GLenum internalFormat = GLInternalColorFormat(ctex->GetColorFormat());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                     ctex->width, ctex->height, 0, colorFormat, 
                     ctex->GetType(), NULL);
        CHECK_FOR_GL_ERROR();
    }

    void TextureCanvasBase::CopyToTexture() {
        GLenum internalFormat = GLInternalColorFormat(ctex->GetColorFormat());
        glBindTexture(GL_TEXTURE_2D, ctex->id);
        CHECK_FOR_GL_ERROR();
        glCopyTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 0, 0, ctex->width, ctex->height, 0);
        CHECK_FOR_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_FOR_GL_ERROR();
    }
    
    void TextureCanvasBase::SetTextureWidth(const unsigned int width) { 
        ctex->width = width; 
        SetupTexture();
    }

    void TextureCanvasBase::SetTextureHeight(const unsigned int height) { 
        ctex->height = height; 
        SetupTexture();
    }
    
    unsigned int TextureCanvasBase::GetTextureWidth() const { 
        return ctex->width; 
    }
    
    unsigned int TextureCanvasBase::GetTextureHeight() const { 
        return ctex->height; 
    }

} // NS OpenGL
} // NS Display
} // NS OpenEngine
