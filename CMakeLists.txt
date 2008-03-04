SUBDIRS(Renderers/OpenGL)

ADD_LIBRARY(Extensions_OpenGLRenderer
  Resources/GLSLResource.cpp
)

TARGET_LINK_LIBRARIES(Extensions_OpenGLRenderer

   ${SDL_LIBRARY}
)




