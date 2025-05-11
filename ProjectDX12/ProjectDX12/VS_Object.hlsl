
struct VS_IN
{
	float3 pos		: POSITION0;
    float3 normal	: NORMAL0;
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

struct VS_OUT
{
	float4 pos		: SV_POSITION;
    float3 normal	: NORMAL0;
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

cbuffer WVP : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
}

VS_OUT main(VS_IN input)
{
	VS_OUT output;
	float4 wvp;
	wvp.xyz = input.pos; 
	wvp.w = 1;
	wvp = mul(wvp, world);
	wvp = mul(wvp, view);
	wvp = mul(wvp, proj);
    output.pos = wvp;
    output.normal = mul(input.normal.xyz, (float3x3)world);
    output.normal = normalize(output.normal);
	output.uv		= input.uv;
	output.color	= input.color;
	return output;
}