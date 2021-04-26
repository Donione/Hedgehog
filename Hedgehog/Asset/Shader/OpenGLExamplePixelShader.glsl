#version 460 core

layout(location = 0) out vec4 a_color;

in vec4 v_position;
in vec4 v_color;

void main()
{
	float distance = length(v_position);
	float att = 1.0f / (1.0f + distance * 0.027f + distance * distance * 0.0028f);

	a_color = v_color;
}