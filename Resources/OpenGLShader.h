// OpenGL Shader Language resource.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_SHADER_RESOURCE_H_
#define _OPENGL_SHADER_RESOURCE_H_

#include <Resources/IShaderResource.h>
#include <Resources/IResourcePlugin.h>

using namespace std;

namespace OpenEngine {
    namespace Resources {
        // forward declarations
        class ITexture2D;
        typedef boost::shared_ptr<ITexture2D> ITexture2DPtr;

        namespace OpenGLShader {
            struct uniform;
        }
        
        using namespace OpenGLShader;

        class OpenGLShader /*: public IShaderResource */{
        protected:
            string resource;
            string vertexShader;
            //string geometryShader;
            string fragmentShader;

            map<string, uniform> boundUniforms;
            map<string, uniform> lazyUniforms;

            void LoadResource(string resource);

        public:
            OpenGLShader();
            OpenGLShader(string resource);
            //OpenGLShader(string vertex, string fragment);
            ~OpenGLShader();

            void Load();
            void Unload();

            void ApplyShader();
            void ReleaseShader();
            void SetTexture(string name, ITexture2DPtr tex);
            TextureList GetTextures() { return texs; }

            // Uniform functions
            // @TODO get functions
#undef UNIFORM1
#define UNIFORM1(type, extension)                                   \
            void SetUniform(string name, type value) = 0; 
#undef UNIFORMn
#define UNIFORMn(params, type, extension)                               \
            void SetUniform(string name, Vector<params, type> value) = 0;
            
#include "UniformList.h"

            // Attribute functions
            //@ todo add buffer object functions (uhmmmm vbo's.....)
            void SetAttribute(string name, Vector<3,float> value);
            void BindAttribute(int id, string name);
            void VertexAttribute(int id, Vector<3,float> vec);
            int GetAttributeID(const string name);
            
        };

    }
}

#endif
