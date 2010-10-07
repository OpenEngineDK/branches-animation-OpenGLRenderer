#include <Renderers/OpenGL/FBOBufferedRenderer.h>

#include <string>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

    FBOBufferedRenderer::FBOBufferedRenderer(/*Viewport* viewport*/)
    : peer(Renderer(/*viewport*/))
    , fbo(0)
    , depthbuffer(0)
    , img(0)
{
    // Vector<4,int> dim(viewport->GetDimension());
    // width  = dim[2];
    // height = dim[3];
    colorbuf = ITexture2DPtr(new ColorBuffer(*this));
}

FBOBufferedRenderer::~FBOBufferedRenderer() {
    // the viewport is deleted in the super class (Renderer)
}

ITexture2DPtr FBOBufferedRenderer::GetColorBuffer() const {
    return colorbuf;
}

void FBOBufferedRenderer::Handle(Display::InitializeEventArg arg) {
    CHECK_FOR_GL_ERROR();

    peer.Handle(arg);
    CHECK_FOR_GL_ERROR();

    //@todo: make sure that there is a gl context

    const std::string fboExt = "GL_EXT_framebuffer_object";
    if (glewGetExtension(fboExt.c_str()) != GL_TRUE )
        throw Exception(fboExt + " not supported");

    glGenFramebuffersEXT(1, &fbo);
    CHECK_FOR_GL_ERROR();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    CHECK_FOR_GL_ERROR();

    // Adding a depth buffer
    glGenRenderbuffersEXT(1, &depthbuffer);
    CHECK_FOR_GL_ERROR();
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
    CHECK_FOR_GL_ERROR();
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
                             GL_DEPTH_COMPONENT, 
                             width, height);
    CHECK_FOR_GL_ERROR();
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                 GL_DEPTH_ATTACHMENT_EXT,
                                 GL_RENDERBUFFER_EXT, depthbuffer);
    CHECK_FOR_GL_ERROR();

    // Adding a Texture
    glGenTextures(1, &img);
    CHECK_FOR_GL_ERROR();
    colorbuf->SetID(img);

    glBindTexture(GL_TEXTURE_2D, img);
    CHECK_FOR_GL_ERROR();

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
    CHECK_FOR_GL_ERROR();
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
                              GL_COLOR_ATTACHMENT0_EXT, 
                              GL_TEXTURE_2D, img, 0);
    CHECK_FOR_GL_ERROR();

    // check FBO state for errors
#ifdef OE_DEBUG_GL
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        throw Exception("Incomplete frame buffer object: " + EnumToString(status) );
#endif

    // done initializing, go back to the main gl context 
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); //unbind
    CHECK_FOR_GL_ERROR();
}

std::string FBOBufferedRenderer::EnumToString(GLenum status) {
    std::string error = "";
    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        error = "GL_FRAMEBUFFER_UNSUPPORTED_EXT\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        error = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        error = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: 
        error = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        error = "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        error = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        error = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n";
        break;
    default: break;
    }
    return error;
}


// void FBOBufferedRenderer::Handle(ProcessEventArg arg) {
//     CHECK_FOR_GL_ERROR();

//     glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); //bind
//     CHECK_FOR_GL_ERROR();

//     glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//     CHECK_FOR_GL_ERROR();

//     peer.Handle(arg);
//     CHECK_FOR_GL_ERROR();

//     glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); //unbind
//     CHECK_FOR_GL_ERROR();

//     //RenderTextureInOrtho();
//     //CHECK_FOR_GL_ERROR();

//     //colorbuf->ChangedEvent().
//     //    Notify(Resources::TextureChangedEventArg(colorbuf));
// }

void FBOBufferedRenderer::RenderTextureInOrtho() {
    GLboolean l = glIsEnabled(GL_LIGHTING);
    GLboolean t = glIsEnabled(GL_TEXTURE_2D);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    CHECK_FOR_GL_ERROR();

    // quad rendering of texture
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    CHECK_FOR_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, img);
    CHECK_FOR_GL_ERROR();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    bool orth = false;
    if (!orth)
        gluPerspective(45.0,width/height,0.3,1000.0);
    else
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    CHECK_FOR_GL_ERROR();

    if (!orth)
        gluLookAt(0,1,4, 0,0,0, 0,1,0);

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.5);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.5);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.5);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-1.0, 1.0, 0.5);

    glEnd();
    CHECK_FOR_GL_ERROR();

    if (!t) glDisable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
}

void FBOBufferedRenderer::Handle(Display::DeinitializeEventArg arg) {
    glDeleteFramebuffersEXT(1, &fbo);
    CHECK_FOR_GL_ERROR();
    glDeleteRenderbuffersEXT(1, &depthbuffer);
    CHECK_FOR_GL_ERROR();
    peer.Handle(arg);
    CHECK_FOR_GL_ERROR();
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
