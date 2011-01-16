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
#include <Resources/IDataBlock.h>

namespace OpenEngine {
    namespace Resources {

        void OpenGLShader::SetAttribute(string name, Vector<3,float> value){
            throw Exception("Not implemented!");
        }

        void OpenGLShader::SetAttribute(string name, IDataBlockPtr values){
            // @TODO Store in map instead! Does the shader remember
            // which attribs was bound to it? Then we also need to
            // bail early if values are already bound.
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

        void OpenGLShader::BindAttribute(int id, string name) {
            throw Exception("Not implemented!");            
        }

        void OpenGLShader::VertexAttribute(int id, Vector<3,float> vec) {
            throw Exception("Not implemented!");
        }

        int OpenGLShader::GetAttributeID(const string name) {
            throw Exception("Not implemented!");
        }

    }
}
