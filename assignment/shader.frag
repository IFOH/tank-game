#version 120

uniform sampler2D TextureMap_uniform;

varying vec2 uv;

void main()
{
	gl_FragColor = texture2D(TextureMap_uniform, uv);
}
