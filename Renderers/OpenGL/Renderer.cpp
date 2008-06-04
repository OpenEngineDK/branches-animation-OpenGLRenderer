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
#include <Display/Viewport.h>
#include <Display/IViewingVolume.h>
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
using OpenEngine::Display::Viewport;
using OpenEngine::Display::IViewingVolume;
using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;

GLSLVersion Renderer::glslversion = GLSL_UNKNOWN;

Renderer::Renderer() : farPlane(-1) {

}

/**
 * Renderer destructor.
 * Deletes all the attached rendering views.
 */
Renderer::~Renderer() {
    // Delete list of IRenderingViews
    list<IRenderingView*>::iterator itr;
    for(itr=vRenderingView.begin(); itr!=vRenderingView.end(); ++itr)
        delete *itr;
    vRenderingView.clear();
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
        return;
    }

    // Find shader version and if supported load them.
    InitializeGLSLVersion();
    if (Renderer::glslversion != GLSL_NONE) {
        ShaderLoader shadLoad;
        root->Accept(shadLoad);
    }

    // Start loading textures by traversing the scene graph,
    // and load in textures when GeometryNodes in the scene
    // graph has FaceSets with associated texture files.
    TextureLoader texLoad;
    root->Accept(texLoad);
}

/**
 * @note The processing function assumes that the scene has not been
 *       replaced by null since the initialization face. 
 */
void Renderer::Process(const float deltaTime, const float percent) {

    //glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    // setup lighting
    root->Accept(lv);

    //glPopAttrib();

    // For each RenderingView render the viewport.
    list<IRenderingView*>::iterator itr;
    for(itr=vRenderingView.begin(); itr!=vRenderingView.end(); ++itr) {

        Viewport& viewport = (*itr)->GetViewport();
        IViewingVolume* volume = viewport.GetViewingVolume();

        // If no viewing volume is set for the viewport ignore it.
        if (volume == NULL) continue;
        volume->SignalRendering(deltaTime);

        // Set viewport size
        Vector<4,int> d = viewport.GetDimension();
        glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);

        // Select The Projection Matrix
        glMatrixMode(GL_PROJECTION);

        // Reset The Projection Matrix
        glLoadIdentity();

         float fov = volume->GetFOV()/PI*180;
         IViewingVolume::ProjectionMode projectionMode = volume->GetProjectionMode();
         if ( projectionMode == IViewingVolume::OE_PERSPECTIVE ) {
            // Projection, e.g. Frustum
            gluPerspective(fov, volume->GetAspect(), volume->GetNear(), (farPlane > 0) ? farPlane : volume->GetFar());
         } else if ( projectionMode == IViewingVolume::OE_ORTHOGONAL ) {
            // Orthogonal, e.g. Orthotope
            glOrtho(-(volume->GetAspect()*fov),(volume->GetAspect()*fov),-fov,fov, volume->GetNear(), (farPlane > 0) ? farPlane : volume->GetFar());
         } else {
            // Error
            logger.error << "ProjectionMode not set in ViewingVolume "
                     << logger.end;
         }

        // Select the modelview matrix
        glMatrixMode(GL_MODELVIEW);

        // Reset the modelview matrix
        glLoadIdentity();

        // Get the view matrix and apply it
        Matrix<4,4,float> matrix = volume->GetViewMatrix();
        float f[16] = {0};
        matrix.ToArray(f);
        glMultMatrixf(f);

        // Start traversing the scene
        (*itr)->Render(this, root);

        // turn off lights
        for (int i = 0; i < lv.count; i++) {
            glDisable(GL_LIGHT0 + i);
        }
        
        lv.count = 0;
    }
}

void Renderer::Deinitialize() {
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


//Visitor methods for setting up the lighting

Renderer::LightVisitor::LightVisitor(): count(0)  {
    pos[0] = 0.0;
    pos[1] = 0.0;
    pos[2] = 0.0;
    pos[3] = 1.0;

    dir[0] = 0.0;
    dir[1] = 0.0;
    dir[2] = -1.0;
    dir[3] = 0.0;
}

Renderer::LightVisitor::~LightVisitor() {}
        
void Renderer::LightVisitor::VisitTransformationNode(TransformationNode* node) {
    
    // push transformation matrix to model view stack
    Matrix<4,4,float> m = node->GetTransformationMatrix();
    float f[16];
    m.ToArray(f);
    glPushMatrix();
    glMultMatrixf(f);
    // traverse sub nodes
    node->VisitSubNodes(*this);
    // pop transformation matrix
    glPopMatrix();
}
    
void Renderer::LightVisitor::VisitDirectionalLightNode(DirectionalLightNode* node) {
    if (!node->active)
        return;
    
    if (count >= GL_MAX_LIGHTS) {
        logger.warning << "Too many lights in scene: " << count+1 << logger.end;
        return;
    }

    glEnable(GL_LIGHT0 + count);

    glLightfv(GL_LIGHT0 + count, GL_POSITION, dir);

    float color[4];
    
    node->ambient.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_AMBIENT, color);
    
    node->diffuse.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_DIFFUSE, color);
    
    node->specular.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_SPECULAR, color);
    
    count++;
}
    
void Renderer::LightVisitor::VisitPointLightNode(PointLightNode* node) {
    if (!node->active) {
        logger.warning << "Too many lights in scene: " << count+1 << logger.end;
        return;
    }
        
    if (count >= GL_MAX_LIGHTS)
        return;

    glEnable(GL_LIGHT0 + count);

    // r->DrawPoint(pos,Vector<3,float>(0.0,0.0,1.0), 5);
    // logger.info << "light pos: " << pos << logger.end;
 
    glLightfv(GL_LIGHT0 + count, GL_POSITION, pos);
            
    float color[4];

    node->ambient.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_AMBIENT, color);

    node->diffuse.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_DIFFUSE, color);
           
    node->specular.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_SPECULAR, color);
            
    glLightf(GL_LIGHT0 + count, GL_CONSTANT_ATTENUATION, node->constAtt);
    glLightf(GL_LIGHT0 + count, GL_LINEAR_ATTENUATION, node->linearAtt);
    glLightf(GL_LIGHT0 + count, GL_QUADRATIC_ATTENUATION, node->quadAtt);
            
    count++;
    node->VisitSubNodes(*this);            
}

void Renderer::LightVisitor::VisitSpotLightNode(SpotLightNode* node) {
    if (!node->active)
        return;
        
    if (count >= GL_MAX_LIGHTS) {
        logger.warning << "Too many lights in scene: " << count+1 << logger.end;
        return;
    }
    glEnable(GL_LIGHT0 + count);
 
    glLightfv(GL_LIGHT0 + count, GL_POSITION, pos);
    glLightfv(GL_LIGHT0 + count, GL_SPOT_DIRECTION, dir);
    glLightf(GL_LIGHT0 + count, GL_SPOT_CUTOFF, node->cutoff);            
    glLightf(GL_LIGHT0 + count, GL_SPOT_EXPONENT, node->exponent);            

    float color[4];

    node->ambient.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_AMBIENT, color);

    node->diffuse.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_DIFFUSE, color);
           
    node->specular.ToArray(color);
    glLightfv(GL_LIGHT0 + count, GL_SPECULAR, color);
            
    glLightf(GL_LIGHT0 + count, GL_CONSTANT_ATTENUATION, node->constAtt);
    glLightf(GL_LIGHT0 + count, GL_LINEAR_ATTENUATION, node->linearAtt);
    glLightf(GL_LIGHT0 + count, GL_QUADRATIC_ATTENUATION, node->quadAtt);
            
    count++;
    node->VisitSubNodes(*this);            
}




} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
