#version 460 core

layout(location = 0) out vec4 a_color;

in vec3 v_position;
in vec4 v_color;

void main()
{
	a_color = v_color;
}