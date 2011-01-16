varying vec3 normal, eyeVec;
#define MAX_LIGHTS 2
#define NUM_LIGHTS 1
varying vec3 lightDir[MAX_LIGHTS];
void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0; 
    gl_Position = ftransform();
    normal = gl_NormalMatrix * gl_Normal;
    vec4 vVertex = gl_ModelViewMatrix * gl_Vertex;
    eyeVec = -vVertex.xyz;
    int i;
    for (i=0; i<NUM_LIGHTS; ++i)
        lightDir[i] =
      vec3(gl_LightSource[i].position.xyz - vVertex.xyz);
}
/* uniform int lights; */
/* varying vec3 normal, lightDir[2], halfVec[2]; */
/* varying float dist[2]; */

/* void main() */
/* {	 */
/*     gl_TexCoord[0] = gl_MultiTexCoord0; */
/* 	normal = normalize(gl_NormalMatrix * gl_Normal); */

/*     for (int i = 0; i < 1; ++i) { */
/*         halfVec[i]  = normalize(gl_LightSource[i].halfVector.xyz); */
/*         if (gl_LightSource[i].position.w == 0.0) {// if directional light */
/*             /\* lightDir[i] = normalize(gl_LightSource[i].position.xyz); *\/ */
/*         } */
/*         else { // else assume positional light */
/*             vec3 aux = (gl_ModelViewMatrix * gl_Vertex).xyz; */
/*             aux = gl_LightSource[i].position.xyz - aux; */
/*             lightDir[i] = normalize(aux); */
/*             dist[i] = length(aux); */
/*         } */
/*     } */
/* 	gl_Position = ftransform(); */
/* } */

