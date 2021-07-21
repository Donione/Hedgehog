#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

layout(set = 0, binding = 0) uniform SceneConstantBuffer
{
    mat4 u_projectionView;
} sceneConstantBuffer;

layout(set = 1, binding = 0) uniform ObjectConstantBuffer
{
    mat4 u_transform;
    vec3 u_lightColor;
} objectConstantBuffer;


void main()
{
	gl_Position = sceneConstantBuffer.u_projectionView * objectConstantBuffer.u_transform * vec4(a_position, 1.0f);
}
