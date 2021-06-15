#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_offset;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = vec4(a_position + a_offset, 0.0, 1.0);
    fragColor = a_color;
}
