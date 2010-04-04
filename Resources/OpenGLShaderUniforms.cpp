// OpenGL Shader Uniform support.
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

#undef UNIFORM1
#define UNIFORM1(type, extension)                                       \
        void OpenGLShader::SetUniform(string name, type value, bool force){ \
            type *data = new type[1];                                   \
            data[0] = value;                                            \
            if (force){                                                 \
                map<string, uniform>::iterator bound = boundUniforms.find(name); \
                if (bound != boundUniforms.end()){                      \
                    DeleteData(bound->second);                          \
                    bound->second.data = data;                          \
                    BindUniform(bound->second);                         \
                }else{                                                  \
                    uniform uni;                                        \
                    uni.loc = GetUniLoc(name.c_str());   \
                    uni.kind = UNIFORM##extension;                      \
                    BindUniform(uni);                                   \
                    boundUniforms[name] = uni;                          \
                }                                                       \
            }else{                                                      \
                uniform uni;                                            \
                uni.loc = 0;                                            \
                uni.kind = UNIFORM##extension;                          \
                uni.data = data;                                        \
                unboundUniforms[name] = uni;                            \
            }                                                           \
        }                                                       
        
#undef UNIFORMn
#define UNIFORMn(params, type, extension)                               \
        void OpenGLShader::SetUniform(string name, Vector<params, type> value, bool force){ \
            type *data = new type[params];                              \
            value.ToArray(data);                                        \
            if (force){                                                 \
                map<string, uniform>::iterator bound = boundUniforms.find(name); \
                if (bound != boundUniforms.end()){                      \
                    DeleteData(bound->second);                          \
                    bound->second.data = data;                          \
                    BindUniform(bound->second);                         \
                }else{                                                  \
                    uniform uni;                                        \
                    uni.loc = GetUniLoc(name.c_str());                  \
                    uni.kind = UNIFORM##params##extension;              \
                    BindUniform(uni);                                   \
                    boundUniforms[name] = uni;                          \
                }                                                       \
            }else{                                                      \
                uniform uni;                                            \
                uni.loc = 0;                                            \
                uni.kind = UNIFORM##params##extension;                  \
                uni.data = data;                                        \
                unboundUniforms[name] = uni;                            \
            }                                                           \
        }
#include "UniformList.h"


        void OpenGLShader::SetUniform(string name, Matrix<4, 4, float> value, bool force){
            if (force){
                map<string, matrix>::iterator bound = boundMatUnis.find(name);
                if (bound != boundMatUnis.end()){
                    bound->second.mat = value;
                    BindUniform(bound->second);
                }else{
                    matrix mat;
                    mat.loc = GetUniLoc(name.c_str());
                    mat.mat = value;
                    BindUniform(bound->second);
                    unboundMatUnis[name] = mat;
                }
            }else{
                matrix mat;
                mat.loc = -1;
                mat.mat = value;
                unboundMatUnis[name] = mat;
            }
        }
        
#undef UNIFORM1
#define UNIFORM1(type, extension)                                       \
        void OpenGLShader::GetUniform(string name, type& value){                      \
            map<string, uniform>::iterator itr = unboundUniforms.begin(); \
            if (itr == unboundUniforms.end()){                          \
                itr = boundUniforms.begin();                            \
                if (itr == boundUniforms.end())                         \
                    throw Exception("Uniform " + name + " not found."); \
            }                                                           \
            type* v = (type*)itr->second.data;                          \
            value = *v;                                                 \
        }
        
#undef UNIFORMn
#define UNIFORMn(params, type, extension)                               \
        void OpenGLShader::GetUniform(string name, Vector<params, type>& value){      \
            map<string, uniform>::iterator itr = unboundUniforms.begin(); \
            if (itr == unboundUniforms.end()){                          \
                itr = boundUniforms.begin();                            \
                if (itr == boundUniforms.end())                         \
                    throw Exception("Uniform " + name + " not found."); \
            }                                                           \
            type* v = (type*)itr->second.data;                          \
            value = Vector<params, type>(v);                            \
        }

