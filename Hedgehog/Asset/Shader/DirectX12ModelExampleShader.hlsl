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
    float4x4 u_projectionView;
};

cbuffer ObjectConstantBuffer : register(b1)
{
    float4x4 u_transform;
    float3 u_lightColor;
}

struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(float3 position : a_position, float3 normal : a_normal)
{
    PSInput result;

    float4x4 VPM = mul(u_projectionView, u_transform);
    result.position = mul(VPM, float4(position, 1.0f));

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(u_lightColor, 1.0f);
}
