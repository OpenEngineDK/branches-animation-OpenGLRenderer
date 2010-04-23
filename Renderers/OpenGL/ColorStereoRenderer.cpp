// OpenGL color stero rendering link
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Renderers/OpenGL/ColorStereoRenderer.h>

#include <Display/StereoCamera.h>
#include <Display/OrthogonalViewingVolume.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

using namespace Display;

ColorStereoRenderer::ColorStereoRenderer() {
    
}

ColorStereoRenderer::~ColorStereoRenderer() {
    
}

inline void SetupTexture(unsigned int width, unsigned int height, GLuint format) {
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,
                 width, height, 0, format, 
                 GL_UNSIGNED_BYTE, NULL);
    CHECK_FOR_GL_ERROR();
    // resizeEvent.Notify(ResizeEventArg(*this));
}

void ColorStereoRenderer::Handle(InitializeEventArg arg) {
    stereo = new StereoCamera(*arg.canvas.GetViewingVolume());

    glGenTextures(1, &leftID);
    glGenTextures(1, &rightID);
    CHECK_FOR_GL_ERROR();
    initEvent.Notify(arg);

    glBindTexture(GL_TEXTURE_2D, leftID);
    SetupTexture(arg.canvas.GetWidth(), arg.canvas.GetHeight(), GL_LUMINANCE);
    glBindTexture(GL_TEXTURE_2D, rightID);
    SetupTexture(arg.canvas.GetWidth(), arg.canvas.GetHeight(), GL_LUMINANCE);
}

void ColorStereoRenderer::Handle(RedrawEventArg arg) {
    width = arg.canvas.GetWidth();
    height = arg.canvas.GetHeight();
    scene = arg.canvas.GetScene();
    depth = arg.canvas.GetDepth();

    // Vector<4,float> bgc = arg.canvas.GetBackgroundColor();
    glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    vv = stereo->GetLeft();
    redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));
    glBindTexture(GL_TEXTURE_2D, leftID);
    // SetupTexture(width, height, GL_LUMINANCE);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, width, height, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    vv = stereo->GetRight();
    redrawEvent.Notify(RedrawEventArg(*this, arg.start, arg.approx));
    glBindTexture(GL_TEXTURE_2D, rightID);
    // SetupTexture(width, height, GL_LUMINANCE);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, width, height, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    Vector<4,int> d(0, 0, width, height);
    glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
    OrthogonalViewingVolume volume(-1, 1, 0, width, 0, height);

    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
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
    glPushMatrix();
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
        
    bool depth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    GLboolean blending = glIsEnabled(GL_BLEND);
    GLboolean texture = glIsEnabled(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float z = 0.0;
    // glColorMask (GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);

    // glEnable(GL_BLEND);
    // glBlendColor(1.0,0.0,0.0,1.0);
    // glBlendFunc(GL_ZERO, GL_CONSTANT_COLOR);

    glBindTexture(GL_TEXTURE_2D, leftID);
    CHECK_FOR_GL_ERROR();
    glBegin(GL_QUADS);
      glColor4f(1.0,.0,0.0,.5);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(0, height, z);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(0.0, 0.0, z);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(width, 0.0, z);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(width, height, z);
    glEnd();
 

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    //glColorMask (GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, rightID);
    CHECK_FOR_GL_ERROR();
    glBegin(GL_QUADS);
      glColor4f(0.0,1.0,1.0,1.0);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(0, height, z);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(0.0, 0.0, z);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(width, 0.0, z);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(width, height, z);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    // glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
        
    if (depth)    glEnable(GL_DEPTH_TEST);
    if (lighting) glEnable(GL_LIGHTING);
    if (blending) glEnable(GL_BLEND);
    if (!texture) glDisable(GL_TEXTURE_2D);
}



void ColorStereoRenderer::Handle(DeinitializeEventArg arg) {
    deinitEvent.Notify(arg);
}

} // NS OpenGL
} // NS OpenEngine
} // NS Renderers
