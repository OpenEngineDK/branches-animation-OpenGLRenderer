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
#include <Scene/LightNode.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/SpotLightNode.h>

#include <Resources/ITexture2D.h>
#include <Resources/ITexture3D.h>


using namespace OpenEngine::Resources;

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace OpenEngine::Math;

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;
using OpenEngine::Display::IViewingVolume;

GLSLVersion Renderer::glslversion = GLSL_UNKNOWN;

Renderer::Renderer(Viewport* viewport)
    : root(NULL) 
    , viewport(viewport)
{
    backgroundColor = Vector<4,float>(1.0);
}

/**
 * Renderer destructor.
 * Deletes the internal viewport.
 */
Renderer::~Renderer() {
    delete viewport;
}

Viewport& Renderer::GetViewport() const {
    return *viewport;
}

void Renderer::SetSceneRoot(ISceneNode* root) {
    this->root = root;
}

ISceneNode* Renderer::GetSceneRoot() const {
    return root;
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
}

void Renderer::SetupTexParameters(ITexture2D* tex){
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_FOR_GL_ERROR();

    switch(tex->GetWrapping()){
    case CLAMP_TO_EDGE:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        break;
    case CLAMP:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
        break;
    case REPEAT:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
        break;
#ifdef OE_SAFE
    default:
        throw Exception("Unknown texture wrapping");
#endif
    }
    if (tex->UseMipmapping()){
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,    GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }else{
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,    GL_FALSE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_FOR_GL_ERROR();
}

GLenum Renderer::GLType(Type t){
    switch(t){
    case UBYTE: return GL_UNSIGNED_BYTE; break;
    case BYTE: return GL_BYTE; break;
    case UINT: return GL_UNSIGNED_INT; break;
    case INT: return GL_INT; break;
    case FLOAT: return GL_FLOAT; break;
#ifdef DEBUG
    case NOTYPE:
        logger.warning << "No type set" << logger.end;
        break;
#endif
    default:
        logger.warning << "Unsupported texture type: " << t << logger.end;
        logger.warning << "Defaulting to unsigned byte." << logger.end;
    }
    return GL_UNSIGNED_BYTE;
}

GLint Renderer::GLInternalColorFormat(ColorFormat f){
    switch (f) {
    case LUMINANCE: 
        return 1; 
        break;
    case BGR:
    case RGB: 
        return 3;
        break;
    case BGRA: 
    case RGBA: 
        return 4;  
        break;
    case RGBA32F: return GL_RGBA32F; break;
    case DEPTH: return GL_DEPTH_COMPONENT;  break;
    default: logger.warning << "Unsupported color format: " << f << logger.end;
        logger.warning << "Defaulting to RGBA." << logger.end;
    }
    return GL_RGBA;
}


GLenum Renderer::GLColorFormat(ColorFormat f){
    switch (f) {
    case LUMINANCE: return GL_LUMINANCE; break;
    case RGB: return GL_RGB;   break;
    case RGBA32F: 
    case RGBA: return GL_RGBA;  break;
    case BGR: return GL_BGR;   break;
    case BGRA: return GL_BGRA;  break;
    case DEPTH: return GL_DEPTH_COMPONENT;  break;
    default: logger.warning << "Unsupported color format: " << f << logger.end;
        logger.warning << "Defaulting to RGBA." << logger.end;
    }
    return GL_RGBA;
}

void Renderer::Handle(InitializeEventArg arg) {
    CHECK_FOR_GL_ERROR();

    InitializeGLSLVersion(); //@todo: HACK - to get Inseminator to work
    CHECK_FOR_GL_ERROR();

    Vector<4,float> bgc = backgroundColor;
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the OpenGL frame buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    CHECK_FOR_GL_ERROR();

    // Enable lighting
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);						   
    CHECK_FOR_GL_ERROR();

    // Set perspective calculations to most accurate
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    CHECK_FOR_GL_ERROR();

    // Check that we have a scene.
    if (root == NULL)
        throw Exception("No scene root found while rendering.");

    this->initialize.Notify(RenderingEventArg(*this));
    this->stage = RENDERER_PREPROCESS;
    CHECK_FOR_GL_ERROR();
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void Renderer::Handle(ProcessEventArg arg) {
    // @todo: assert we are in preprocess stage

    Vector<4,float> bgc = backgroundColor;
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // run the processing phases
    RenderingEventArg rarg(*this, arg.start, arg.approx);
    this->preProcess.Notify(rarg);
    this->stage = RENDERER_PROCESS;
    this->process.Notify(rarg);
    this->stage = RENDERER_POSTPROCESS;
    this->postProcess.Notify(rarg);
    this->stage = RENDERER_PREPROCESS;
}

void Renderer::Handle(DeinitializeEventArg arg) {
    this->stage = RENDERER_DEINITIALIZE;
    this->deinitialize.Notify(RenderingEventArg(*this));
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
        texr->Load();
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

    GLenum type = GLType(texr->GetType());
    GLint internalFormat = GLInternalColorFormat(texr->GetColorFormat());
    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexImage2D(GL_TEXTURE_2D,
                 0, // mipmap level
                 internalFormat,
                 texr->GetWidth(),
                 texr->GetHeight(),
                 0, // border
                 colorFormat,
                 type,
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

#ifdef OE_SAFE
    if (texr->GetID() == 0)
        throw Exception("Trying to rebind unbound texture.");
#endif

    // Bind the texture
    GLuint texid = texr->GetID();
    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();

    // Setup texture parameters
    SetupTexParameters(texr);

    GLenum type = GLType(texr->GetType());
    GLenum colorFormat = GLColorFormat(texr->GetColorFormat());

    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    xOffset,
                    yOffset,
                    width,
                    height,
                    colorFormat,
                    type,
                    texr->GetVoidDataPtr());
    CHECK_FOR_GL_ERROR();

}

void Renderer::DrawFace(FacePtr f) {
    if (f->mat->texr == NULL) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, f->mat->texr->GetID());
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
