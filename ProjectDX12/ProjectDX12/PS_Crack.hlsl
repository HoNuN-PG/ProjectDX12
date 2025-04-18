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
	float dfc;
	float dfe;
	float2 cto;
	float cell;

	GetVoronoi(pin.uv,7,10 + (t * 0.05f),dfc,dfe,cto,cell);

	float3 color = float3(134.0f / 255,74.0f / 255,43.0f / 255);
	float noise = PerlinNoise(pin.uv, 10);
	color = lerp(color, float3(0.3f, 1.0f, 0.1f), pow(noise,3));

	dfe = step(dfe, 0.1f);
	color = lerp(color, float3(0, 0, 0), dfe);

	return float4(color, 1.0f);
}