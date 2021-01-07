#version 460 core

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_position;
out vec4 v_color;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * a_position;

	v_position = a_position;
	v_color = a_color;
}
