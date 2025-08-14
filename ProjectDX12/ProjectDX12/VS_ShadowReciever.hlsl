
#include "CommonDefine.hlsl"
#include "CommonLighting.hlsl"
#include "CommonShadow.hlsl"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 posWS : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 reciever[3] : TEXCOORD1;
};

cbuffer WVP : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

cbuffer Light : register(b1)
{
    LightParam LightParams;
}

cbuffer ShadowReciever : register(b2)
{
    ShadowRecieverParam ShadowRecieverParams;
}

VS_OUT main(VS_IN input)
{
    VS_OUT output;

	// WVP•ÏŠ·
    float4 wvp;
    wvp.xyz = input.pos;
    wvp.w = 1;
    wvp = mul(wvp, world);
    output.posWS = wvp;
    wvp = mul(wvp, view);
    wvp = mul(wvp, proj);
    output.pos = wvp;
    output.normal = mul(input.normal.xyz, (float3x3) world);
    output.normal = normalize(output.normal);
    output.uv = input.uv;

	// LVP•ÏŠ·
    float4 lvp;
    lvp.xyz = input.pos;
    lvp.w = 1;
    lvp = mul(lvp, world);
    output.reciever[0] = mul(lvp, ShadowRecieverParams.LVP[0]);
    output.reciever[1] = mul(lvp, ShadowRecieverParams.LVP[1]);
    output.reciever[2] = mul(lvp, ShadowRecieverParams.LVP[2]);

    output.reciever[0].z = length(output.posWS.xyz - LightParams.LihgtPos.xyz) / LIGHT_LENGTH;
    output.reciever[1].z = output.reciever[0].z;
    output.reciever[2].z = output.reciever[0].z;

    return output;
}
