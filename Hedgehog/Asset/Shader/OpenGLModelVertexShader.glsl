#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_offset;

uniform mat4 u_projectionView;
uniform mat4 u_transform;

out vec3 v_Position;
out vec3 v_Normal;

void main()
{
	vec4 position = u_transform * vec4(a_position + a_offset, 1.0f);
	
	gl_Position = u_projectionView * position;

	v_Position = vec3(position);
	v_Normal = normalize(vec3(u_transform * vec4(a_normal, 0.0)));
}
