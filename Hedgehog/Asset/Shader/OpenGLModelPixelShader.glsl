#version 460 core

layout(location = 0) out vec4 a_color;

in vec3 v_Position;
in vec3 v_Normal;

uniform vec3 u_viewPos;

void main()
{
	vec3 lightColor = vec3(0.56f, 0.14f, 0.14f);
	vec3 lightPosition = vec3(3.0f, 3.0f, 5.0f);

	vec3 ambient = vec3(0.0f, 0.0f, 0.0f);

	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(lightPosition - v_Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	float specularStrength = 0.2;
	vec3 viewDir = normalize(u_viewPos - v_Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	a_color = vec4(ambient + diffuse + specular, 1.0f);
}