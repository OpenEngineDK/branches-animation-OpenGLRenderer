// OpenGL phong shader abstraction
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Resources/PhongShader.h>

#include <Resources/DirectoryManager.h>
#include <Logging/Logger.h>
#include <Resources/Texture2D.h>

namespace OpenEngine {
namespace Resources {
        
using namespace Geometry;

PhongShader::PhongShader(MaterialPtr mat)
    : OpenGLShader(DirectoryManager::FindFileInPath("extensions/OpenGLRenderer/shaders/PhongShader.glsl"))
    , mat(mat)
{
    logger.info << "ambient: " << mat->ambient << logger.end;
    logger.info << "diffuse: " << mat->diffuse << logger.end;
    logger.info << "specular: " << mat->specular << logger.end;
    unsigned char* d = new unsigned char[3];
    d[0] = 0xFF;
    d[1] = 0xFF;
    d[2] = 0xFF;
    whitetex = UCharTexture2DPtr(new Texture2D<unsigned char>(1, 1, 3, d));        
    whitetex->SetFiltering(NONE);
    whitetex->SetColorFormat(RGB);
    whitetex->SetMipmapping(false);
    whitetex->SetCompression(false);
    whitetex->SetWrapping(REPEAT);

    Vector<4,float> white(1.0);
    ambient = mat->Get2DTextures()["ambient"];
    if (!ambient) ambient = whitetex;
    else mat->ambient = white;
    SetTexture("ambientMap", ambient);
    
    diffuse = mat->Get2DTextures()["diffuse"];
    if (!diffuse) diffuse = whitetex;
    else mat->diffuse = white;
    SetTexture("diffuseMap", diffuse);

    specular = mat->Get2DTextures()["specular"];
    if (!specular) specular = whitetex;
    mat->specular = white;
    SetTexture("specularMap", specular);
}

PhongShader::~PhongShader() {

}

// void PhongShader::ApplyShader() {
//     // logger.info << "PHONG SHADER" << logger.end;
//     // OpenGLShader::ApplyShader();
// }

}
}

