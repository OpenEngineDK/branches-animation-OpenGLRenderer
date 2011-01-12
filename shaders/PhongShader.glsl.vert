varying vec3 normal, lightDir, halfVec;
/* varying vec2 uv; */

void main()
{	
    gl_TexCoord[0] = gl_MultiTexCoord0;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	/* vec3 vert = (gl_ModelViewMatrix * gl_Vertex).xyz; */
	lightDir = /*vert -*/ normalize(gl_LightSource[0].position.xyz);
    halfVec = normalize(gl_LightSource[0].halfVector.xyz);
	/* eyeVec = -vert; */
	gl_Position = ftransform();		
}
