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

cbuffer ObjectConstantBuffer : register(b1)
{
    matrix u_Transform;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord: TEXCOORD;
};

PSInput VSMain(float4 position : a_position, float4 color : a_color, float2 texCoords : a_textureCoordinates)
{
    PSInput result;

    matrix VPM = mul(u_ViewProjection, u_Transform);
    result.position = mul(VPM, position);

    result.texCoord = texCoords;

    return result;
}


Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    return t1.Sample(s1, input.texCoord);
}
