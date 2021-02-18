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
};

cbuffer SceneConstantBuffer2 : register(b1)
{
    matrix u_Transform;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

PSInput VSMain(float3 position : a_position, float3 normal : a_normal)
{
    PSInput result;

    result.position = mul(mul(u_ViewProjection, u_Transform), float4(position, 1.0f));

    float3 V = normalize(float3(3.0f, -3.0f, -5.0f) - position);
    float3 ambient = float3(0.2f, 0.2f, 0.2f) * float3(0.8f, 0.2f, 0.2f);
    ambient = float3(0.0f, 0.0f, 0.0f);
    float diffuse = dot(normalize(normal), V);

    if (diffuse > 0)
    {
        ambient = ambient + (diffuse * float3(0.56f, 0.14f, 0.14f));
    }
    result.color = min(float3(1.0f, 1.0f, 1.0f), ambient);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4( input.color, 1.0f );
}
