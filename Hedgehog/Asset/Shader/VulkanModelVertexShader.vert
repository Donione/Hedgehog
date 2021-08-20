#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_offset;

layout(set = 0, binding = 0) uniform SceneConstantBuffer
{
	mat4 u_projectionView;
    vec3 u_viewPos;
} sceneConstantBuffer;

layout(set = 2, binding = 0) uniform ObjectConstantBuffer
{
	mat4 u_transform;
    float u_magnitude;
} objectConstantBuffer;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec3 v_Normal;

void main()
{
	vec4 position = objectConstantBuffer.u_transform * vec4(a_position + a_offset, 1.0f);
	
	gl_Position = sceneConstantBuffer.u_projectionView * position;

	v_Position = vec3(position);
	v_Normal = normalize(vec3(objectConstantBuffer.u_transform * vec4(a_normal, 0.0)));
}
