#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_color;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_position, 1.0f);

	
	vec3 V = normalize(vec3(3.0f, -3.0f, -5.0f) - a_position);

	vec3 ambient = vec3(0.2f, 0.2f, 0.2f) * vec3(0.8f, 0.2f, 0.2f);
	ambient = vec3(0.0f, 0.0f, 0.0f);

	float diffuse = dot(normalize(a_normal), V);

	if (diffuse > 0)
	{
		ambient = ambient + diffuse * vec3(0.56f, 0.14f, 0.14f);
	}

	v_color = min(vec3(1.0f, 1.0f, 1.0f), ambient);
}
