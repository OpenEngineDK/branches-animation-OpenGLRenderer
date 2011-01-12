varying vec3 normal, lightDir, halfVec;
//varying vec2 uv;

uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

void main (void)
{
    vec3 n = normalize(normal);
    vec3 l = normalize(lightDir);
    vec3 h = normalize(halfVec);
    vec4 color = vec4(0.0);

    // calculate ambient
    color += texture2D(ambientMap, gl_TexCoord[0].st) * (gl_LightModel.ambient + gl_LightSource[0].ambient) * gl_FrontMaterial.ambient;

    // Calculate diffuse
    float ndotl = max(dot(n, l), 0.0);
    if (ndotl > 0.0) {
        color += texture2D(diffuseMap, gl_TexCoord[0].st) * gl_LightSource[0].diffuse * ndotl * gl_FrontMaterial.diffuse;
        
        // Calculate specular
        float ndoth = max(dot(n,h), 0.0);
        float specular = pow(ndoth, gl_FrontMaterial.shininess);
        color += texture2D(specularMap, gl_TexCoord[0].st) * gl_LightSource[0].specular * gl_FrontMaterial.specular * specular;
    }
    gl_FragColor = color;
}
