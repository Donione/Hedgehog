#version 460 core

layout(location = 0) out vec4 a_color;

in vec2 v_textureCoordinates;

uniform sampler2D u_texture;

void main()
{
	a_color = texture(u_texture, v_textureCoordinates);
}