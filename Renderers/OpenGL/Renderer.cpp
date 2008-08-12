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
#include <Renderers/OpenGL/TextureLoader.h>
#include <Renderers/OpenGL/ShaderLoader.h>
#include <Scene/ISceneNode.h>
#include <Core/IGameEngine.h>
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

using OpenEngine::Core::IGameEngine;
using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;

GLSLVersion Renderer::glslversion = GLSL_UNKNOWN;

Renderer::Renderer() : farPlane(-1) {

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

void Renderer::Initialize() {
    InitializeGLSLVersion(); //@todo: HACK - to get Inseminator to work

    // Clear the OpenGL frame buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    // Enable lighting
    glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);						   

    // Set perspective calculations to most accurate
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Check that we have a scene.
    if (root == NULL) {
        logger.error << "No scene root found." << logger.end;
        IGameEngine::Instance().Stop();
        // @todo: throw exception here!
        return;
    }

    RenderingEventArg arg = { *this, 0 };

    this->initialize.Notify(arg);

    // Find shader version and if supported load them.
    // InitializeGLSLVersion();
    // if (Renderer::glslversion != GLSL_NONE) {
    //     ShaderLoader shadLoad;
    //     root->Accept(shadLoad);
    // }
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void Renderer::Process(const float deltaTime, const float percent) {
    // run the processing phases
    RenderingEventArg arg = { *this, deltaTime };
    this->preProcess.Notify(arg);
    this->process.Notify(arg);
    this->postProcess.Notify(arg);
}

void Renderer::Deinitialize() {
    RenderingEventArg arg = { *this, 0 };
    this->deinitialize.Notify(arg);
}

bool Renderer::IsTypeOf(const std::type_info& inf) {
    return ((typeid(Renderer) == inf) || IRenderer::IsTypeOf(inf));
}

bool Renderer::IsGLSLSupported() {
    return (glslversion != GLSL_NONE && glslversion != GLSL_UNKNOWN);
}

GLSLVersion Renderer::GetGLSLVersion() {
    return glslversion;
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
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glLineWidth(width);

    glBegin(GL_LINES);
        glColor3f(color[0],color[1],color[2]);
        glVertex3f(face->vert[0][0], face->vert[0][1], face->vert[0][2]);
        glVertex3f(face->vert[1][0], face->vert[1][1], face->vert[1][2]);

        glVertex3f(face->vert[1][0], face->vert[1][1], face->vert[1][2]);
        glVertex3f(face->vert[2][0], face->vert[2][1], face->vert[2][2]);

        glVertex3f(face->vert[2][0], face->vert[2][1], face->vert[2][2]);
        glVertex3f(face->vert[0][0], face->vert[0][1], face->vert[0][2]);
    glEnd();

    // reset state
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
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
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glLineWidth(width);

    glBegin(GL_LINES);
        glColor3f(color[0],color[1],color[2]);
        glVertex3f(line.point1[0],line.point1[1],line.point1[2]);
        glVertex3f(line.point2[0],line.point2[1],line.point2[2]);
    glEnd();

    // reset state 
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
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
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glPointSize(size);

    glBegin(GL_POINTS);
        glColor3f(color[0],color[1],color[2]);
        glVertex3f(point[0],point[1],point[2]);
    glEnd();

    // reset state
    if (t) glEnable(GL_TEXTURE_2D);
    if (l) glEnable(GL_LIGHTING);
}

void Renderer::SetFarPlane(float farPlane) {
    this->farPlane = farPlane;
}

float Renderer::GetFarPlane() {
    return farPlane;
}


} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
