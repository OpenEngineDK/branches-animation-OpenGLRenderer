# depends on the glew library
IF(GLEW_FOUND)

SET( EXTENSION_NAME "Extensions_OpenGLRenderer")

ADD_LIBRARY( ${EXTENSION_NAME}
  Resources/GLSLResource.h
  Resources/GLSLResource.cpp
  Resources/OpenGLShader.cpp
  Renderers/OpenGL/FBOBufferedRenderer.h
  Renderers/OpenGL/FBOBufferedRenderer.cpp
  Renderers/OpenGL/GLCopyBufferedRenderer.h
  Renderers/OpenGL/GLCopyBufferedRenderer.cpp
  Renderers/OpenGL/Renderer.h
  Renderers/OpenGL/Renderer.cpp
  Renderers/OpenGL/RenderingView.h
  Renderers/OpenGL/RenderingView.cpp
  Renderers/OpenGL/TextureLoader.h
  Renderers/OpenGL/TextureLoader.cpp
  Renderers/OpenGL/ShaderLoader.h
  Renderers/OpenGL/ShaderLoader.cpp
  Renderers/OpenGL/LightRenderer.h
  Renderers/OpenGL/LightRenderer.cpp
  Scene/DisplayListNode.cpp
  Scene/DisplayListTransformer.cpp
)

TARGET_LINK_LIBRARIES( ${EXTENSION_NAME}
  OpenEngine_Core
  OpenEngine_Scene
  OpenEngine_Geometry
  OpenEngine_Renderers
  ${OPENGL_LIBRARY}
  ${GLEW_LIBRARIES}
  ${SDL_LIBRARY}
)

ENDIF(GLEW_FOUND)
