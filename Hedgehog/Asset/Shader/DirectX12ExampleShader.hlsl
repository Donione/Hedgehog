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
    float4 color : COLOR;
};

PSInput VSMain(float4 position : a_position, float4 color : a_color, float2 texCoords : a_textureCoordinates)
{
    PSInput result;

    //gl_Position = u_ViewProjection * u_Transform * a_position
    matrix VPM = mul(u_ViewProjection, u_Transform);
    result.position = mul(VPM, position);

    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    //float4 colour = {1.0f, 0.0f, 0.0f, 1.0f};
    //return colour;
    return input.color;
}
