
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
    float4 camParam;
}
cbuffer Light : register(b1)
{
    float4 ligParam1;
    float4 ligParam2;
    float4 ligColor;
}
SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
    float3 color = CalcLambert(input.normal,ligParam1.xyz) * ligParam1.w + ligParam2.w;
    return float4(color * ligColor.xyz * input.color.xyz,1);
}