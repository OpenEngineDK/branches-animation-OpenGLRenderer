#define MAX_LIGHTS 2
#define NUM_LIGHTS 1

varying vec3 normal, eyeVec;
uniform sampler2D diffuseMap;
varying vec3 lightDir[MAX_LIGHTS];
void main (void)
{
  vec4 final_color =
       gl_FrontLightModelProduct.sceneColor;
  vec3 N = normalize(normal);
  int i;
  for (i=0; i<NUM_LIGHTS; ++i)
  {
    vec3 L = normalize(lightDir[i]);
    float lambertTerm = dot(N,L);
    if (lambertTerm > 0.0)
    {
      final_color +=
        gl_LightSource[i].diffuse *
          gl_FrontMaterial.diffuse *  texture2D(diffuseMap, gl_TexCoord[0].st) *
        lambertTerm;
      vec3 E = normalize(eyeVec);
      vec3 R = reflect(-L, N);
      float specular = pow(max(dot(R, E), 0.0),
                           gl_FrontMaterial.shininess);
      final_color +=
        gl_LightSource[i].specular *
        gl_FrontMaterial.specular *
        specular;
    }
  }
  gl_FragColor = final_color;
}


/* uniform int lights; */

/* varying vec3 normal, lightDir[2], halfVec[2]; */
/* varying float dist[2]; */

/* uniform sampler2D ambientMap; */
/* uniform sampler2D diffuseMap; */
/* uniform sampler2D specularMap; */

/* void main (void) */
/* { */
/*     vec4 color = gl_LightModel.ambient * gl_FrontMaterial.ambient; */
/*     vec3 n = normalize(normal); */
    
/*     for (int i = 0; i < 1; ++i) { */
/*         float att = 1.0; */
/*         /\* if (gl_LightSource[i].position.w == 1.0) {// if positional light  *\/ */
/*         /\*     att /=  *\/ */
/*         /\*         gl_LightSource[i].constantAttenuation +  *\/ */
/*         /\*         gl_LightSource[i].linearAttenuation * dist[i] +  *\/ */
/*         /\*         gl_LightSource[i].quadraticAttenuation * dist[i] * dist[i]; *\/ */
/*         /\* } *\/ */
/*         vec3 l = normalize(lightDir[i]); */
/*         vec3 h = normalize(halfVec[i]); */
        
/*         // calculate ambient */
/*         //color +=  att * gl_LightSource[i].ambient * gl_FrontMaterial.ambient;  */
        
/*         // Calculate diffuse */
/*         float ndotl = max(dot(n, l), 0.0); */
/*         if (ndotl > 0.0) {  */
/*             color += att * ndotl * texture2D(diffuseMap, gl_TexCoord[0].st);// *gl_FrontMaterial.diffuse; */
/*         } */
        
/*         // Calculate specular */
/*         float ndoth = max(dot(n,h), 0.0); */
/*         if (ndoth > 0.0) { */
/*             float specular = pow(ndoth, gl_FrontMaterial.shininess); */
/*             color += att  * gl_LightSource[i].specular * gl_FrontMaterial.specular * specular; */
/*         } */
/*     } */
/*     gl_FragColor = color; */
/* } */
