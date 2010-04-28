// OpenGL Shader Texture Support.
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources/OpenGLShader.h>

#include <Resources/ITexture2D.h>
#include <Resources/ITexture3D.h>

namespace OpenEngine {
    namespace Resources {

        void OpenGLShader::SetTexture(string name, ITexture2DPtr tex, bool force){
            sampler2D sam;
            sam.tex = tex;
            if (force){
                map<string, sampler2D>::iterator bound = boundTex2Ds.find(name);
                if (bound != boundTex2Ds.end()){ 
                    // sampler already bound, replace it's texture
                    bound->second.tex = sam.tex;
                }else{
                    // Set the samplers values and add it to the bound
                    // map.
                    sam.loc = GetUniLoc(name.c_str());
                    sam.texUnit = nextTexUnit++;
                    glUniform1i(sam.loc, sam.texUnit);
                    boundTex2Ds[name] = sam;
                }
            }else{
                sam.loc = 0;
                sam.texUnit = 0;
                unboundTex2Ds[name] = sam;
            }
        }

        void OpenGLShader::SetTexture(string name, ITexture3DPtr tex, bool force){
            sampler3D sam;
            sam.tex = tex;
            if (force){
                map<string, sampler3D>::iterator bound = boundTex3Ds.find(name);
                if (bound != boundTex3Ds.end()){ 
                    // sampler already bound, replace it's texture
                    bound->second.tex = sam.tex;
                }else{
                    // Set the samplers values and add it to the bound
                    // map.
                    sam.loc = GetUniLoc(name.c_str());
                    sam.texUnit = nextTexUnit++;
                    glUniform1i(sam.loc, sam.texUnit);
                    boundTex3Ds[name] = sam;
                }
            }else{
                sam.loc = 0;
                sam.texUnit = 0;
                unboundTex3Ds[name] = sam;
            }
        }

        void OpenGLShader::GetTexture(string name, ITexture2DPtr& tex){
            map<string, sampler2D>::iterator itr = unboundTex2Ds.begin();
            if (itr == unboundTex2Ds.end()){
                itr = boundTex2Ds.begin();
#ifdef OE_SAFE
                if (itr == boundTex2Ds.end())
                    throw Exception("2D texture " + name + " not found");
#endif
            }
            tex = itr->second.tex;
        }

        void OpenGLShader::GetTexture(string name, ITexture3DPtr& tex){
            map<string, sampler3D>::iterator itr = unboundTex3Ds.begin();
            if (itr == unboundTex3Ds.end()){
                itr = boundTex3Ds.begin();
#ifdef OE_SAFE
                if (itr == boundTex3Ds.end())
                    throw Exception("3D texture " + name + " not found");
#endif
            }
            tex = itr->second.tex;
        }

        TextureList OpenGLShader::GetTextures(){
            TextureList list = TextureList();
            map<string, sampler2D>::iterator itr = boundTex2Ds.begin();
            while(itr != boundTex2Ds.end()){
                list.push_back(itr->second.tex);
                itr++;
            }

            itr = unboundTex2Ds.begin();
            while(itr != unboundTex2Ds.end()){
                list.push_back(itr->second.tex);
                itr++;
            }

            return list;
        }

        /**
         * Binds the unbound textures to the shader. Then bind them to the gpu.
         *
         * Assumes the shader is already applied.
         */
        void OpenGLShader::BindTextures(){

            // Apply the unbound textures.
            map<string, sampler2D>::iterator unbound = unboundTex2Ds.begin();
            while(unbound != unboundTex2Ds.end()){
                // Check if the texture already exists
                string name = unbound->first;
                sampler2D sam = unbound->second;
                map<string, sampler2D>::iterator bound = boundTex2Ds.find(name);
                if (bound != boundTex2Ds.end()){ 
                    // sampler already bound, replace it's texture
                    bound->second.tex = sam.tex;
                }else{
                    // Set the samplers values and add it to the bound
                    // map.
                    sam.loc = GetUniLoc(name.c_str());
                    sam.texUnit = nextTexUnit++;
                    glUniform1i(sam.loc, sam.texUnit);
                    boundTex2Ds[name] = sam;
                }
                unbound++;
            }
            unboundTex2Ds.clear();

            map<string, sampler3D>::iterator lazy = unboundTex3Ds.begin();
            while(lazy != unboundTex3Ds.end()){
                // Check if the texture already exists
                string name = lazy->first;
                sampler3D sam = lazy->second;
                map<string, sampler3D>::iterator bound = boundTex3Ds.find(name);
                if (bound != boundTex3Ds.end()){ 
                    // sampler already bound, replace it's texture
                    bound->second.tex = sam.tex;
                }else{
                    // Set the samplers values and add it to the bound
                    // map.
                    sam.loc = GetUniLoc(name.c_str());
                    sam.texUnit = nextTexUnit++;
                    glUniform1i(sam.loc, sam.texUnit);
                    boundTex3Ds[name] = sam;
                }
                lazy++;
            }
            unboundTex3Ds.clear();

            // Bind all the textures
            map<string, sampler2D>::iterator itr2 = boundTex2Ds.begin();
            while(itr2 != boundTex2Ds.end()){
                glActiveTexture(GL_TEXTURE0 + itr2->second.texUnit);
                glBindTexture(GL_TEXTURE_2D, itr2->second.tex->GetID());
                itr2++;
            }
            map<string, sampler3D>::iterator itr3 = boundTex3Ds.begin();
            while(itr3 != boundTex3Ds.end()){
                glActiveTexture(GL_TEXTURE0 + itr3->second.texUnit);
                glBindTexture(itr3->second.tex->GetUseCase(), itr3->second.tex->GetID());
                itr3++;
            }

            // reset the active texture
            glActiveTexture(GL_TEXTURE0);
        }

    }
}
