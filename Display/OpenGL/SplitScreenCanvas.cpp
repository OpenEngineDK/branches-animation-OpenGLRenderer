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
#include <Display/ICanvasBackend.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>
#include <Meta/OpenGL.h>
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Display {
namespace OpenGL {
    using Math::Matrix;
    using Math::Vector;

    SplitScreenCanvas::SplitScreenCanvas(ICanvasBackend* backend, ICanvas& first, ICanvas& second, Split split, float firstPercentage)
        : ICanvas(backend)
        , first(first) 
        , second(second)
        , init(false)
        , split(split)
        , firstPercentage(firstPercentage)
    {
    }
    
    SplitScreenCanvas::~SplitScreenCanvas() {}


    void SplitScreenCanvas::UpdateChildCanvases() {
        const unsigned int width = backend->GetTexture()->GetWidth();
        const unsigned int height = backend->GetTexture()->GetHeight();
        if (split == VERTICAL) {
            int fw = width * firstPercentage;
            first.SetPosition(pos);
            first.SetWidth(fw);
            first.SetHeight(height);
            second.SetPosition(pos + Vector<2,int>(fw,0));
            second.SetWidth(width - fw);
            second.SetHeight(height);
        }
        else {
            int fh = height * firstPercentage;
            first.SetPosition(pos);
            first.SetWidth(width);
            first.SetHeight(fh);
            second.SetPosition(pos + Vector<2,int>(0,fh));
            second.SetWidth(width);
            second.SetHeight(height - fh);
        }
    }

    void SplitScreenCanvas::Handle(Display::InitializeEventArg arg) {
        if (init) return;
        const unsigned int width = arg.canvas.GetWidth();
        const unsigned int height = arg.canvas.GetHeight();
        backend->Init(width, height);
        ((IListener<Display::InitializeEventArg>&)first).Handle(Display::InitializeEventArg(*this));
        ((IListener<Display::InitializeEventArg>&)second).Handle(Display::InitializeEventArg(*this));
        UpdateChildCanvases();
        init = true;
    }
    
    void SplitScreenCanvas::Handle(Display::DeinitializeEventArg arg) {
        if (!init) return;
        ((IListener<Display::DeinitializeEventArg>&)first).Handle(arg);
        ((IListener<Display::DeinitializeEventArg>&)second).Handle(arg);
        backend->Deinit();
        init = false;
    }

    void SplitScreenCanvas::Handle(Display::ResizeEventArg arg) {
        unsigned int width = arg.canvas.GetWidth();
        unsigned int height = arg.canvas.GetHeight();
        backend->SetDimensions(width, height);
        UpdateChildCanvases();
    }

    void SplitScreenCanvas::Handle(Display::ProcessEventArg arg) {
        ((IListener<Display::ProcessEventArg>&)first).Handle(arg);
        ((IListener<Display::ProcessEventArg>&)second).Handle(arg);

        backend->Pre();
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
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, first.GetTexture()->GetID());
        CHECK_FOR_GL_ERROR();
        const unsigned int z = 0;
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3i(0, first.GetHeight(), z);
        glTexCoord2f(0.0, 1.0);
        glVertex3i(0, 0, z);
        glTexCoord2f(1.0, 1.0);
        glVertex3i(first.GetWidth(), 0, z);
        glTexCoord2f(1.0, 0.0);
        glVertex3i(first.GetWidth(), first.GetHeight(), z);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, second.GetTexture()->GetID());
        CHECK_FOR_GL_ERROR();
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3i(first.GetWidth(), second.GetHeight(), z);
        glTexCoord2f(0.0, 1.0);
        glVertex3i(first.GetWidth(), 0, z);
        glTexCoord2f(1.0, 1.0);
        glVertex3i(GetWidth(), 0, z);
        glTexCoord2f(1.0, 0.0);
        glVertex3i(GetWidth(), GetHeight(), z);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
 
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
    
unsigned int SplitScreenCanvas::GetWidth() const {
    return backend->GetTexture()->GetWidth();
}

unsigned int SplitScreenCanvas::GetHeight() const {
    return backend->GetTexture()->GetHeight();
}
    
void SplitScreenCanvas::SetWidth(const unsigned int width) {
    backend->SetDimensions(width, backend->GetTexture()->GetHeight());
}

void SplitScreenCanvas::SetHeight(const unsigned int height) {
    backend->SetDimensions(backend->GetTexture()->GetWidth(), height);
}
    
ITexture2DPtr SplitScreenCanvas::GetTexture() {
    return backend->GetTexture();
}

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

 
