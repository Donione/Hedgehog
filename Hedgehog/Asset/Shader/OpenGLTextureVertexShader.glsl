#version 460 core

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_textureCoordinates;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_textureCoordinates;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * a_position;

	v_textureCoordinates = a_textureCoordinates;
}
