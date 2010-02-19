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

namespace OpenEngine {
    namespace Resources {

        void OpenGLShader::SetAttribute(string name, Vector<3,float> value){
            throw Exception("Not implemented!");
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
