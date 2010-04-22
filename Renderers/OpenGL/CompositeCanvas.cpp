// OpenGL split screen renderer implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#include <Renderers/OpenGL/CompositeCanvas.h>

#include <Display/OrthogonalViewingVolume.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>

#include <Meta/OpenGL.h>

#include <Logging/Logger.h>

namespace OpenEngine {
namespace Renderers {
namespace OpenGL {

    using Display::OrthogonalViewingVolume;
    using Math::Matrix;
    using Math::Vector;

    CompositeCanvas::CompositeCanvas()
        : init(false)
        , maxframe(NULL)
        , animate(false)
        , updateFrames(false)
        , cols(2)
        , elapsedTime(.0)
        , maxTime(0.4) {}

    CompositeCanvas::~CompositeCanvas() {}

    void CompositeCanvas::SetColumns(unsigned int cols) {
        this->cols = cols;
        updateFrames = true;
    }

    unsigned int CompositeCanvas::GetColumns() {
        return cols;
    }

    void CompositeCanvas::UpdateFrames(ICanvas& frame) {
        unsigned int rows = (frames.size() / cols) + (frames.size() % cols == 0 ? 0 : 1);
        unsigned int width = frame.GetWidth();
        unsigned int h = frame.GetHeight() / rows;
        vector<Layout*>::iterator li = frames.begin();
        for (unsigned int i = 0; i < rows; ++i) {
            unsigned int c = (frames.size() - i * cols);
            c = c < cols ? c : cols;
            unsigned int w = width / c;
            for (unsigned int j = 0; j < c; ++j) {
                Layout* l = *(li++);
                l->canvas->SetWidth(w);
                l->canvas->SetHeight(h);
                l->x = j * w;
                l->y = i * h;
            }
        }
        updateFrames = false;
    }

  void CompositeCanvas::Handle(Display::InitializeEventArg arg) {
      UpdateFrames(arg.canvas);
      set<ICanvas*>::iterator i = processSet.begin();
        for(; i != processSet.end(); ++i) {
            ((IListener<Display::InitializeEventArg>*)*i)->Handle(arg);
        }
      init = true;
    }

    void CompositeCanvas::Handle(Display::DeinitializeEventArg arg) {
        set<ICanvas*>::iterator i = processSet.begin();
        for(; i != processSet.end(); ++i) {
            ((IListener<Display::DeinitializeEventArg>*)*i)->Handle(arg);
        }
        init = false;
    }

    void CompositeCanvas::Handle(ResizeEventArg arg) {
        if (maxframe != NULL) {
            unsigned int i = maxid;
            ToggleFrame(i);
            UpdateFrames(arg.canvas);
            ToggleFrame(i);
            maxframe->canvas->SetWidth(arg.canvas.GetWidth());
            maxframe->canvas->SetHeight(arg.canvas.GetHeight());
        }
        else {
            UpdateFrames(arg.canvas);
        }
    }

    void CompositeCanvas::Handle(RedrawEventArg arg) {
        logger.info << "compositedraw: " << logger.end;
        if (updateFrames) UpdateFrames(arg.canvas);
        if (animate) {
            elapsedTime += float(arg.approx) / 1000000.0;
            float delta;
            if (elapsedTime >= maxTime) {
                delta = 1.0;
                animate = false;
            }
            else {
                delta = elapsedTime / maxTime;
            }
            if (maxframe != NULL) {
                maxframe->x = origX - (origX * delta);
                maxframe->y = origY - (origY * delta);
                maxframe->canvas->SetWidth(arg.canvas.GetWidth() - (1-delta) * (arg.canvas.GetWidth() - origWidth));
                maxframe->canvas->SetHeight(arg.canvas.GetHeight() - (1-delta) * (arg.canvas.GetHeight() - origHeight));
            }
        }
        
        set<ICanvas*>::iterator itr = processSet.begin();
        for(; itr != processSet.end(); ++itr) {
            ((IListener<RedrawEventArg>*)*itr)->Handle(arg);
        }
        
        
        Vector<4,int> d(0, 0, arg.canvas.GetWidth(), arg.canvas.GetHeight());
        glViewport((GLsizei)d[0], (GLsizei)d[1], (GLsizei)d[2], (GLsizei)d[3]);
        OrthogonalViewingVolume volume(-1, 1, 0, arg.canvas.GetWidth(), 0, arg.canvas.GetHeight());

        Vector<4,float> bgc(1.0, 1.0, 1.0, 1.0);
        glClearColor(bgc[0], bgc[1], bgc[2], bgc[3]);

        // Clear the OpenGL frame buffer.
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
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

        vector<Layout*>::iterator i = layout.begin();
        for(; i != layout.end(); ++i) {
            Layout* l = *i;
            glBindTexture(GL_TEXTURE_2D, l->canvas->GetTexture()->GetID());
            CHECK_FOR_GL_ERROR();
            const float z = 0.0;
            glBegin(GL_QUADS);
            
            glTexCoord2f(0.0, 0.0);
            glVertex3f(l->x, l->y, z);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(l->x, l->y + l->canvas->GetHeight(), z);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(l->x + l->canvas->GetWidth(), l->y + l->canvas->GetHeight(), z);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(l->x + l->canvas->GetWidth(), l->y, z);
            glEnd();
        }
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

    void CompositeCanvas::AddCanvas(TextureCanvas* canvas) {
        Layout* l = new Layout(canvas, 0, 0);
        layout.push_back(l);
        frames.push_back(l);
        processSet.insert(canvas);
    }

    void CompositeCanvas::MoveToFront(Layout* l) {
        vector<Layout*>::iterator itr = layout.begin();
        for (; itr != layout.end(); ++itr) {
            if (*itr == l) {
                layout.erase(itr);  
                layout.push_back(l);
            }
        }
    }

    void CompositeCanvas::Animate() {
        animate = true;
        elapsedTime = 0;
   }

    void CompositeCanvas::ToggleFrame(unsigned int frame) {
        Layout* l = frames[frame];
        if (maxframe != NULL) {
            maxframe->x = origX;
            maxframe->y = origY;
            maxframe->canvas->SetWidth(origWidth);
            maxframe->canvas->SetHeight(origHeight);
        }
        if (maxframe != l) {
            maxframe = l;
            maxid = frame;
            origX = maxframe->x;
            origY = maxframe->y;
            origWidth = maxframe->canvas->GetWidth();
            origHeight = maxframe->canvas->GetHeight();
            MoveToFront(l);            
        }
        else {
            maxframe = NULL;
        }
    }

    void CompositeCanvas::ToggleMaximize(unsigned int frame) {
        #ifdef OE_SAFE
        if (frame >= frames.size()) {
            logger.info << "Invalid frame number: " << frame << logger.end;
            return;
        }
        #endif
        ToggleFrame(frame);
        Animate();
    }

} // NS OpenGL
} // NS Renderers
} // NS OpenEngine

 
