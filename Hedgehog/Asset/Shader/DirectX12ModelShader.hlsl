//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 u_ViewProjection;
    float3 u_viewPos;
};

struct DirectionalLight
{
    float3 color;
    float pad0;
    float3 direction;
    float pad1;
};

struct PointLight
{
    float3 color;
    float3 position;
    float3 attenuation;// x = constant, y = linear, z = quadratic components
};

struct SpotLight
{
    float3 color;
    float3 position;
    float3 attenuation; // x = constant, y = linear, z = quadratic components
    float3 direction;
    float2 cutoffAngle;
};

cbuffer SceneLightsBuffer : register(b1)
{
    DirectionalLight u_directionalLight;
    int u_numberOfPointLights;
    PointLight u_pointLight[3];
    SpotLight u_spotLight;
}

cbuffer ObjectConstantBuffer : register(b2)
{
    float4x4 u_Transform;
    float u_magnitude;
}


struct GSInput
{
    float4 position : SV_POSITION;
    float3 pos : POSITIONT;
    float3 normal : NORMAL;
};

GSInput VSMain(float3 position : a_position, float3 normal : a_normal, float3 offset : a_offset)
{
    GSInput result;

    float4 pos = mul(u_Transform, float4(position, 1.0f));

    result.position = mul(u_ViewProjection, pos);
    result.pos = pos.xyz;
    result.normal = mul(u_Transform, float4(normal, 0.0f)).xyz;

    return result;
}


float3 GetDirection(float3 v0, float3 v1, float3 v2)
{
    float3 a = v1 - v0;
    float3 b = v2 - v0;
    return normalize(cross(a, b));
}

float3 Explode(float3 position, float3 direction)
{
    return position + direction * u_magnitude;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float3 pos : POSITIONT;
    float3 normal : NORMAL;
};

[maxvertexcount(3)]
void GSMain(triangle GSInput input[3], inout TriangleStream<PSInput> OutputStream)
{
    PSInput result;

    float3 direction = GetDirection(input[0].pos, input[1].pos, input[2].pos);

    result.pos = Explode(input[0].pos, direction);
    result.position = mul(u_ViewProjection, float4(result.pos, 1.0f));
    result.normal = input[0].normal;
    OutputStream.Append(result);

    result.pos = Explode(input[1].pos, direction);
    result.position = mul(u_ViewProjection, float4(result.pos, 1.0f));
    result.normal = input[1].normal;
    OutputStream.Append(result);

    result.pos = Explode(input[2].pos, direction);
    result.position = mul(u_ViewProjection, float4(result.pos, 1.0f));
    result.normal = input[2].normal;
    OutputStream.Append(result);
}


// Directional light is a sun-like light, that is light virtually infinitely far away so the light rays are parallel
// so the light position is irrelecant, only the light direction is taken into account
float3 CalculateDirectionalLight(float3 lightDirection, // normalized direction from pixel to the light
                                 float3 lightColor,
                                 float3 position,       // pixel position
                                 float3 normal)         // normalized pixel normal vector
{
    //float3 objectColor = float3(0.333f, 0.125f, 0.024f);

    float3 objectColor = float3(1.0f, 1.0f, 1.0f);
    
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    
    float diff = max(dot(normal, lightDirection), 0.0f);
    float3 diffuse = diff * lightColor;

    float specularStrength = 0.2;
    float3 viewDirection = normalize(u_viewPos - position);
    float3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    float3 specular = specularStrength * spec * lightColor;

    return (ambient + diffuse + specular) * objectColor;
}

// PointLight is basically a directional light with attenuation and the light position is taken into account
float3 CalculatePointLight(float3 lightPosition,
                           float3 lightColor,
                           float3 attenuation,   // vector of attenuation components, x = constant, y = linear, z = quadratic
                           float3 position,      // pixel position
                           float3 normal)        // normalized pixel normal vector
{
    float3 lightDirection = normalize(lightPosition - position);

    float3 result = CalculateDirectionalLight(lightDirection, lightColor, position, normal);

    // Attenuation is computed according to the formula:
    //    1 / ( Kc + d * Kl + d^2 * Kq ), where
    //        d = distance
    //       Kc = constant component
    //       Kl = linear component
    //       Kq = quadratic component
    float lightDistance = length(lightPosition - position);
    float att = 1.0f / (attenuation.x + lightDistance * attenuation.y + lightDistance * lightDistance * attenuation.z);

    return att * result;
}

// Spotlight is a pointlight with limited light radius
float3 CalculateSpotLight(float3 lightPosition,
                          float3 lightColor,
                          float3 lightDirection, // normalized direction into the spotlight in world space
                          float2 cutoffAngle,    //cosine of angles from lightDirection to the inner and outer cutoff radius in radians
                          float3 attenuation,    // vector of attenuation components, x = constant, y = linear, z = quadratic
                          float3 position,       // pixel position
                          float3 normal)         // normalized pixel normal vector
{
    float3 result;

    float3 lightDir = normalize(lightPosition - position);
    // The angle between two vectors can be computed using the following formula:
    //    cos theta = dot(u, v) / (length(u) * length(v))
    // Since both u and v are normalized (their length is 1), the formula can be simplified to:
    //    cos theta = dot(u, v),
    // and then
    //    theta = acos(dot(u, v))
    //float theta = acos(dot(lightDir, lightDirection));

    // For soft edges, we compute the light intensity using the following formula:
    //    I = (theta - outer radius) / (outer radius - inner radius)
    // and then limiting the Intensity between 0.0 and 1.0
    // Basically we're computing the ratio between theta and the soft edge area,
    //float epsilon = cutoffAngle.y - cutoffAngle.x;
    //float intensity = smoothstep(0.0f, 1.0f, (cutoffAngle.y - theta) / epsilon);

    // To go without the acos here in the shader, we can input the angles already in the form of cosine of the angle
    // and tweak the calculations as follows:
    float cosTheta = dot(lightDir, lightDirection);
    float epsilon = cutoffAngle.x - cutoffAngle.y;
    float intensity = smoothstep(0.0f, 1.0f, (cosTheta - cutoffAngle.y) / epsilon);

    result = intensity * CalculatePointLight(lightPosition, lightColor, attenuation, position, normal);

    return result;
}


float4 PSMain(PSInput input) : SV_TARGET
{
    float3 result = float3(0.0f, 0.0f, 0.0f);

    float3 norm = normalize(input.normal);

    result += CalculateDirectionalLight(normalize(-u_directionalLight.direction), u_directionalLight.color, input.pos, norm);
    for (int i = 0; i < u_numberOfPointLights; i++)
    {
        result += CalculatePointLight(u_pointLight[i].position, u_pointLight[i].color, u_pointLight[i].attenuation, input.pos, norm);
    }
    result += CalculateSpotLight(u_spotLight.position,
                                 u_spotLight.color,
                                 normalize(-u_spotLight.direction),
                                 u_spotLight.cutoffAngle,
                                 u_spotLight.attenuation,
                                 input.pos,
                                 norm);

    return float4(result, 1.0f);
}
