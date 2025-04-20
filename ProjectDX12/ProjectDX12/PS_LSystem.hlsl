struct PS_IN
{
    float4 pos		: SV_POSITION0;
    float3 normal	: NORMAL0;
	float4 color	: COLOR0;
};

cbuffer Light:register(b0)
{
	float4 ligParam;
	float4 ligColor;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = pin.color;

	color.xyz = saturate(color.xyz * dot(pin.normal, -normalize(ligParam.xyz)) + 0.15f);

	return color;
};
