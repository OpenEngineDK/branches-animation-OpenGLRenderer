uniform int lights;
varying vec3 normal, lightDir[2], halfVec[2];
varying float dist[2];

void main()
{	
    gl_TexCoord[0] = gl_MultiTexCoord0;
	normal = normalize(gl_NormalMatrix * gl_Normal);

    for (int i = 0; i < lights; ++i) {
        halfVec[i]  = normalize(gl_LightSource[i].halfVector.xyz);
        if (gl_LightSource[i].position.w == 0.0) {// if directional light
            lightDir[i] = normalize(gl_LightSource[i].position.xyz);
        }
        else { // else assume positional light
            vec3 aux = (gl_ModelViewMatrix * gl_Vertex).xyz;
            aux = aux - gl_LightSource[i].position.xyz;
            lightDir[i] = normalize(aux);
            dist[i] = length(aux);
        }
    }
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
