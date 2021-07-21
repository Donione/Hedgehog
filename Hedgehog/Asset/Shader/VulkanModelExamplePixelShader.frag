#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform ObjectConstantBuffer
{
    mat4 u_transform;
    vec3 u_lightColor;
} objectConstantBuffer;

layout(location = 0) out vec4 a_color;

void main()
{
	a_color = vec4(objectConstantBuffer.u_lightColor, 1.0f);
}