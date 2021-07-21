#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_projectionView;
uniform mat4 u_transform;


void main()
{
	gl_Position = u_projectionView * u_transform * vec4(a_position, 1.0f);
}
