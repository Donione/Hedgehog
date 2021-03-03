#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_Position;
out vec3 v_Normal;

void main()
{
	vec4 position = u_Transform * vec4(a_position, 1.0f);
	
	gl_Position = u_ViewProjection * position;

	v_Position = vec3(position);
	v_Normal = vec3(u_Transform * vec4(a_normal, 0.0));
}
