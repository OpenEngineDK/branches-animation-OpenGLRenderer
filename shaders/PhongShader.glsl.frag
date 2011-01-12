uniform int lights;

varying vec3 normal, lightDir[2], halfVec[2];
varying float dist[2];

uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

void main (void)
{
    vec4 color = texture2D(ambientMap, gl_TexCoord[0].st) * gl_LightModel.ambient * gl_FrontMaterial.ambient;
    vec3 n = normalize(normal);
    
    for (int i = 0; i < lights; ++i) {
        float att = 1.0;
        if (gl_LightSource[i].position.w == 1.0) {// if positional light 
            att /= 
                gl_LightSource[i].constantAttenuation + 
                gl_LightSource[i].linearAttenuation * dist[i] + 
                gl_LightSource[i].quadraticAttenuation * dist[i] * dist[i];
        }
        vec3 l = normalize(lightDir[i]);
        vec3 h = normalize(halfVec[i]);
        
        // calculate ambient
        color +=  att * texture2D(ambientMap, gl_TexCoord[0].st) * gl_LightSource[i].ambient * gl_FrontMaterial.ambient;
        
        // Calculate diffuse
        float ndotl = max(dot(n, l), 0.0);
        /* if (ndotl > 0.0) { */
        color += att * texture2D(diffuseMap, gl_TexCoord[0].st) * gl_LightSource[i].diffuse * ndotl * gl_FrontMaterial.diffuse;
        
        // Calculate specular
        float ndoth = max(dot(n,h), 0.0);
        float specular = pow(ndoth, gl_FrontMaterial.shininess);
        color += att * texture2D(specularMap, gl_TexCoord[0].st) * gl_LightSource[i].specular * gl_FrontMaterial.specular * specular;
        /* } */
    }
    gl_FragColor = color;
}
