// OpenGL Shader Language resource.
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// Modified by Anders Bach Nielsen <abachn@daimi.au.dk> - 21. Nov 2007
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#define printinfo true

#include <Resources/OpenGLShader.h>

#include <Logging/Logger.h>
#include <Meta/OpenGL.h>
#include <Resources/Exceptions.h>
#include <Resources/File.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>
#include <Resources/ITexture3D.h>

namespace OpenEngine {
    namespace Resources {

        OpenGLShader::OpenGLShader() {
            resource.clear();
            nextTexUnit = 0;
            shaderProgram = 0;
        }

        OpenGLShader::OpenGLShader(string filename)
            : resource(filename) {
            nextTexUnit = 0;
            shaderProgram = 0;
        }

        OpenGLShader::~OpenGLShader() {
            // Delete the uniforms data.
            map<string, uniform>::iterator itr = boundUniforms.begin();
            while(itr != boundUniforms.end()){
                DeleteData(itr->second);
                ++itr;
            }
            itr = unboundUniforms.begin();
            while(itr != unboundUniforms.end()){
                DeleteData(itr->second);
                ++itr;
            }
        }
        
        void OpenGLShader::Load() {
            // Load the resource and its attributes from a file, if a
            // file is available.
            if (!resource.empty()){
                LoadResource(resource);
            }

#ifdef OE_SAFE
            if (vertexShaders.empty() && 
                geometryShaders.empty() && 
                fragmentShaders.empty())
                throw ResourceException("No shaders specified");
#endif
      
            // Load the shader onto the gpu.
            BindShaderPrograms();
        }
        
        void OpenGLShader::Unload() {
            glDeleteShader(shaderProgram);
            shaderProgram = 0;
        }

        void OpenGLShader::ApplyShader(){
#ifdef OE_SAFE
            if (shaderProgram == 0)
                throw ResourceException("No shader to apply. Perhaps loading failed.");
#endif

            // Bind the shader program.
            glUseProgram(shaderProgram);

            BindUniforms();
            BindTextures();
        }

        void OpenGLShader::ReleaseShader(){
            glUseProgram(0);
        }

        unsigned int OpenGLShader::GetUniformID(string name){            
            return GetUniLoc(name.c_str());
        }

        //  *** Private helper methods ***
        
        /**
         * Resets the uniforms. This means preparing the known
         * uniforms to be bound again.
         */
        void OpenGLShader::ResetProperties(){
            // Move bound uniforms to unbound, to preserve attributes not
            // specified in the glsl file.
            boundUniforms.insert(unboundUniforms.begin(), unboundUniforms.end());
            unboundUniforms = map<string, uniform>(boundUniforms);
            boundTex2Ds.insert(unboundTex2Ds.begin(), unboundTex2Ds.end());
            unboundTex2Ds = map<string, sampler2D>(boundTex2Ds);
            boundTex3Ds.insert(unboundTex3Ds.begin(), unboundTex3Ds.end());
            unboundTex3Ds = map<string, sampler3D>(boundTex3Ds);
            boundUniforms.clear();
            boundTex2Ds.clear();
            boundTex3Ds.clear();

            // Set all their loc's to 0 since we no longer know where they are.
            map<string, uniform>::iterator itr = unboundUniforms.begin();
            while (itr != unboundUniforms.end()){
                itr->second.loc = 0;
                itr++;
            }
            map<string, sampler2D>::iterator itr2 = unboundTex2Ds.begin();
            while (itr2 != unboundTex2Ds.end()){
                itr2->second.loc = 0;
                itr2++;
            }
            map<string, sampler3D>::iterator itr3 = unboundTex3Ds.begin();
            while (itr3 != unboundTex3Ds.end()){
                itr3->second.loc = 0;
                itr3++;
            }
        }

        void OpenGLShader::LoadResource(string resource){
            ResetProperties();

            // Load the file.
            ifstream* in = File::Open(resource);
            
            char buf[255], file[255];
            int line = 0;
            while (!in->eof()){
                ++line;
                in->getline(buf, 255);
                string type = string(buf,5);

                // Empty lines and comments can be ignored.
                if (type.empty() ||
                    buf[0] == '#')
                    continue;

                if (type == "vert:") {
                    if (sscanf(buf, "vert: %s", file) == 1)
                        vertexShaders.push_back(file);
                    else
                        logger.warning << "Line("<<line<<") Invalid vertex shader." << logger.end;
                }else if (type == "geom:") {
                    if (sscanf(buf, "geom: %s", file) == 1)
                        geometryShaders.push_back(file);
                    else
                        logger.warning << "Line("<<line<<") Invalid geometry shader." << logger.end;
                }else if (type == "frag:") {
                    if (sscanf(buf, "frag: %s", file) == 1)
                        fragmentShaders.push_back(file);
                    else
                        logger.warning << "Line("<<line<<") Invalid fragment shader." << logger.end;
                }else if (type == "text:" || type == "tex2D" || type == "tex3D") {
                    const int maxlength = 300;
                    char fileandname[maxlength];
                    if (sscanf(buf, "text: %s", fileandname) == 1 ||
                        sscanf(buf, "tex2D: %s", fileandname) == 1 ||
                        sscanf(buf, "tex3D: %s", fileandname) == 1) {
                        int seperator=0;
                        for(int i=0;i<maxlength;i++) {
                            if(fileandname[i]=='|')
                                seperator=i;
                            if(fileandname[i]=='\0')
                                break;
                        }
#ifdef OE_SAFE
                        if(seperator==0)
                            throw Exception("no separetor(|) between texture name and file");
#endif
                        string texname = string(fileandname,seperator);
                        string texfile = string(fileandname+seperator+1);
                        if (type == "text:" || type == "tex2D") {
                            ITexture2DPtr t = ResourceManager<ITexture2D>::Create(texfile);
                            SetTexture(texname, t);
                        }else{
                            ITexture3DPtr t = ResourceManager<ITexture3D>::Create(texfile);
                            SetTexture(texname, t);
                        }
                    }
#ifdef OE_SAFE
                    else{
                        logger.error << "Line("<<line<<") Invalid texture resource: '" << file << "'" << logger.end;
                        throw Exception("Invalid texture resource");
                    }
#endif
                }else if (type == "attr:" || type == "unif:") {
                    char name[255];
                    float attr[4];
                    int n = sscanf(buf, "attr: %s = %f %f %f %f", name, &attr[0], &attr[1], &attr[2], &attr[3]) - 1;
                    switch(n){
                    case 1:
                        SetUniform(string(name), attr[0]);
                        break;
                    case 2:
                        SetUniform(string(name), Vector<2, float>(attr[0], attr[1]));
                        break;
                    case 3:
                        SetUniform(string(name), Vector<3, float>(attr[0], attr[1], attr[2]));
                        break;
                    case 4:
                        SetUniform(string(name), Vector<4, float>(attr[0], attr[1], attr[2], attr[3]));
                        break;
                    }
                }
            }
        
            in->close();
            delete in;
        }     