#include "UniformList.h"

        void OpenGLShader::GetUniform(string name, Matrix<4, 4, float>& value){
            map<string, matrix>::iterator itr = unboundMatUnis.find(name);
            if (itr == unboundMatUnis.end()){
                itr = boundMatUnis.find(name);
                if (itr == boundMatUnis.end())
                    throw Exception("Uniform " + name + " not found.");
            }
            value = itr->second.mat;
        }

        
        //  *** Private helper methods ***

        /**
         * Binds the unbound uniforms to the shader.
         *
         * Assumes the shader is already applied.
         */
        void OpenGLShader::BindUniforms(){
            {
                // Apply the unbound uniforms.
                map<string, uniform>::iterator unbound = unboundUniforms.begin();
                while (unbound != unboundUniforms.end()){
                    string name = unbound->first;
                    uniform uni = unbound->second;
                    map<string, uniform>::iterator bound = boundUniforms.find(name);
                    if (bound != boundUniforms.end()){
                        // If the uniform has already been bound, then
                        // delete the old data, copy the new and bind it.
                        DeleteData(bound->second);
                        bound->second.data = uni.data;
                        BindUniform(bound->second);
                    }else{
                        // Else get the location of the uniform, bind it
                        // and copy it to the bound array.
                        uni.loc = GetUniLoc(name.c_str());
                        BindUniform(uni);
                        boundUniforms[name] = uni;
                    }
                    ++unbound;
                }
                unboundUniforms.clear();
            }
            {
                // Apply 4x4 mats
                map<string, matrix>::iterator unbound = unboundMatUnis.begin();
                while (unbound != unboundMatUnis.end()){
                    string name = unbound->first;
                    matrix uni = unbound->second;
                    map<string, matrix>::iterator bound = boundMatUnis.find(name);
                    if (bound != boundMatUnis.end()){
                        bound->second.mat = uni.mat;
                        BindUniform(bound->second);
                    }else{
                        uni.loc = GetUniLoc(name.c_str());
                        BindUniform(uni);
                        boundMatUnis[name] = uni;
                    }
                    ++unbound;
                }
                unboundMatUnis.clear();
            }
        }
              
        /**
         * Bind the uniform to the GPU.
         */
        void OpenGLShader::BindUniform(uniform uni){
            switch(uni.kind){
                
#undef UNIFORM1
#define UNIFORM1(type, extension)                                       \
                case UNIFORM##extension :                               \
                    glUniform1##extension##v (uni.loc, 1, (const GL##type*) uni.data); \
                    break;
#undef UNIFORMn
#define UNIFORMn(params, type, extension)                               \
                case UNIFORM##params##extension :                       \
                    glUniform##params##extension##v (uni.loc, 1, (const GL##type*) uni.data); \
                    break;
#include "UniformList.h"

            default:
                throw Exception("Unsupported uniform type. How did you manage that?");
            }
        }

        void OpenGLShader::BindUniform(matrix mat){
            float data[16];
            mat.mat.ToArray(data);
            glUniformMatrix4fv(mat.loc, 1, false, data);
        }

        /**
         * Delete the uniforms data.
         */
        void OpenGLShader::DeleteData(uniform uni){
            switch(uni.kind){

#undef UNIFORM1
#define UNIFORM1(type, extension)                                    \
                case UNIFORM##extension :                            \
                    delete [] ( type* ) uni.data;                    \
                    break;
#undef UNIFORMn
#define UNIFORMn(params, type, extension)                               \
                case UNIFORM##params##extension :                       \
                    delete [] ( type* ) uni.data;                       \
                    break;
#include "UniformList.h"

            default:
                throw Exception("Unsupported uniform type. How did you manage that?");
            }
        }

    }
}
