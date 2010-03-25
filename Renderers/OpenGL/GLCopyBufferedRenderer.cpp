#include <Renderers/OpenGL/GLCopyBufferedRenderer.h>

#include <string>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

GLCopyBufferedRenderer::GLCopyBufferedRenderer(Viewport* viewport)
    : peer(Renderer(viewport))
    , depthbuffer(0)
    , img(0)
{
    Vector<4,int> dim(viewport->GetDimension());
    width  = dim[2];
    height = dim[3];
    colorbuf = ITexture2DPtr(new ColorBuffer(*this));
}

GLCopyBufferedRenderer::~GLCopyBufferedRenderer() {
    // the viewport is deleted in the super class (Renderer)
}

ITexture2DPtr GLCopyBufferedRenderer::GetColorBuffer() const {
    return colorbuf;
}

void GLCopyBufferedRenderer::Handle(InitializeEventArg arg) {
    CHECK_FOR_GL_ERROR();

    peer.Handle(arg);
    CHECK_FOR_GL_ERROR();

    //@todo: make sure that there is a gl context

    // Adding a Texture
    glGenTextures(1, &img);
    CHECK_FOR_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, img);
    CHECK_FOR_GL_ERROR();
    colorbuf->SetID(img);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
    CHECK_FOR_GL_ERROR();
}

void GLCopyBufferedRenderer::Handle(ProcessEventArg arg) {
    CHECK_FOR_GL_ERROR();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    CHECK_FOR_GL_ERROR();

    peer.Handle(arg);
    CHECK_FOR_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D,img);
    CHECK_FOR_GL_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);
    CHECK_FOR_GL_ERROR();

}

void GLCopyBufferedRenderer::Handle(DeinitializeEventArg arg) {
    peer.Handle(arg);
    CHECK_FOR_GL_ERROR();
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
