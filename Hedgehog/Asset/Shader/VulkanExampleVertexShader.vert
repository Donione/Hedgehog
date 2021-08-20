#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_textureCoordinates;
layout(location = 3) in float a_segmentID;


layout(set = 0, binding = 0) uniform SceneConstantBuffer
{
    mat4 u_projectionView;
} sceneConstantBuffer;

layout(set = 1, binding = 0) uniform ObjectConstantBuffer
{
    mat4 u_transform;
    mat4 u_segmentTransforms[65];
} objectConstantBuffer;


layout(location = 0) out vec4 v_position;
layout(location = 1) out vec4 v_color;

void main()
{
	mat4 finalTransform;
	if (a_segmentID != -1.0f)
	{
		finalTransform = objectConstantBuffer.u_transform * objectConstantBuffer.u_segmentTransforms[int(a_segmentID)];
	}
	else
	{
		finalTransform = objectConstantBuffer.u_transform;
	}

	gl_Position = sceneConstantBuffer.u_projectionView * finalTransform * a_position;

	v_position = a_position;
	v_color = a_color;
}
