#version 460 core

struct PointLight
{
    vec3 color;
    vec3 position;
    vec3 attenuation;// x = constant, y = linear, z = quadratic components
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_textureCoordinates;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec3 a_bitangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

uniform vec3 u_viewPos;
uniform PointLight u_pointLight[3];

out vec3 v_Position;
out vec2 v_textureCoordinates;
out mat3 TBN;
out vec3 v_positionTan;
out vec3 v_lightPosTan[3];
out vec3 v_viewPosTan;
out vec3 v_normalTan;

void main()
{
	vec4 position = u_Transform * vec4(a_position, 1.0f);

	gl_Position = u_ViewProjection * position;
	
	v_Position = vec3(position);
	v_textureCoordinates = a_textureCoordinates;


	vec3 T = normalize(vec3(u_Transform * vec4(a_tangent,   0.0)));
	vec3 B = normalize(vec3(u_Transform * vec4(a_bitangent, 0.0)));
	vec3 N = normalize(vec3(u_Transform * vec4(a_normal,    0.0)));
	
	// pass the TBN matrix to pixel shader to transform normal samples to world space
	TBN = mat3(T, B, N);

	// or invert it before passing to insted transform light vectors into tangent space
	TBN = transpose(TBN);

	// or transform all the relevant light vectors to tangent space here and pass those
	v_positionTan = TBN * v_Position;
	for (int i = 0; i < 3; i++)
	{
		v_lightPosTan[i] = TBN * u_pointLight[i].position;
	}
	v_viewPosTan = TBN * u_viewPos;
	v_normalTan = TBN * vec3(u_Transform * vec4(a_normal, 0.0f));
}
