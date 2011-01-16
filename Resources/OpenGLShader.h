// OpenGL Shader Language resource.
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
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
#include <Meta/OpenGL.h>
#include <Utils/DateTime.h>
#include <Utils/Timer.h>

using namespace std;

namespace OpenEngine {
    namespace Resources {
        // forward declarations
        class ITexture2D;
        typedef boost::shared_ptr<ITexture2D> ITexture2DPtr;
        class ITexture3D;
        typedef boost::shared_ptr<ITexture3D> ITexture3DPtr;

        class IDataBlock;
        typedef boost::shared_ptr<IDataBlock> IDataBlockPtr;

        namespace OpenGLShaderStructs {
            // Define the UniformKind enum
#undef GL_SHADER_SCALAR
#define GL_SHADER_SCALAR(type, extension)       \
            UNIFORM##extension, 
#undef GL_SHADER_VECTOR
#define GL_SHADER_VECTOR(params, type, extension)   \
            UNIFORM##params##extension, 
            
            enum UniformKind {
#include "UniformList.h"
                UNKNOWN };

            struct uniform{
                GLuint loc;
                UniformKind kind;
                bool bound;
                void* data;
            };
            struct matrix {
                GLuint loc;
                Matrix<4, 4, float> mat;
            };
            struct sampler2D{
                GLuint loc;
                GLint texUnit;
                ITexture2DPtr tex;
            };
            struct sampler3D{
                GLuint loc;
                GLint texUnit;
                ITexture3DPtr tex;
            };
        }
        
        using namespace OpenGLShaderStructs;

        class OpenGLShader : public IShaderResource{
        protected:
            // @TODO make static and have renderer initialize it
            static int shaderModel;
            static bool vertexSupport, geometrySupport, fragmentSupport;
            
        protected:
            string resource;
            vector<string> vertexShaders;
            vector<string> geometryShaders;
            vector<string> fragmentShaders;
            std::map<std::string, Utils::DateTime> timestamps;

            GLuint shaderProgram;
            GLint nextTexUnit;

            Utils::Timer timer;

            map<string, uniform> boundUniforms;
            map<string, uniform> unboundUniforms;

            map<string, matrix> boundMatUnis;
            map<string, matrix> unboundMatUnis;

            map<string, sampler2D> boundTex2Ds;
            map<string, sampler2D> unboundTex2Ds;

            map<string, sampler3D> boundTex3Ds;
            map<string, sampler3D> unboundTex3Ds;

            void LoadResource(string resource);
            void ResetProperties();
            void PrintShaderInfoLog(GLuint shader);
            void PrintProgramInfoLog(GLuint program);
            GLint GetUniLoc(const GLchar *name);
            void BindShaderPrograms();
            GLuint LoadShader(vector<string>, int);
            void BindUniforms();
            void BindUniform(uniform uni);
            void BindUniform(matrix mat);
            void DeleteData(uniform uni);
            void BindTextures();
            
        public:
            OpenGLShader();
            OpenGLShader(string resource);
            ~OpenGLShader();

            void Load();
            void Unload();

            void ApplyShader();
            void ReleaseShader();

            int GetUniformID(string name);
            
            void SetTexture(string name, ITexture2DPtr tex, bool force = false);
            void SetTexture(string name, ITexture3DPtr tex, bool force = false);
            void GetTexture(string name, ITexture2DPtr& tex);
            void GetTexture(string name, ITexture3DPtr& tex);
            TextureList GetTextures();

            // Uniform functions
#undef GL_SHADER_SCALAR
#define GL_SHADER_SCALAR(type, extension)                               \
            void SetUniform(string name, type value, bool force = false); \
            void GetUniform(string name, type& value);
#undef GL_SHADER_VECTOR
#define GL_SHADER_VECTOR(params, type, extension)                       \
            void SetUniform(string name, Vector<params, type> value, bool force = false); \
            void GetUniform(string name, Vector<params, type>& value);
            
#include "UniformList.h"
            void SetUniform(string name, Matrix<4, 4, float> value, bool force = false);
            void GetUniform(string name, Matrix<4, 4, float>& value);

            // Attribute functions
            void SetAttribute(string name, IDataBlockPtr values);
            bool HasAttribute(string name);

            static void ShaderSupport();

            inline int GetShaderModel() { return shaderModel; }
            inline bool HasVertexSupport() { return vertexSupport; }
            inline bool HasGeometrySupport() { return geometrySupport; }
            inline bool HasFragmentSupport() { return fragmentSupport; }

            
            // DEBUG
            void PrintUniforms();
        };

        /**
         * OpenGL shader resource plugin.
         *
         * @class GLShaderPlugin OpenGLShader.h Resources/OpenGLShader.h 
         */
        class GLShaderPlugin : public IResourcePlugin<IShaderResource> {
        public:
            GLShaderPlugin() {
                this->AddExtension("glsl");
            }
            IShaderResourcePtr CreateResource(string file) {
                return IShaderResourcePtr(new OpenGLShader(file));
            }
        };
    }
}

#endif
