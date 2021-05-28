#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vv_Position[];
in vec3 vv_Normal[];

out vec3 v_Position;
out vec3 v_Normal;

uniform mat4 u_ViewProjection;

uniform float u_magnitude;


vec3 explode(vec3 position, vec3 normal)
{
    return position + normal * u_magnitude;
}

vec3 GetFaceNormal()
{
    vec3 a = vv_Position[1] - vv_Position[0];
    vec3 b = vv_Position[2] - vv_Position[0];
    return normalize(cross(a, b));
}

void main()
{
    vec3 normal = GetFaceNormal();

    v_Position = explode(vv_Position[0], normal);
    gl_Position = u_ViewProjection * vec4(v_Position, 1.0f);
    v_Normal = vv_Normal[0];
    EmitVertex();

    v_Position = explode(vv_Position[1], normal);
    gl_Position = u_ViewProjection * vec4(v_Position, 1.0f);
    v_Normal = vv_Normal[1];
    EmitVertex();

    v_Position = explode(vv_Position[2], normal);
    gl_Position = u_ViewProjection * vec4(v_Position, 1.0f);
    v_Normal = vv_Normal[2];
    EmitVertex();

    EndPrimitive();
}
