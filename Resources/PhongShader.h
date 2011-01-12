// OpenGL phong shader abstraction
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENGL_PHONG_SHADER_RESOURCE_H_
#define _OPENGL_PHONG_SHADER_RESOURCE_H_

// #include <Resources/IShaderResource.h>
#include <Resources/OpenGLShader.h>
#include <Resources/ITexture2D.h>
#include <Geometry/Material.h>

namespace OpenEngine {
namespace Resources {
        
using Geometry::MaterialPtr;

class PhongShader: public OpenGLShader {
private:
    IShaderResourcePtr shader;
    MaterialPtr mat;
    ITexture2DPtr ambient, diffuse, specular, whitetex;
public:
    PhongShader(MaterialPtr mat);
    virtual ~PhongShader();
    //void ApplyShader();

};

}
}

#endif //_OPENGL_PHONG_SHADER_RESOURCE_H_
