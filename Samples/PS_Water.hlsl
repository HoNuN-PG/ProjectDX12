#include "CommonFunction.hlsl"

struct PS_IN
{
	float4 pos		: SV_POSITION;
	float3 normal	: NORMAL0;
	float2 uv		: TEXCOORD0;
	float3 posWS	: TEXCOORD1;
};

cbuffer Param : register(b0)
{
	float time;
	float3 camPos;
}

float CalcFBM(float2 uv)
{
	int block = 2;
	float scale = 4;
	float lacunarity = 2.0f; float gain = 0.85f; float amplitude = 0.75f; int roll = 3;
	float fbm1 = PerlinFBM(lacunarity, gain, amplitude, roll, uv + time * 0.01f, block);
	float fbm2 = PerlinFBM(lacunarity, gain, amplitude, roll, uv * float2(1.7f, 2.8f) + time * 0.05f, block);
	float fbm = PerlinFBM(lacunarity, gain, amplitude, roll, uv + float2(fbm1, fbm2) + time * 0.15f, block);

	return fbm;
}

float3 CalcNormal(float2 UV,float2 margin)
{
	float2 uv;

	// X軸方向の計算
	uv = UV - float2(margin.x, 0);
	float hVX1 = CalcFBM(uv);

	uv = UV + float2(margin.x, 0);
	float hVX3 = CalcFBM(uv);

	float3 x = float3(margin.x, (hVX3 - hVX1) / 2 * margin.x, 0); // 変位の平均を正しく求める

	// Z軸方向の計算
	uv = UV - float2(0, margin.y);
	float hVZ1 = CalcFBM(uv);

	uv = UV + float2(0, margin.y);
	float hVZ3 = CalcFBM(uv);

	float3 z = float3(0, (hVZ3 - hVZ1) / 2 * margin.y, margin.y);

	// 法線の計算
	float3 normal = normalize(cross(z, x));

	return normal;
}

float Fresnel(float f0, float u)
{
	return f0 + (1.0f - f0) * pow(1.0f - u, 5.0f);
}

float CalcSpec(float3 N, float3 L, float3 pos, float3 camPos)
{
	// 反射ベクトル計算
	float3 ref = reflect(L, N);
	// 視点ベクトル計算
	float3 vec = camPos.xyz - pos;
	vec = normalize(vec);

	// 鏡面反射の強さを計算
	float spec = saturate(dot(ref, vec));
	spec = pow(spec, 5.0f);

	return spec;
}

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float3 lightDir = float3(-1.0f, -1.0f, 1.0f);

	float fbm = CalcFBM(pin.uv);
	color.xyz = (fbm * float3(0.2f, 0.6f, 1.0f));

	float3 N = CalcNormal(pin.uv, float2(1.0f / 50.0f, 1.0f / 50.0f));
	float3 L = normalize(lightDir);
	float3 V = normalize(camPos - pin.posWS);
	float3 H = normalize(-L + V);
	float VdotN = saturate(dot(V, N));
	float VdotH = saturate(dot(V, H));
	float F = 1 - pow(1 - Fresnel(0.2f, VdotN),2);

	color.xyz *= saturate(dot(N, -L) * 0.5f + 0.5f);
	color.a = F;

	float spec = CalcSpec(N,-L,pin.posWS,camPos) * 2;
	color.xyz += spec;
	color.a += spec;

	return color;
}