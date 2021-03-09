#version 460 core

layout(location = 0) out vec4 a_color;

uniform vec3 u_lightColor;


void main()
{
	a_color = vec4(u_lightColor, 1.0f);
}