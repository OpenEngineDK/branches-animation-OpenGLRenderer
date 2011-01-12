# depends on the glew library
IF(GLEW_FOUND)

SET( EXTENSION_NAME "Extensions_OpenGLRenderer")

ADD_LIBRARY( ${EXTENSION_NAME}
#  Resources/GLSLResource.h
#  Resources/GLSLResource.cpp
  Resources/OpenGLShader.cpp
  Resources/OpenGLShaderAttributes.cpp
  Resources/OpenGLShaderUniforms.cpp
  Resources/OpenGLShaderTextures.cpp
  Resources/PhongShader.h
  Resources/PhongShader.cpp
  # Renderers/OpenGL/FBOBufferedRenderer.h
  # Renderers/OpenGL/FBOBufferedRenderer.cpp
  # Renderers/OpenGL/GLCopyBufferedRenderer.h
  # Renderers/OpenGL/GLCopyBufferedRenderer.cpp
  Renderers/OpenGL/Renderer.h
  Renderers/OpenGL/Renderer.cpp
  Renderers/OpenGL/RenderingView.h
  Renderers/OpenGL/RenderingView.cpp
  Renderers/OpenGL/ShaderLoader.h
  Renderers/OpenGL/ShaderLoader.cpp
  Renderers/OpenGL/LightRenderer.h
  Renderers/OpenGL/LightRenderer.cpp
  Scene/DisplayListNode.cpp
  Scene/DisplayListTransformer.cpp
  Display/OpenGL/TextureCopy.h
  Display/OpenGL/TextureCopy.cpp
  Display/OpenGL/FrameBufferBackend.h
  Display/OpenGL/FrameBufferBackend.cpp
  Display/OpenGL/ColorStereoCanvas.h
  Display/OpenGL/ColorStereoCanvas.cpp
  Display/OpenGL/SplitStereoCanvas.h
  Display/OpenGL/SplitStereoCanvas.cpp
  Display/OpenGL/SplitScreenCanvas.h
  Display/OpenGL/SplitScreenCanvas.cpp
  Display/OpenGL/BlendCanvas.h
  Display/OpenGL/BlendCanvas.cpp
  Display/OpenGL/FadeCanvas.h
  Display/OpenGL/FadeCanvas.cpp
  # Display/OpenGL/CompositeCanvas.h
  # Display/OpenGL/CompositeCanvas.cpp
)

TARGET_LINK_LIBRARIES( ${EXTENSION_NAME}
  OpenEngine_Core
  OpenEngine_Scene
  OpenEngine_Geometry
  OpenEngine_Renderers
  OpenEngine_Display
  ${OPENGL_LIBRARY}
  ${GLEW_LIBRARIES}
  ${SDL_LIBRARY}
)

ENDIF(GLEW_FOUND)
