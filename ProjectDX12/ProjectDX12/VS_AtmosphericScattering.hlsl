
static const float fInnerRadius = 1000.0f;
static const float fOuterRadius = 1015.0f;
static const float Kr = 0.0025f;
static const float Km = 0.001f;

cbuffer WVP : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

struct PS_IN
{
	float4 pos			: SV_POSITION0;
	float3 normal		: NORMAL0;
	float2 uv			: TEXCOORD0;
	float4 posWS		: TEXCOORD1;
};

PS_IN main(VS_IN input)
{
	PS_IN output;

	// WVP変換
	float4 wvp;
	wvp = float4(input.pos,1);		// ローカル座標
	wvp = mul(wvp, world);			// ワールド座標 
	output.posWS = normalize(wvp) * fOuterRadius;
	wvp = mul(wvp, view);			// ビュー座標
	wvp = mul(wvp, proj);			// プロジェクション座標
	output.pos = wvp;

	// UV座標
	output.uv = input.uv;

	return output;
}