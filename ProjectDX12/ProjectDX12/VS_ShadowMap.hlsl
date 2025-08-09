
#include "CommonInfo.hlsl"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float2 z : TEXCOORD1;
};

cbuffer WVP : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

cbuffer Light : register(b1)
{
    float4 ligParam1;
    float4 ligParam2;
    float4 ligColor;
}

cbuffer ShadowMaps : register(b2)
{
    float4x4 lvpc;
}

VS_OUT main(VS_IN input)
{
    VS_OUT output;

	// WVP•ÏŠ·
    float4 wvp;
    wvp.xyz = input.pos;
    wvp.w = 1;
    wvp = mul(wvp, world);
    float3 wpos = wvp.xyz;
    wvp = mul(wvp, lvpc);
    output.pos = wvp;

    output.uv = input.uv;

    output.z.x = length(wpos - ligParam2.xyz) / LIGHT_LENGTH;
    output.z.y = pow(output.z.x, 2);

    return output;
}