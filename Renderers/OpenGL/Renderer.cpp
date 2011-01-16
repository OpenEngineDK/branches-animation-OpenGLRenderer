// OpenGL renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers/IRenderingView.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Scene/ISceneNode.h>
#include <Logging/Logger.h>
#include <Meta/OpenGL.h>
#include <Math/Math.h>
#include <Display/IViewingVolume.h>

#include <Scene/TransformationNode.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/SpotLightNode.h>

#include <Resources/ITexture2D.h>
#include <Resources/ITexture3D.h>
#include <Resources/FrameBuffer.h>

#include <Resources/OpenGLShader.h>

using namespace OpenEngine::Resources;

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace OpenEngine::Math;

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;
using OpenEngine::Display::IViewingVolume;

GLSLVersion Renderer::glslversion = GLSL_UNKNOWN;

Renderer::Renderer(): init(false)
{
    //backgroundColor = Vector<4,float>(1.0);
}

/**
 * Renderer destructor.
 * Deletes the internal viewport.
 */
Renderer::~Renderer() {
}

void Renderer::InitializeGLSLVersion() {
    // Initialize the "OpenGL Extension Wrangler" library
    GLenum err = glewInit();
    if (err!=GLEW_OK)
        logger.error << "GLEW: "
                     << (const char*)glewGetErrorString(err)
                     << logger.end;
    else {
        logger.info << "OpenGL: "
                    << (const char*)glGetString(GL_VERSION)
                    << " GLEW: "
                    << (const char*)glewGetString(GLEW_VERSION)
                    << logger.end;

		if (glewIsSupported("GL_VERSION_2_0")) {
            glslversion = GLSL_20;
			logger.info << "Using OpenGL version 2.0 with GLSL: "
                        << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)
                        << logger.end;
		}
		else if (glewIsSupported("GL_VERSION_1_4") &&
                 GLEW_ARB_vertex_shader &&
                 GLEW_ARB_fragment_shader) {
            glslversion = GLSL_14;
			logger.info << "Using OpenGL version 1.4 with shaders as extensions"
                        << logger.end;
		}
		else {
            glslversion = GLSL_NONE;
            logger.info << "GLSL not supported - shaders are disabled"
                        << logger.end;
		}
    }

    OpenGLShader::ShaderSupport();
}

void Renderer::SetupTexParameters(ITexture2D* tex){
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_FOR_GL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,         tex->GetWrapping());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,         tex->GetWrapping());
    if (tex->UseMipmapping()){
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,    GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->GetFiltering());
    }else{
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,    GL_FALSE);
        if (tex->GetFiltering() == NONE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    if (tex->GetFiltering() == NONE)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_FOR_GL_ERROR();
}

