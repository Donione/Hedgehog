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
    matrix u_ViewProjection;
    vector u_viewPos;
};

cbuffer ObjectConstantBuffer : register(b1)
{
    matrix u_Transform;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float3 pos : POSITIONT;
    float3 normal : NORMAL;
};

PSInput VSMain(float3 position : a_position, float3 normal : a_normal)
{
    PSInput result;

    float4 pos = mul(u_Transform, float4(position, 1.0f));

    result.position = mul(u_ViewProjection, pos);
    result.pos = pos;
    result.normal = mul(u_Transform, float4(normal, 0.0f));

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 lightColor = float3(0.56f, 0.14f, 0.14f);
    float3 lightPosition = float3(3.0f, 3.0f, 5.0f);

    float3 ambient = float3(0.0f, 0.0f, 0.0f);

    float3 norm = normalize(input.normal);
    float3 lightDir = normalize(lightPosition - input.pos);
    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = diff * lightColor;

    float specularStrength = 0.2;
    float3 viewDir = normalize(u_viewPos - input.pos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = specularStrength * spec * lightColor;

    return float4(ambient + diffuse + specular, 1.0f);
}
