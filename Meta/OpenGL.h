// Meta header for OpenGL components.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENENGINE_OPENGL_H_
#define _OPENENGINE_OPENGL_H_

#include <Core/Exceptions.h>
#include <Utils/Convert.h>

using OpenEngine::Core::Exception;
using OpenEngine::Utils::Convert;

#include <GL/glew.h>

#if defined __APPLE__
  #include <GL/glew.h> //only included here for windows to supprot glMultTransposeMatrix
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>

#else

  #ifdef _WIN32
    #include <Windows.h>
  #endif

  #include <GL/gl.h>
  #include <GL/glu.h>

#endif

/**
 *  Should never be used in the code, use CHECK_FOR_GL_ERROR(); instead
 */
inline void CHECK_FOR_GL_ERROR(const std::string file, const int line) {
    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
        const GLubyte* errorString = gluErrorString(errorCode);
        throw Exception("[file:" + file +
                        " line:" + Convert::ToString(line) +
                        "] OpenGL Error: " +
                        std::string((const char*)errorString));
    }
}

/**
 *  Checks for Open GL errors and throws an exception if
 *  an error was detected, is only available in debug mode
 */
#ifdef DEBUG
#define CHECK_FOR_GL_ERROR(); CHECK_FOR_GL_ERROR(__FILE__,__LINE__);
#else
#define CHECK_FOR_GL_ERROR();
#endif

#endif // _OPENENGINE_OPENGL_H_