void Renderer::SetupTexParameters(ITexture3D* tex){
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_FOR_GL_ERROR();

    GLenum target = tex->GetUseCase();

    glTexParameteri(target, GL_TEXTURE_WRAP_S,         tex->GetWrapping());
    glTexParameteri(target, GL_TEXTURE_WRAP_T,         tex->GetWrapping());
    glTexParameteri(target, GL_TEXTURE_WRAP_R,         tex->GetWrapping());
    if (tex->UseMipmapping()){
        glTexParameteri(target, GL_GENERATE_MIPMAP,    GL_TRUE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, tex->GetFiltering());
    }else{
        glTexParameteri(target, GL_GENERATE_MIPMAP,    GL_FALSE);
        if (tex->GetFiltering() == NONE)        
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    if (tex->GetFiltering() == NONE)
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_FOR_GL_ERROR();
}

void Renderer::SetTextureCompression(ITexture* tex){
    if (compressionSupport && tex->UseCompression()){
        switch(tex->GetColorFormat()){
        case ALPHA:
            tex->SetColorFormat(ALPHA_COMPRESSED);
            break;
        case LUMINANCE:
            tex->SetColorFormat(LUMINANCE_COMPRESSED);
            break;
        case LUMINANCE_ALPHA:
            tex->SetColorFormat(LUMINANCE_ALPHA_COMPRESSED);
            break;
        case RGB:
            tex->SetColorFormat(RGB_COMPRESSED);
            break;
        case RGBA:
            tex->SetColorFormat(RGBA_COMPRESSED);
            break;
        default:
            throw Core::Exception("unknown compression type");
        }
    }
}

GLint Renderer::GLInternalColorFormat(ColorFormat f){
    switch (f) {
    case ALPHA:
        return GL_ALPHA;
    case LUMINANCE: 
        return GL_LUMINANCE;
    case LUMINANCE_ALPHA: 
        return GL_LUMINANCE_ALPHA;
    case BGR:
    case RGB: 
        return GL_RGB;
    case BGRA: 
    case RGBA: 
        return GL_RGBA;
    case ALPHA_COMPRESSED: return GL_COMPRESSED_ALPHA;
    case LUMINANCE_COMPRESSED: return GL_COMPRESSED_LUMINANCE;
    case LUMINANCE32F: return GL_R32F;
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

GLenum Renderer::GLColorFormat(ColorFormat f){
    switch (f) {
    case ALPHA:
    case ALPHA_COMPRESSED:
        return GL_ALPHA;
    case LUMINANCE: 
    case LUMINANCE_COMPRESSED: 
    case LUMINANCE32F:
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

unsigned int Renderer::GLTypeSize(Type t){
    switch(t){
    case Types::UBYTE:
        return sizeof(GLubyte);
    case Types::SBYTE:
        return sizeof(GLbyte);
    case Types::UINT:
        return sizeof(GLuint);
    case Types::INT:
        return sizeof(GLint);
    case Types::FLOAT:
        return sizeof(GLfloat);
    case Types::DOUBLE:
        return sizeof(GLdouble);
    case Types::NOTYPE:
        return 0;
    }
    return sizeof(GLshort);
}

GLenum Renderer::GLAccessType(BlockType b, UpdateMode u){
    if (u == STATIC){
        switch (b){
        case PIXEL_PACK:
            return GL_STATIC_COPY;
        default:
            return GL_STATIC_DRAW;
        }
    }else if (u == DYNAMIC){
        switch (b){
        case PIXEL_PACK:
            return GL_DYNAMIC_COPY;
        default:
            return GL_DYNAMIC_DRAW;
        }
    }
    return GL_STATIC_DRAW;
}

void Renderer::Handle(Renderers::InitializeEventArg arg) {
    if (init) return;
    //logger.info << "INITIALIZE RENDERER" << logger.end;
    CHECK_FOR_GL_ERROR();

    InitializeGLSLVersion(); //@todo: HACK - to get Inseminator to work
    CHECK_FOR_GL_ERROR();

    // Check if texture2D arrays are supported
    texture2DArraySupport = glewGetExtension("GL_EXT_texture_array") == GL_TRUE;

    // Check if texture compression with s3tc and dxt1 is supported.
    compressionSupport = 
        glewGetExtension("GL_EXT_texture_compression_dxt1") == GL_TRUE &&
        glewGetExtension("GL_EXT_texture_compression_s3tc") == GL_TRUE;

    bufferSupport = glewIsSupported("GL_VERSION_2_0");
    fboSupport = glewGetExtension("GL_EXT_framebuffer_object") == GL_TRUE;
        
    // Vector<4,float> bgc = backgroundColor;
    // glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the OpenGL frame buffer.
    // glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    // CHECK_FOR_GL_ERROR();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);						   
    CHECK_FOR_GL_ERROR();

    // Set perspective calculations to most accurate
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    CHECK_FOR_GL_ERROR();

    this->stage = RENDERER_INITIALIZE;
    this->initialize.Notify(RenderingEventArg(arg.canvas, *this));
    this->stage = RENDERER_PREPROCESS;
    CHECK_FOR_GL_ERROR();

    init = true;
}

void Renderer::Handle(Renderers::ProcessEventArg arg) {
    // @todo: assert we are in preprocess stage

    Vector<4,float> bgc = backgroundColor;
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


    // run the processing phases
    RenderingEventArg rarg(arg.canvas, *this, arg.start, arg.approx);
    this->preProcess.Notify(rarg);


    IViewingVolume* volume = arg.canvas.GetViewingVolume();
    // If no viewing volume is set for the viewport ignore it.
    if (volume != NULL) {
        volume->SignalRendering(arg.approx);

        // Set viewport size 
        Vector<4,int> d(0, 0, arg.canvas.GetWidth(), arg.canvas.GetHeight());
        glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
        CHECK_FOR_GL_ERROR();

        // apply the volume
        ApplyViewingVolume(*volume);
    }
    CHECK_FOR_GL_ERROR();

    this->stage = RENDERER_PROCESS;
    this->process.Notify(rarg);
    this->stage = RENDERER_POSTPROCESS;
    this->postProcess.Notify(rarg);
    this->stage = RENDERER_PREPROCESS;
}


/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
// void Renderer::Handle(ProcessEventArg arg) {
//     logger.warning << "rendering should be triggered using RedrawEvent" << logger.end;
// }

void Renderer::Handle(Renderers::DeinitializeEventArg arg) {
    if (!init) return;
    this->stage = RENDERER_DEINITIALIZE;
    this->deinitialize.Notify(RenderingEventArg(arg.canvas, *this));
    init = false;
}

IEvent<RenderingEventArg>& Renderer::InitializeEvent() {
    return initialize;
}
IEvent<RenderingEventArg>& Renderer::PreProcessEvent() {
    return preProcess;
}
IEvent<RenderingEventArg>& Renderer::ProcessEvent() {
    return process;
}
IEvent<RenderingEventArg>& Renderer::PostProcessEvent() {
    return postProcess;
}
IEvent<RenderingEventArg>& Renderer::DeinitializeEvent() {
    return deinitialize;
}

void Renderer::ApplyViewingVolume(IViewingVolume& volume) {
    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    CHECK_FOR_GL_ERROR();

    // Reset The Projection Matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Setup OpenGL with the volumes projection matrix
    Matrix<4,4,float> projMatrix = volume.GetProjectionMatrix();
    float arr[16] = {0};
    projMatrix.ToArray(arr);
    glMultMatrixf(arr);
    CHECK_FOR_GL_ERROR();

    // Select the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    CHECK_FOR_GL_ERROR();

    // Reset the modelview matrix
    glLoadIdentity();
    CHECK_FOR_GL_ERROR();

    // Get the view matrix and apply it
    Matrix<4,4,float> matrix = volume.GetViewMatrix();
    float f[16] = {0};
    matrix.ToArray(f);
    glMultMatrixf(f);
    CHECK_FOR_GL_ERROR();
}

bool Renderer::IsGLSLSupported() {
    return (glslversion != GLSL_NONE && glslversion != GLSL_UNKNOWN);
}

bool Renderer::BufferSupport(){
    return bufferSupport;
}

bool Renderer::FrameBufferSupport(){
    return fboSupport;
}

GLSLVersion Renderer::GetGLSLVersion() {
    return glslversion;
}

void Renderer::LoadTexture(ITexture2DPtr texr) {
    LoadTexture(texr.get());
}
void Renderer::LoadTexture(ITexture2D* texr) {
    // check for null pointers
    if (texr == NULL) return;

    // check if textures has already been bound.
    if (texr->GetID() != 0) return;

    // signal we need the texture data if not loaded.
    bool loaded = true;
    if (texr->GetVoidDataPtr() == NULL){
        loaded = false;
        texr->Load(); //@todo: what the #@!%?
    }


    // Generate and bind the texture id.
    GLuint texid;
    glGenTextures(1, &texid);
    CHECK_FOR_GL_ERROR();

    texr->SetID(texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();
    
    SetupTexParameters(texr);
    CHECK_FOR_GL_ERROR();

    SetTextureCompression(texr);
    GLint internalFormat = GLInternalColorFormat(texr->GetColorFormat());
    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexImage2D(GL_TEXTURE_2D,
                 0, // mipmap level
                 internalFormat,
                 texr->GetWidth(),
                 texr->GetHeight(),
                 0, // border
                 colorFormat,
                 texr->GetType(),
                 texr->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();
    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Return the texture in the state we got it.
    if (!loaded)
        texr->Unload();
}

void Renderer::LoadTexture(ITexture3DPtr texr) {
    LoadTexture(texr.get());
}
void Renderer::LoadTexture(ITexture3D* texr) {
    // check for null pointers
    if (texr == NULL) return;

    // check if textures has already been bound.
    if (texr->GetID() != 0) return; // @todo: throw exception!

    // signal we need the texture data if not loaded.
    bool loaded = true;
    if (texr->GetVoidDataPtr() == NULL){
        loaded = false;
        texr->Load(); //@todo: what the #@!%?
    }

    if (!texture2DArraySupport){
        // Use texture 3d instead
        texr->SetUseCase(ITexture3D::TEXTURE3D);
        // disable mipmapping to preserve the individual slices.
        texr->SetMipmapping(false);
    }

    // Generate and bind the texture id.
    GLuint texid;
    glGenTextures(1, &texid);
    CHECK_FOR_GL_ERROR();

    texr->SetID(texid);
    glBindTexture(texr->GetUseCase(), texid);
    CHECK_FOR_GL_ERROR();
    
    SetupTexParameters(texr);
    CHECK_FOR_GL_ERROR();

    GLint internalFormat = GLInternalColorFormat(texr->GetColorFormat());
    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexImage3D(texr->GetUseCase(),
                 0, // mipmap level
                 internalFormat,
                 texr->GetWidth(),
                 texr->GetHeight(),
                 texr->GetDepth(),
                 0, // border
                 colorFormat,
                 texr->GetType(),
                 texr->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();
    
    // Return the texture in the state we got it.
    if (!loaded)
        texr->Unload();
}

void Renderer::RebindTexture(ITexture2DPtr texr, unsigned int xOffset, unsigned int yOffset, unsigned int width, unsigned int height) {
    RebindTexture(texr.get(), xOffset, yOffset, width, height);
}
void Renderer::RebindTexture(ITexture2D* texr, unsigned int xOffset, unsigned int yOffset, unsigned int width, unsigned int height) {
    // check for null pointers
    if (texr == NULL) return;

#if OE_SAFE
    if (texr->GetID() == 0)
        throw Exception("Trying to rebind unbound texture.");
#endif

    // Bind the texture
    GLuint texid = texr->GetID();
    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();

    // Setup texture parameters
    SetupTexParameters(texr);

    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    xOffset,
                    yOffset,
                    width,
                    height,
                    colorFormat,
                    texr->GetType(),
                    texr->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();

}

void Renderer::RebindTexture(ITexture3DPtr texr, unsigned int xOffset, unsigned int yOffset, unsigned int zOffset, unsigned int width, unsigned int height, unsigned int depth) {
    RebindTexture(texr.get(), xOffset, yOffset, zOffset, width, height, depth);
}
void Renderer::RebindTexture(ITexture3D* texr, unsigned int xOffset, unsigned int yOffset, unsigned int zOffset, unsigned int width, unsigned int height, unsigned int depth) {
    // check for null pointers
    if (texr == NULL) return;

#if OE_SAFE
    if (texr->GetID() == 0)
        throw Exception("Trying to rebind unbound texture.");
#endif

    // Bind the texture
    GLuint texid = texr->GetID();
    glBindTexture(texr->GetUseCase(), texid);
    CHECK_FOR_GL_ERROR();

    // Setup texture parameters
    SetupTexParameters(texr);

    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexSubImage3D(texr->GetUseCase(),
                    0,
                    xOffset,
                    yOffset,
                    zOffset,
                    width,
                    height,
                    depth,
                    colorFormat,
                    texr->GetType(),
                    texr->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();

}

void Renderer::BindFrameBuffer(FrameBuffer* fb){
#if OE_SAFE
    if (fb == NULL) throw Exception("Cannot bind NULL frame buffer");
    if (fb->GetID() != 0) throw Exception("Will not bind already bound frame buffer.");
#endif

    GLuint fboID;
    glGenFramebuffersEXT(1, &fboID);
    CHECK_FOR_GL_ERROR();
    fb->SetID(fboID);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
    CHECK_FOR_GL_ERROR();

    /*
     * On some Platforms (OS X) the color textures MUST be bound
     * before the depth texture
     */

    if (fb->GetNumberOfAttachments() == 0) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    for (unsigned int i = 0; i < fb->GetNumberOfAttachments(); ++i){
        ITexture2DPtr tex = fb->GetTexAttachment(i);
        LoadTexture(tex.get());
        glBindTexture(GL_TEXTURE_2D, tex->GetID());  // Why is this needed?
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
                                  GL_COLOR_ATTACHMENT0_EXT + i,
                                  GL_TEXTURE_2D, tex->GetID(), 0);
        CHECK_FOR_GL_ERROR();
    }

    if (fb->GetDepthTexture() != NULL) {
        LoadTexture(fb->GetDepthTexture());
        glBindTexture(GL_TEXTURE_2D, fb->GetDepthTexture()->GetID()); // Why is this needed?
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
                                  GL_DEPTH_ATTACHMENT_EXT,
                                  GL_TEXTURE_2D, fb->GetDepthTexture()->GetID(), 0);
    } else {
        Vector<2, int> viewDim = fb->GetDimension();
        
        GLuint depth;
        glGenRenderbuffersEXT(1, &depth);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, viewDim[0], viewDim[1]);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
        
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT, depth);
        CHECK_FOR_GL_ERROR();
    }
    CHECK_FRAMEBUFFER_STATUS();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void Renderer::BindDataBlock(IDataBlock* bo){
#if OE_SAFE
    if (bo == NULL) throw Exception("Cannot bind NULL data block.");
    if (bo->GetID() != 0) throw Exception("Cannot bind already bound data block.");
    if (bo->GetVoidDataPtr() == NULL) throw Exception("Cannot bind already data block with no data.");
#endif

    if (bufferSupport){
        GLuint id;
        glGenBuffers(1, &id);
        CHECK_FOR_GL_ERROR();
    
        bo->SetID(id);
        glBindBuffer(bo->GetBlockType(), id);
        CHECK_FOR_GL_ERROR();
    
        unsigned int size = GLTypeSize(bo->GetType()) * bo->GetSize() * bo->GetDimension();
        GLenum access = GLAccessType(bo->GetBlockType(), bo->GetUpdateMode());
        
        glBufferData(bo->GetBlockType(), 
                     size,
                     bo->GetVoidDataPtr(), access);
        
        if (bo->GetUnloadPolicy() == UNLOAD_AUTOMATIC)
            bo->Unload();
    }
    // Do not unload if there's no buffer support. We will need the
    // data in memory client side to be able to pass it to the
    // graphics card..
}

void Renderer::DrawFace(FacePtr f) {
    if (f->mat->Get2DTextures().size() == 0) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, (*f->mat->Get2DTextures().begin()).second->GetID());
    }
    float col[4];
    f->mat->diffuse.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
    f->mat->ambient.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);
    f->mat->specular.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col);
    f->mat->emission.ToArray(col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, col);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, f->mat->shininess);
    glBegin(GL_TRIANGLES);
    // for each vertex ...
    for (int i=0; i<3; i++) {
        Vector<3,float> v = f->vert[i];
        Vector<2,float> t = f->texc[i];
        Vector<3,float> n = f->norm[i];
        Vector<4,float> c = f->colr[i];
        glTexCoord2f(t[0],t[1]);
        glColor4f (c[0],c[1],c[2],c[3]);
        glNormal3f(n[0],n[1],n[2]);
        glVertex3f(v[0],v[1],v[2]);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Helper function drawing a wired face.
 *
 * @param face face to draw.
 * @param color line color.
 * @param width line width, default i one.
 */
void Renderer::DrawFace(FacePtr face, Vector<3,float> color, float width) {
    GLboolean t = glIsEnabled(GL_TEXTURE_2D);
    GLboolean l = glIsEnabled(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();

    glLineWidth(width);
    CHECK_FOR_GL_ERROR();

    glBegin(GL_LINES);
        glColor3f(color[0],color[1],color[2]);
        glVertex3f(face->vert[0][0], face->vert[0][1], face->vert[0][2]);
        glVertex3f(face->vert[1][0], face->vert[1][1], face->vert[1][2]);

        glVertex3f(face->vert[1][0], face->vert[1][1], face->vert[1][2]);
        glVertex3f(face->vert[2][0], face->vert[2][1], face->vert[2][2]);

        glVertex3f(face->vert[2][0], face->vert[2][1], face->vert[2][2]);
        glVertex3f(face->vert[0][0], face->vert[0][1], face->vert[0][2]);
    glEnd();
    CHECK_FOR_GL_ERROR();

    // reset state
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
}

/**
 * Helper function drawing a line.
 *
 * @param line line to draw.
 * @param color line color.
 * @param width line width, default i one.
 */
void Renderer::DrawLine(Line line, Vector<3,float> color, float width) {
    GLboolean t = glIsEnabled(GL_TEXTURE_2D);
    GLboolean l = glIsEnabled(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();

    glLineWidth(width);
    CHECK_FOR_GL_ERROR();

    glBegin(GL_LINES);
        glColor3f(color[0],color[1],color[2]);
        glVertex3f(line.point1[0],line.point1[1],line.point1[2]);
        glVertex3f(line.point2[0],line.point2[1],line.point2[2]);
    glEnd();
    CHECK_FOR_GL_ERROR();

    // reset state 
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
}

/**
 * Helper function drawing a point.
 *
 * @param point point to draw.
 * @param color line color.
 * @param size dot size, default i one.
 */
void Renderer::DrawPoint(Vector<3,float> point, Vector<3,float> color , float size) {
    GLboolean t = glIsEnabled(GL_TEXTURE_2D);
    GLboolean l = glIsEnabled(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();

    glPointSize(size);
    CHECK_FOR_GL_ERROR();

    glBegin(GL_POINTS);
        glColor3f(color[0],color[1],color[2]);
        glVertex3f(point[0],point[1],point[2]);
    glEnd();
    CHECK_FOR_GL_ERROR();

    // reset state
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
}

void Renderer::SetBackgroundColor(Vector<4,float> color) {
    backgroundColor = color;
}

Vector<4,float> Renderer::GetBackgroundColor() {
     return backgroundColor;
}

/**
 * Helper function drawing a sphere.
 *
 * @param center Center of sphere.
 * @param radius Radius of sphere.
 * @param color  Color of sphere.
 */
    void Renderer::DrawSphere(Vector<3,float> center, float radius, Vector<3,float> color) {
    GLboolean t = glIsEnabled(GL_TEXTURE_2D);
    GLboolean l = glIsEnabled(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();

    glPushMatrix();
    glTranslatef(center[0], center[1], center[2]);
    glColor3f(color[0], color[1], color[2]);
    GLUquadricObj* qobj = gluNewQuadric();
    glLineWidth(1);
    gluQuadricNormals(qobj, GLU_SMOOTH);
    gluQuadricDrawStyle(qobj, GLU_LINE);
    gluQuadricOrientation(qobj, GLU_INSIDE);
    gluSphere(qobj, radius, 10, 10); 
    gluDeleteQuadric(qobj);
    glPopMatrix();

    // reset state
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
    CHECK_FOR_GL_ERROR();
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
