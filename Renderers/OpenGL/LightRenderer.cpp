// OpenGL light preprocessor implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers/OpenGL/LightRenderer.h>
#include <Scene/TransformationNode.h>
#include <Scene/LightNode.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/SpotLightNode.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using OpenEngine::Math::Vector;
using OpenEngine::Math::Matrix;


LightRenderer::LightRenderer()  {
    pos[0] = 0.0;
    pos[1] = 0.0;
    pos[2] = 0.0;
    pos[3] = 1.0;

    dir[0] = 0.0;
    dir[1] = -1.0;
    dir[2] = 0.0;
    dir[3] = 0.0;
}

LightRenderer::~LightRenderer() {}
        
void LightRenderer::VisitTransformationNode(TransformationNode* node) {
    
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
    
void LightRenderer::VisitDirectionalLightNode(DirectionalLightNode* node) {
    if (!node->active) {
        node->VisitSubNodes(*this);            
        return;
    } 
    
    if (lightCount <= GL_MAX_LIGHTS) {
        GLint light = GL_LIGHT0+lightCount;
        float color[4];
        glLightfv(light, GL_POSITION, dir);
        node->ambient.ToArray(color);
        glLightfv(light, GL_AMBIENT, color);
        node->diffuse.ToArray(color);
        glLightfv(light, GL_DIFFUSE, color);
        node->specular.ToArray(color);
        glLightfv(light, GL_SPECULAR, color);
        glEnable(light);
        lightCount++;
    }
    else {
        logger.warning << "OpenGL: Too many lights in scene. Ignoring light no. " 
                       << lightCount << logger.end;
    }
    node->VisitSubNodes(*this);            
}
    
void LightRenderer::VisitPointLightNode(PointLightNode* node) {
    if (!node->active) {
        node->VisitSubNodes(*this);            
        return;
    } 
    if (lightCount <= GL_MAX_LIGHTS) {
        GLint light = GL_LIGHT0+lightCount;
        float color[4];
        glLightfv(light, GL_POSITION, pos);
        node->ambient.ToArray(color);
        glLightfv(light, GL_AMBIENT, color);
        node->diffuse.ToArray(color);
        glLightfv(light, GL_DIFFUSE, color);
        node->specular.ToArray(color);
        glLightfv(light, GL_SPECULAR, color);
        glLightf(light, GL_CONSTANT_ATTENUATION, node->constAtt);
        glLightf(light, GL_LINEAR_ATTENUATION, node->linearAtt);
        glLightf(light, GL_QUADRATIC_ATTENUATION, node->quadAtt);
        glEnable(light);
        lightCount++;
    }
    else {
        logger.warning << "OpenGL: Too many lights in scene. Ignoring light no. " 
                       << lightCount << logger.end;
    }
    node->VisitSubNodes(*this);            
}

void LightRenderer::VisitSpotLightNode(SpotLightNode* node) {
    if (!node->active) {
        node->VisitSubNodes(*this);            
        return;
    } 
    
    if (lightCount <= GL_MAX_LIGHTS) {
        GLint light = GL_LIGHT0+lightCount;
        float color[4];
        glLightfv(light, GL_POSITION, pos);
        glLightfv(light, GL_SPOT_DIRECTION, dir);
        glLightf(light, GL_SPOT_CUTOFF, node->cutoff);            
        glLightf(light, GL_SPOT_EXPONENT, node->exponent);            
        node->ambient.ToArray(color);
        glLightfv(light, GL_AMBIENT, color);
        node->diffuse.ToArray(color);
        glLightfv(light, GL_DIFFUSE, color);
        node->specular.ToArray(color);
        glLightfv(light, GL_SPECULAR, color);
        glLightf(light, GL_CONSTANT_ATTENUATION, node->constAtt);
        glLightf(light, GL_LINEAR_ATTENUATION, node->linearAtt);
        glLightf(light, GL_QUADRATIC_ATTENUATION, node->quadAtt);
        glEnable(light);
        lightCount++;
    }
    else {
        logger.warning << "OpenGL: Too many lights in scene. Ignoring light no. " 
                       << lightCount << logger.end;
    }
    node->VisitSubNodes(*this);            
}

void LightRenderer::Handle(RenderingEventArg arg) {
    // turn off lights
    for (int i = 0; i < GL_MAX_LIGHTS; i++) {
        //logger.info << "disable light " << i << logger.end;
        glDisable(GL_LIGHT0 + i);
    }

    glEnable(GL_LIGHTING);
    lightCount = 0;
    arg.renderer.GetSceneRoot()->Accept(*this);
}


} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
