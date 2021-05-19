#version 460 core

struct PointLight
{
    vec3 color;
    vec3 position;
    vec3 attenuation;// x = constant, y = linear, z = quadratic components
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in float a_textureSlot;
layout(location = 2) in vec2 a_textureCoordinates;
layout(location = 3) in vec3 a_normal;
layout(location = 4) in vec3 a_tangent;
layout(location = 5) in vec3 a_bitangent;
layout(location = 6) in vec4 a_segmentIDs;
layout(location = 7) in vec4 a_segmentWeigths;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_segmentTransforms[65];

uniform vec3 u_viewPos;
uniform PointLight u_pointLight[3];

out vec3 v_Position;
flat out int v_texSlot;
out vec2 v_textureCoordinates;
out mat3 v_TBN;
out vec3 v_positionTan;
out vec3 v_lightPosTan[3];
out vec3 v_viewPosTan;
out vec3 v_normalTan;

void main()
{
	vec3 finalPosition = vec3(0.0f);
	vec3 finalNormal = vec3(0.0f);
	vec3 finalTangent = vec3(0.0f);
	//vec3 finalBitangent = vec3(0.0f);

	for (int i = 0; i < 4; i++)
	{
		if (a_segmentIDs[i] == -1.0f)
		{
			continue;
		}

		vec4 segmentPosition = u_segmentTransforms[int(a_segmentIDs[i])] * vec4(a_position, 1.0f);
		finalPosition += segmentPosition.xyz * a_segmentWeigths[i];

		vec3 segmentNormal = mat3(u_segmentTransforms[int(a_segmentIDs[i])]) * a_normal;
		finalNormal += segmentNormal * a_segmentWeigths[i];

		vec3 segmentTangent = mat3(u_segmentTransforms[int(a_segmentIDs[i])]) * a_tangent;
		finalTangent += segmentTangent.xyz * a_segmentWeigths[i];

		//vec3 segmentBitangent = mat3(u_segmentTransforms[int(a_segmentIDs[i])]) * a_bitangent;
		//finalBitangent += segmentBitangent * a_segmentWeigths[i];
	}

	vec4 position = u_Transform * vec4(finalPosition, 1.0f);

	gl_Position = u_ViewProjection * position;
	
	v_Position = vec3(position);
	v_texSlot = int(a_textureSlot);
	v_textureCoordinates = a_textureCoordinates;

	vec3 T = normalize(vec3(u_Transform * vec4(finalTangent, 0.0)));
	//vec3 B = normalize(vec3(u_Transform * vec4(finalBitangent, 0.0)));
	vec3 N = normalize(vec3(u_Transform * vec4(finalNormal, 0.0)));

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);
	
	// pass the TBN matrix to pixel shader to transform normal samples to world space
	v_TBN = mat3(T, B, N);

	// or invert it before passing to insted transform light vectors into tangent space
	v_TBN = transpose(v_TBN);

	// or transform all the relevant light vectors to tangent space here and pass those
	v_positionTan = v_TBN * v_Position;
	for (int i = 0; i < 3; i++)
	{
		v_lightPosTan[i] = v_TBN * u_pointLight[i].position;
	}
	v_viewPosTan = v_TBN * u_viewPos;
	v_normalTan = v_TBN * vec3(u_Transform * vec4(finalNormal, 0.0f));
}
