
#include "CommonLighting.hlsl"

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 posWS : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

cbuffer Grid : register(b0)
{
    float4 gridParam; // .x = GridDistance
}
SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
    float gridx = frac(input.posWS.x * (1.0f / gridParam.x));
    gridx = step(0.05f, gridx);
    float gridz = frac(input.posWS.z * (1.0f / gridParam.x));
    gridz = step(0.05f, gridz);
    float value = gridx * gridz;
    float3 color = lerp(float3(0.7f,0.7f,0.7f), float3(0.2f,0.2f,0.2f), value);
    return float4(color, 1);
}