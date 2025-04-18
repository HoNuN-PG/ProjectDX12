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
	const float lacunarity = 3.0f;
	const float gain = 0.5f;

	float amplitude = 0.75f;
	float frequency = 2.0f;

	float n = 0.0f;

	// 炎の表現のために、横方向のみ値を増やす
	// →横方向に圧縮された見た目になる
	float2 vec = pin.uv * float2(5.0f, 1.0f);

	// 炎のゆらぎ
	vec.x += sin(PerlinNoise(vec + sin(t * 0.05f), 3)) * 0.25f;
	vec.y += t * 0.01f;

	int i = 0;
	for (int i = 0; i < 4; ++i)
	{
		n += PerlinNoise(vec, frequency) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	// ノイズの値をもとに色を付ける
	float3 red = float3(1, 0, 0);
	float3 yellow = float3(1, 1, 0);
	float3 color = lerp(red, yellow, n);

	// 上に行くほど透明にする
	float alpha = pin.uv.y;
	alpha = saturate(alpha + pow(n,5));
	if (alpha < 0.5f)
		discard;
	alpha *= pow(pin.uv.y,1.2f);

	return float4(color, alpha);
}