// OpenGL texture copy backend for canvases
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Display/OpenGL/TextureCopy.h>
#include <Meta/OpenGL.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {

using namespace Resources;

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
    
TextureCopy::TextureCopy()
    : ctex(new CustomTexture())
    , tex(ITexture2DPtr(ctex))
{
    ctex->id = -1; // ugly hack: make sure that a texture loader wont try to bind this texture
    ctex->channels = 4;
    ctex->format = Resources::RGBA;
    ctex->type = Types::UBYTE;
}
    
TextureCopy::~TextureCopy() {        
}
    
void TextureCopy::Create(const unsigned int width, const unsigned int height) {
    ctex->width = width;
    ctex->height = height;    
}

void TextureCopy::Init(const unsigned int width, const unsigned int height) {
    ctex->width = width;
    ctex->height = height;
    glGenTextures(1, &ctex->id);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, ctex->id);
    CHECK_FOR_GL_ERROR();
    GLenum colorFormat = GLColorFormat(ctex->GetColorFormat());
    GLenum internalFormat = GLInternalColorFormat(ctex->GetColorFormat());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                 ctex->width, ctex->height, 0, colorFormat, 
                 ctex->GetType(), NULL);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureCopy::Deinit() {
    //todo: unbind the texture
}

void TextureCopy::Pre() {

}

void TextureCopy::Post() {
    GLenum colorFormat = GLColorFormat(ctex->GetColorFormat());
    glBindTexture(GL_TEXTURE_2D, ctex->id);
    CHECK_FOR_GL_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, colorFormat, 0, 0, ctex->width, ctex->height, 0);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_FOR_GL_ERROR();
}
    
void TextureCopy::SetDimensions(const unsigned int width, const unsigned int height) { 
    ctex->width = width;
    ctex->height = height;
    if (ctex->id == (unsigned int)-1) return;
    //todo: update the texture
    glBindTexture(GL_TEXTURE_2D, ctex->id);
    CHECK_FOR_GL_ERROR();
    GLenum colorFormat = GLColorFormat(ctex->GetColorFormat());
    GLenum internalFormat = GLInternalColorFormat(ctex->GetColorFormat());
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                 ctex->width, ctex->height, 0, colorFormat, 
                 ctex->GetType(), NULL);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
}

ICanvasBackend* TextureCopy::Clone() {
    TextureCopy* clone = new TextureCopy();
    return clone;
}

Resources::ITexture2DPtr TextureCopy::GetTexture() {
    return tex;
}
    
} // NS OpenGL
} // NS Display
} // NS OpenEngine
