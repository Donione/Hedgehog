#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;


void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_position, 1.0f);
}
