
#include "CommonLighting.hlsl"

struct PS_IN
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL0;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

cbuffer Camera : register(b0)
{
    CameraParam CameraParams;
}

cbuffer Light : register(b1)
{
    LightParam LightParams;
}

float4 main(PS_IN input) : SV_TARGET
{
    float3 color = input.color.xyz;
    float lambert = CalcLambert(input.normal, LightParams.LightDir.xyz) * LightParams.LightAdd.x + LightParams.LightAdd.y;
    return float4(color * lambert,1);
}