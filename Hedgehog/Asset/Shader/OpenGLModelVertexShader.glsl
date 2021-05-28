#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_offset;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 vv_Position;
out vec3 vv_Normal;

void main()
{
	vec4 position = u_Transform * vec4(a_position, 1.0f);
	
	gl_Position = u_ViewProjection * position;

	vv_Position = vec3(position);
	vv_Normal = normalize(vec3(u_Transform * vec4(a_normal, 0.0)));
}
