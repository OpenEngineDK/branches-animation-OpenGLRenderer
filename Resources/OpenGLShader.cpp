// OpenGL Shader Language resource.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources/OpenGLShader.h>

#include <Resources/ITexture2D.h>

namespace OpenEngine {
    namespace Resources {

        namespace OpenGLShader {
            struct uniform {
                unsigned int id;
                void* data;
                // and type enum
            };
            
        }
        
    }
}
