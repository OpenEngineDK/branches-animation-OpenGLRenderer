SET( EXTENSION_NAME "Extensions_OpenGLRenderer")

SUBDIRS(Renderers/OpenGL)

ADD_LIBRARY( ${EXTENSION_NAME}
  Resources/GLSLResource.cpp
  Scene/DisplayListNode.cpp
  Scene/DisplayListTransformer.cpp
)

TARGET_LINK_LIBRARIES( ${EXTENSION_NAME}
   ${SDL_LIBRARY}
)