        void OpenGLShader::PrintShaderInfoLog(GLuint shader){
#ifdef DEBUG
            GLint infologLength = 0, charsWritten = 0;
            GLchar *infoLog;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
            if (infologLength > 0) {
                infoLog = (GLchar *)malloc(infologLength);
                if (infoLog==NULL) {
                    throw Exception("Could not allocate InfoLog buffer");
                }
                glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
                logger.info << "Shader InfoLog:\n \"" << infoLog << "\"" << logger.end;
                free(infoLog);
            }
#endif
        }
        
        void OpenGLShader::PrintProgramInfoLog(GLuint program){
#ifdef DEBUG
            GLint infologLength = 0, charsWritten = 0;
            GLchar* infoLog;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
            if (infologLength > 0) {
                infoLog = (GLchar *)malloc(infologLength);
                if (infoLog==NULL) {
                    logger.error << "Could not allocate InfoLog buffer" << logger.end;
                    return;
                }
                glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
                logger.info << "Program InfoLog:\n \"" << infoLog << "\"" << logger.end;
                free(infoLog);
            }
#endif
        }
        
        GLint OpenGLShader::GetUniLoc(const GLchar *name){
            GLint loc = glGetUniformLocation(shaderProgram, name);
#ifdef OE_SAFE
            if (loc == -1)
                throw Exception( string("No such uniform named \"") + name + "\"");
#endif
            return loc;
        }
        
        void OpenGLShader::BindShaderPrograms(){
            shaderProgram = glCreateProgram();

            // attach vertex shader
            if (!vertexShaders.empty()){
                GLuint shader = LoadShader(vertexShaders, GL_VERTEX_SHADER);
#ifdef OE_SAFE
                if (shader == 0)
                    throw Exception("Failed loading vertexshader");
#endif
                glAttachShader(shaderProgram, shader);
            }

            /*        
            // attach geometry shader
            if (!geometryShaders.empty()){
                GLuint shader = LoadShader(geometryShaders, GEOMETRY_SHADER_ARB);
#ifdef OE_SAFE
                if (shader == 0)
                    throw Exception("Failed loading geometryshader");
#endif
                glAttachShader(shaderProgram, shader);
            }
            */

            // attach fragment shader
            if (!fragmentShaders.empty()){
                GLuint shader = LoadShader(fragmentShaders, GL_FRAGMENT_SHADER);
#ifdef OE_SAFE
                if (shader == 0)
                    throw Exception("Failed loading fragmentshader");
#endif
                glAttachShader(shaderProgram, shader);
            }
            
            // Link the program object and print out the info log
            glLinkProgram(shaderProgram);
            GLint linked;
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
            
            CHECK_FOR_GL_ERROR();
            PrintProgramInfoLog(shaderProgram);
#ifdef OE_SAFE            
            if(linked == 0)
                throw Exception("Could not link shader program");
#endif
            
        }

        /**
         * Loads the given shader. OpenGL 2.0 and above.
         */        
        GLuint OpenGLShader::LoadShader(vector<string> files, int type){
            GLuint shader = glCreateShader(type);
            
            unsigned int size = files.size();
            const GLchar* shaderBits[size];

            // Read all the shaders from disk
            for (unsigned int i = 0; i < size; ++i){
                if (printinfo)
                    logger.info << "Loading shader: " << files[i] << logger.end;
                shaderBits[i] = File::ReadShader<GLchar>(DirectoryManager::FindFileInPath(files[i]));
                if (shaderBits[i] == NULL) return 0;
            }

            glShaderSource(shader, size, shaderBits, NULL);

            // Compile shader
            glCompileShader(shader);
            GLint  compiled;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
#ifdef OE_SAFE
            if (compiled==0) {
                logger.error << "Failed compiling shader program consisting of: " << logger.end;
                for (unsigned int i = 0; i< size; ++i)
                    logger.error << files[i] << logger.end;
                GLsizei bufsize;
                const int maxBufSize = 100;
                char buffer[maxBufSize];
                glGetShaderInfoLog(shader, maxBufSize, &bufsize, buffer);
                logger.error << "compile errors: " << buffer << logger.end;
                throw Exception("Failed compiling shader program.");
            }
#endif
            
            PrintShaderInfoLog(shader);
            return shader;
        }
                
    }
}
