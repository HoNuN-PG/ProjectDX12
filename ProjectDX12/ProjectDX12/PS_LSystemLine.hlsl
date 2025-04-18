struct PS_IN
{
	float4 pos : SV_POSITION0;
	float3 normal : NORMAL0;
	float4 color : COLOR0;
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

	color = float4(0.8,0.5,0.2,1);

	return color;
};
