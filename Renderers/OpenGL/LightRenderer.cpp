// OpenGL light preprocessor implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Renderers/OpenGL/LightRenderer.h>

#include <Meta/OpenGL.h>

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


LightRenderer::LightRenderer(): count(0)  {
    pos[0] = 0.0;
    pos[1] = 0.0;
    pos[2] = 0.0;
    pos[3] = 1.0;

    dir[0] = 0.0;
    dir[1] = 0.0;
    dir[2] = -1.0;
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
    if (!node->active)
        return;
    
    if (count >= GL_MAX_LIGHTS) {
        logger.warning << "OpenGL: Too many lights in scene. Ignoring light no. " << count+1 << logger.end;
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
    
void LightRenderer::VisitPointLightNode(PointLightNode* node) {
    if (!node->active) {
        logger.warning << "OpenGL: Too many lights in scene. Ignoring light no. " << count+1 << logger.end;
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

void LightRenderer::VisitSpotLightNode(SpotLightNode* node) {
    if (!node->active)
        return;
        
    if (count >= GL_MAX_LIGHTS) {
        logger.warning << "OpenGL: Too many lights in scene. Ignoring light no. " << count+1 << logger.end;
        return;
    }
    glEnable(GL_LIGHT0 + count);
 
    // TODO: maybe check of attributes have changed to save gl calls?
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

void LightRenderer::Handle(RenderingEventArg arg) {
    // turn off lights
    for (int i = 0; i < GL_MAX_LIGHTS; i++) {
        glDisable(GL_LIGHT0 + i);
    }

    count = 0;
    arg.renderer.GetSceneRoot()->Accept(*this);
}


} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
