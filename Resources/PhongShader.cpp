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

    PhongShader::PhongShader(MaterialPtr mat, LightRenderer& lr)
    : OpenGLShader(DirectoryManager::FindFileInPath("extensions/OpenGLRenderer/shaders/PhongShader.glsl"))
    , mat(mat)
    , lr(lr)
{
    lr.LightCountChangedEvent().Attach(*this);
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
    if (!ambient) {
        ambient = whitetex;
        logger.info << "no ambient" << logger.end;
    }
    // SetTexture("ambientMap", ambient);
    
    diffuse = mat->Get2DTextures()["diffuse"];
    if (!diffuse) {
        diffuse = whitetex;
        logger.info << "no diffuse" << logger.end;
    }
    else mat->diffuse = white;

    SetTexture("diffuseMap", diffuse);

    specular = mat->Get2DTextures()["specular"];
    if (!specular) {
        specular = whitetex;
        logger.info << "no specular" << logger.end;
    }
    // SetTexture("specularMap", specular);

    SetUniform("lights", 0);
}

PhongShader::~PhongShader() {

}

void PhongShader::Handle(LightCountChangedEventArg arg) {
    
    if (arg.count > 2) {
        SetUniform("lights", 2);
        logger.warning << "Phong shader is given " << arg.count << " lights but only 2 is supported." << logger.end;
    }
    else {
        SetUniform("lights", arg.count);
        logger.info << "Phong shader is given " << arg.count << " lights." << logger.end;
    }
}

}
}

