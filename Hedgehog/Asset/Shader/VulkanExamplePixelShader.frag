#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_color;

layout(location = 0) out vec4 a_color;

void main()
{
	float distance = length(v_position);
	float att = 1.0f / (1.0f + distance * 0.027f + distance * distance * 0.0028f);

	a_color = v_color;
}
