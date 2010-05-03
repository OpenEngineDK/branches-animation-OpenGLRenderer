// OpenGL split screen canvas implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Display/OpenGL/SplitScreenCanvas.h>

#include <Display/OrthogonalViewingVolume.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <Meta/OpenGL.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {
    using Math::Matrix;
    using Math::Vector;

    SplitScreenCanvas::SplitScreenCanvas(ICanvas& first, ICanvas& second)
        : TextureCanvasBase()
        , first(first) 
        , second(second)
    {}

    SplitScreenCanvas::~SplitScreenCanvas() {}


    void SplitScreenCanvas::Handle(Display::InitializeEventArg arg) {
        unsigned int width = arg.canvas.GetWidth();
        unsigned int height = arg.canvas.GetHeight();
        unsigned int halfwidth = 0.5 * width;
        CreateTexture();
        SetTextureWidth(halfwidth);
        SetTextureHeight(height);
        ((IListener<Display::InitializeEventArg>&)first).Handle(Display::InitializeEventArg(*this));
        ((IListener<Display::InitializeEventArg>&)second).Handle(Display::InitializeEventArg(*this));
        SetTextureWidth(width);
        SetupTexture();
    }
    
    void SplitScreenCanvas::Handle(Display::DeinitializeEventArg arg) {
        ((IListener<Display::DeinitializeEventArg>&)first).Handle(arg);
        ((IListener<Display::DeinitializeEventArg>&)second).Handle(arg);
    }

    void SplitScreenCanvas::Handle(Display::ResizeEventArg arg) {
        unsigned int width = arg.canvas.GetWidth();
        unsigned int height = arg.canvas.GetHeight();
        unsigned int halfwidth = 0.5 * width;
        SetTextureWidth(width);
        SetTextureHeight(height);
        first.SetWidth(halfwidth);
        first.SetHeight(height);
        second.SetWidth(width - halfwidth);
        first.SetHeight(height);
    }

    void SplitScreenCanvas::Handle(Display::ProcessEventArg arg) {
        ((IListener<Display::ProcessEventArg>&)first).Handle(arg);
        ((IListener<Display::ProcessEventArg>&)second).Handle(arg);
        
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
        
        bool depth = glIsEnabled(GL_DEPTH_TEST);
        GLboolean lighting = glIsEnabled(GL_LIGHTING);
        GLboolean blending = glIsEnabled(GL_BLEND);
        GLboolean texture = glIsEnabled(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, first.GetTexture()->GetID());
        CHECK_FOR_GL_ERROR();
        const float z = 0.0;
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(0, first.GetHeight(), z);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(0, 0, z);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(first.GetWidth(), 0, z);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(first.GetWidth(), first.GetHeight(), z);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, second.GetTexture()->GetID());
        CHECK_FOR_GL_ERROR();
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(first.GetWidth(), second.GetHeight(), z);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(first.GetWidth(), 0, z);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(GetWidth(), 0, z);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(GetWidth(), GetHeight(), z);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
 
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
    
unsigned int SplitScreenCanvas::GetWidth() const {
    return GetTextureWidth();
}

unsigned int SplitScreenCanvas::GetHeight() const {
    return GetTextureHeight();
}
    
void SplitScreenCanvas::SetWidth(const unsigned int width) {
    SetTextureWidth(width);
}

void SplitScreenCanvas::SetHeight(const unsigned int height) {
    SetTextureHeight(height);
}
    
ITexture2DPtr SplitScreenCanvas::GetTexture() {
    return tex;
}



} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

 
