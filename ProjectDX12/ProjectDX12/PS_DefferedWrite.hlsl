
struct PS_IN
{
	float4 pos			: SV_POSITION;
	float3 normal		: NORMAL0;
	float2 uv			: TEXCOORD0;
	float4 screenPos	: TEXCOORD1;
};

struct PS_OUT
{
	float4 albedo	: SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 depth	: SV_TARGET2;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

PS_OUT main(PS_IN input)
{
	PS_OUT output;

	output.albedo = tex.Sample(samp, input.uv);

	output.normal.xyz = input.normal * 0.5f + 0.5f;
	output.normal.w = 1;

	output.depth.xyz = input.screenPos.z / input.screenPos.w;
	output.depth.w = 1;

	return output;
}