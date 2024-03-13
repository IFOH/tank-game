#version 120
attribute vec3 aVertexPosition;
attribute vec2 aVertexTexcoord;

uniform mat4x4 MVMatrix_uniform;
uniform mat4x4 ProjMatrix_uniform;

varying vec2 uv;

void main()
{
	uv = aVertexTexcoord;
	gl_Position = ProjMatrix_uniform * MVMatrix_uniform * vec4(aVertexPosition, 1.0);
}

