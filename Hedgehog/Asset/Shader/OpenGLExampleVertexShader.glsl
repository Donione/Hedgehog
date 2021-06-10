#version 460 core

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_textureCoordinates;
layout(location = 3) in float a_segmentID;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_segmentTransforms[65];

out vec4 v_position;
out vec4 v_color;

void main()
{

    mat4 finalTransform;
    if (a_segmentID != -1.0f)
        finalTransform = u_Transform * u_segmentTransforms[int(a_segmentID)];
    else
        finalTransform = u_Transform;

	gl_Position = u_ViewProjection * finalTransform * a_position;

	v_position = a_position;
	v_color = a_color;
}
