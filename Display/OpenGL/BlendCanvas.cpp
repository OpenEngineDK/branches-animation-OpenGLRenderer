// OpenGL based blending canvas implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Display/OpenGL/BlendCanvas.h>

#include <Display/OrthogonalViewingVolume.h>
#include <Display/ICanvasBackend.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <Meta/OpenGL.h>
#include <Logging/Logger.h>
#include <Renderers/IRenderer.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {
    using Math::Matrix;
    using Math::Vector;

BlendCanvas::BlendCanvas(ICanvasBackend* backend)
    : ICanvas(backend)
    , init(false)
    , bg(Vector<4,float>(0.0,0.0,0.0,1.0))
{
}
    
BlendCanvas::~BlendCanvas() {
}

void BlendCanvas::Handle(Display::InitializeEventArg arg) {
    if (init) return;
    list<ICanvas*>::iterator i = inits.begin();
    for (; i != inits.end(); ++i) {
        ((IListener<Display::InitializeEventArg>*)*i)->Handle(arg);
    }
    const unsigned int width = arg.canvas.GetWidth();
    const unsigned int height = arg.canvas.GetHeight();
    backend->Init(width, height);
    init = true;
}
    
void BlendCanvas::Handle(Display::DeinitializeEventArg arg) {
    if (!init) return;
    backend->Deinit();
    init = false;
}
    
void BlendCanvas::Handle(Display::ResizeEventArg arg) {
    unsigned int width = arg.canvas.GetWidth();
    unsigned int height = arg.canvas.GetHeight();
    backend->SetDimensions(width, height);
}

void BlendCanvas::Handle(Display::ProcessEventArg arg) {
    backend->Pre();
    
    glClearColor(bg[0], bg[1], bg[2], bg[3]);
    
    // Clear the screen and the depth buffer.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    Vector<4,int> d(0, 0, arg.canvas.GetWidth(), arg.canvas.GetHeight());
    glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
    OrthogonalViewingVolume volume(-1, 1, 0, arg.canvas.GetWidth(), 0, arg.canvas.GetHeight());

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
        
    GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    GLboolean blending = glIsEnabled(GL_BLEND);
    GLboolean texture = glIsEnabled(GL_TEXTURE_2D);
    GLint texenv;
    glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenv);
    // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
    list<Element>::iterator itr = elements.begin();
    for (; itr != elements.end(); ++itr) {
        Element e = *itr;
        int x = e.x;
        int y = e.y;
        const unsigned int z = 0;
        ITexture2DPtr tex = e.tex;
        int w = tex->GetWidth();
        int h = tex->GetHeight();
            
        glBindTexture(GL_TEXTURE_2D, tex->GetID());
        CHECK_FOR_GL_ERROR();
        glColor4f(e.color[0],e.color[1], e.color[2], e.color[3]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3i(x, y + h, z);
        glTexCoord2f(0.0, 1.0);
        glVertex3i(x, y, z);
        glTexCoord2f(1.0, 1.0);
        glVertex3i(x + w, y, z);
        glTexCoord2f(1.0, 0.0);
        glVertex3i(x + w, y + h, z);
        glEnd();
    }

    glDisable(GL_BLEND);
 
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    CHECK_FOR_GL_ERROR();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenv);
    if (depth)    glEnable(GL_DEPTH_TEST);
    if (lighting) glEnable(GL_LIGHTING);
    if (blending) glEnable(GL_BLEND);
    if (!texture) glDisable(GL_TEXTURE_2D);

    backend->Post();
}
    
unsigned int BlendCanvas::GetWidth() const {
    return backend->GetTexture()->GetWidth();
}

unsigned int BlendCanvas::GetHeight() const {
    return backend->GetTexture()->GetHeight();
}
    
void BlendCanvas::SetWidth(const unsigned int width) {
    backend->SetDimensions(width, backend->GetTexture()->GetHeight());
}

void BlendCanvas::SetHeight(const unsigned int height) {
    backend->SetDimensions(backend->GetTexture()->GetWidth(), height);
}
    
ITexture2DPtr BlendCanvas::GetTexture() {
    return backend->GetTexture();
}

void BlendCanvas::AddTexture(ITexture2DPtr tex, int x, int y, Vector<4,float> color) {
    Element e;
    e.tex = tex;
    e.x = x;
    e.y = y;
    e.color = color;
    elements.push_back(e);
}

void BlendCanvas::SetBackground(Vector<4,float> bg) {
    this->bg = bg;
}

void BlendCanvas::Clear() {
    elements.clear();
}

void BlendCanvas::InitCanvas(ICanvas* canvas) {
    inits.push_back(canvas);
}


} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

 
