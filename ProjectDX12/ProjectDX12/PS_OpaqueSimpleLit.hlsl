
#include "CommonParam.hlsl"
#include "CommonLighting.hlsl"

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

Texture2D<float4> albedoTex : register(t0); // ƒAƒ‹ƒxƒh
SamplerState samp : register(s0);

cbuffer Camera : register(b0)
{
    CameraParam CameraParams;
}

cbuffer Light : register(b1)
{
    LightParam LightParams;
}

cbuffer Common : register(b2)
{
    CommpnParam CommonParams;
}

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = albedoTex.Sample(samp, input.uv);
    clip(color.a - CommonParams.AlphaCut);
    float lambert = CalcLambert(input.normal, LightParams.LightDir.xyz) * LightParams.LightAdd.x + LightParams.LightAdd.y;
    return float4(color.xyz * lambert, 1);
}