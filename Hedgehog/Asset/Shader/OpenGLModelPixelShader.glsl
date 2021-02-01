#version 460 core

layout(location = 0) out vec4 a_color;

in vec3 v_color;

void main()
{
	a_color = vec4(v_color, 1.0f);
}