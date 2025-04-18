#include "CommonFunction.hlsl"

cbuffer Params : register(b0) {
	float t;
};

struct PS_IN
{
	float4 pos : SV_POSITION0;
	float2 uv : TEXCOORD0;
};

float4 main(PS_IN pin) : SV_TARGET
{
	int block = 10;
	float noise = rand(floor(pin.uv * block));
	return float4(noise, noise, noise, 1.0f);
}