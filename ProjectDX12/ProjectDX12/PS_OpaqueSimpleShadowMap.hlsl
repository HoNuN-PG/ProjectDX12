
#include "CommonParam.hlsl"

struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float2 z : TEXCOORD1;
};

Texture2D<float4> albedoTex : register(t0); // ƒAƒ‹ƒxƒh
SamplerState samp : register(s0);

cbuffer Common : register(b0)
{
    CommpnParam CommonParams;
}

float4 main(PS_IN input) : SV_TARGET
{
    float alpha = albedoTex.Sample(samp, input.uv).a;
    clip(alpha - CommonParams.AlphaCut);
    float4 depth = float4(input.z.x, input.z.y, 1, 1);
    return depth;
}
