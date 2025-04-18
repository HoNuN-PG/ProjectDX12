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

	float3 color = float3(0.1,0.2,1);
	color = lerp(color, float3(1, 1, 1), pow(dfc,3));

	dfe = step(dfe, 0.1);

	return float4(color, 1.0f);

	return float4(dfe,dfe,dfe, 1.0f);
}