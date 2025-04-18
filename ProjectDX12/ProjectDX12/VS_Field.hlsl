#include "CommonFunction.hlsl"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
	float y : TEXCOORD1;
};

cbuffer WVP : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

float3 CalcNormal(float2 UV, float block, float height, float2 margin)
{
	float2 uv;

	// X軸方向の計算
	uv = UV - float2(margin.x, 0);
	float hVX1 = TurbulenceFBM(3, 0.5f, 0.75f, 3, uv, block) * height;

	uv = UV + float2(margin.x, 0);
	float hVX3 = TurbulenceFBM(3, 0.5f, 0.75f, 3, uv, block) * height;

	float3 x = float3(margin.x, (hVX3 - hVX1) / 2, 0);

	// Z軸方向の計算
	uv = UV - float2(0, margin.y);
	float hVZ1 = TurbulenceFBM(3, 0.5f, 0.75f, 3, uv, block) * height;

	uv = UV + float2(0, margin.y);
	float hVZ3 = TurbulenceFBM(3, 0.5f, 0.75f, 3, uv, block) * height;

	float3 z = float3(0, (hVZ3 - hVZ1) / 2 , margin.y);

	// 法線の計算
	float3 normal = normalize(cross(z, x));

	return normal;
}

VS_OUT main(VS_IN vin)
{
	float block = 2;
	float height = 7;

    VS_OUT vout;
    vout.pos = float4(vin.pos, 1.0f);
	vout.pos.y = TurbulenceFBM(3, 0.45f, 0.75f, 3, vin.uv, block) * height; // ノイズ関数(uv) * height;
    vout.pos.y -= height * 0.5f;

    vout.pos = mul(vout.pos, world);
	vout.y = vout.pos.y;
    vout.pos = mul(vout.pos, view);
    vout.pos = mul(vout.pos, proj);

	// 法線の計算
	vout.normal = CalcNormal(vin.uv,block, height, float2(1.0f / 50.0f, 1.0f / 50.0f));
	
    vout.normal = mul(vout.normal, (float3x3) world);
    vout.uv = vin.uv;

    return vout;
}