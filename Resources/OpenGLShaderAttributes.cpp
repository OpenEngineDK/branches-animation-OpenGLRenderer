// OpenGL Shader Attribute support.
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources/OpenGLShader.h>

#include <Resources/Exceptions.h>
#include <Resources/IDataBlock.h>

#include <Logging/Logger.h>

namespace OpenEngine {
    namespace Resources {

        void OpenGLShader::SetAttribute(string name, IDataBlockPtr values){
            // @TODO Store in map instead! Does the shader remember
            // which attribs was bound to it? Then we also need to
            // bail early if values are already bound.

            logger.info << "Trying to set attribute " << name << logger.end;

            if (!HasAttribute(name))
                return;

            logger.info << "Setting attribute " << name << logger.end;

            GLint loc = glGetAttribLocation(shaderProgram, name.c_str());
            glEnableClientState(GL_VERTEX_ARRAY);
            if (values->GetID() == 0){
                // Use vertex arrays
                glVertexAttribPointer(loc, values->GetDimension(), values->GetType(), 0, 0, values->GetVoidData());
            }else{
                glBindBuffer(GL_ARRAY_BUFFER, values->GetID());
                glVertexAttribPointer(loc, values->GetDimension(), values->GetType(), 0, 0, 0);
            }
            CHECK_FOR_GL_ERROR();
        }

        bool OpenGLShader::HasAttribute(string name){
#if OE_SAFE
            if (shaderProgram == 0)
                throw ResourceException("No shader to apply. Perhaps it was not loaded.");
#endif
            GLint loc = glGetAttribLocation(shaderProgram, name.c_str());
            CHECK_FOR_GL_ERROR();
            return loc != -1;
        }

    }
}
