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
	int block = 10;
	int2 i_uv = floor(pin.uv * block);	// ブロック番号
	float2 f_uv = frac(pin.uv * block);		// ブロックごとの値

	// バリューノイズの計算に使用する4箇所の位置を定義
	int2 offset[] = {
		{0,0}, {1,0},
		{0,1}, {1,1},
	};

	// 4隅の値を取得
	float lt, rt, lb, rb;
	lt = rand(i_uv + offset[0]);
	rt = rand(i_uv + offset[1]);
	lb = rand(i_uv + offset[2]);
	rb = rand(i_uv + offset[3]);

	// ノイズの値を補間
	float top = lerp(lt, rt, f_uv.x);
	float bottom = lerp(lb, rb, f_uv.x);
	float noise = lerp(top, bottom, f_uv.y);

	return float4(noise, noise, noise, 1);
}