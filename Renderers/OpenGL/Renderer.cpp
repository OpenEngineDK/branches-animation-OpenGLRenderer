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

#include <Scene/TransformationNode.h>
#include <Scene/LightNode.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/SpotLightNode.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace OpenEngine::Math;

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;

GLSLVersion Renderer::glslversion = GLSL_UNKNOWN;

Renderer::Renderer() {

}

/**
 * Renderer destructor.
 * Performs no clean up.
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
}

void Renderer::Handle(InitializeEventArg arg) {
    CHECK_FOR_GL_ERROR();

    InitializeGLSLVersion(); //@todo: HACK - to get Inseminator to work
    CHECK_FOR_GL_ERROR();

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
    CHECK_FOR_GL_ERROR();

    // Check that we have a scene.
    if (root == NULL)
        throw Exception("No scene root found while rendering.");

    this->initialize.Notify(RenderingEventArg(*this));
    CHECK_FOR_GL_ERROR();
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void Renderer::Handle(ProcessEventArg arg) {
    // run the processing phases
    RenderingEventArg rarg(*this, arg.start, arg.approx);
    this->preProcess.Notify(rarg);
    this->process.Notify(rarg);
    this->postProcess.Notify(rarg);
}

void Renderer::Handle(DeinitializeEventArg arg) {
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


bool Renderer::IsGLSLSupported() {
    return (glslversion != GLSL_NONE && glslversion != GLSL_UNKNOWN);
}

GLSLVersion Renderer::GetGLSLVersion() {
    return glslversion;
}

void Renderer::LoadTexture(ITextureResourcePtr texr) {
    // check for null pointers
    if (texr == NULL) return;

    // signal we need the texture data
    texr->Load();

    // bind the texture
    RebindTexture(texr);

    // signal we are done with the texture data
    texr->Unload();
}

void Renderer::RebindTexture(ITextureResourcePtr texr) {
    // check for null pointers
    if (texr == NULL) return;

    GLuint texid = texr->GetID();
    if (texid == 0) {
        //@todo: check that there is a gl context

        glGenTextures(1, &texid);
        CHECK_FOR_GL_ERROR();
        texr->SetID(texid);
    }

    // if the texture has been loaded delete it before reloading
    glDeleteTextures(1, &texid); //ignored by gl if not loaded
    CHECK_FOR_GL_ERROR();
            
    glBindTexture(GL_TEXTURE_2D, texid);
    CHECK_FOR_GL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_FOR_GL_ERROR();
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_FOR_GL_ERROR();
        
    GLuint depth = 0;
    switch (texr->GetDepth()) {
    case 8:  depth = GL_LUMINANCE; break;
    case 24: depth = GL_RGB;   break;
    case 32: depth = GL_RGBA;  break;
    default: logger.warning << "Unsupported color depth: " 
                            << texr->GetDepth() << logger.end;
    }
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 depth,
                 texr->GetWidth(),
                 texr->GetHeight(),
                 0,
                 depth,
                 GL_UNSIGNED_BYTE,
                 texr->GetData());
    CHECK_FOR_GL_ERROR();
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

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
