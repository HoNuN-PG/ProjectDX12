#include "CommonFunction.hlsl"

cbuffer Params : register(b0) {
	float t;
};

struct PS_IN
{
	float4 pos	: SV_POSITION0;
	float2 uv	: TEXCOORD0;
};

float4 main(PS_IN pin) : SV_TARGET
{
	float noise = PerlinNoise(pin.uv, 10);
	return float4(noise, noise, noise, 1);
}