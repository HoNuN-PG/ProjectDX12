
#include "CommonLighting.hlsl"

struct PS_IN
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

cbuffer Light:register(b0)
{
	float4x4 vpInv;
	float4 ligParam;
	float4 ligColor;
	float4 camPos;
}

Texture2D albedo	: register(t0);
Texture2D normal	: register(t1);
Texture2D depth		: register(t2);
Texture2D emission	: register(t3);
SamplerState samp	: register(s0);

float3 CalcWorldPosFromUVZ(float2 uv, float zInProjectionSpace, float4x4 mViewProjInv)
{
	// 正規スクリーン座標系（-1～1）
	float3 screenPos;
	screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
	screenPos.z = zInProjectionSpace;

	// 射影空間変換逆行列からワールド座標を計算
	float4 worldPos = mul(mViewProjInv, float4(screenPos, 1.0f));
	worldPos.xyz /= worldPos.w;
	return worldPos.xyz;
}

float4 main(PS_IN input) : SV_TARGET
{
	float4 color = albedo.Sample(samp, input.uv);
	
	// 法線深度
	float3 N = normalize(normal.Sample(samp, input.uv).xyz * 2.0f - 1.0f);
	float  D = depth.Sample(samp, input.uv);

	float3 posWS = CalcWorldPosFromUVZ(input.uv, D, vpInv);

	// その他のパラメータ
	float3 L = normalize(ligParam.xyz);		// ライトのベクトル
	float3 V = normalize(posWS.xyz - camPos.xyz);	// 視線ベクトル(ワールド座標←カメラ位置)
	float3 R = normalize(reflect(V, N));			// 反射ベクトル

	// 拡散反射の計算
	float lambert = CalcLambert(N, L);
	lambert += ligColor.w;
	float3 lamColor = lambert * ligColor.xyz;
	color.xyz *= lamColor;

	// 鏡面反射の計算
	float specular = saturate(dot(-L, R));
	specular = pow(specular, 5.0f);
	float3 specColor = specular * ligColor.xyz;

	// 環境光(IBL)の計算
	float PI = 3.141592f;
	float u = atan2(-R.x, -R.z) / PI * 0.5f + 0.5f; // 反射ベクトルからU値の算出
	float v = asin(R.y) / PI + 0.5f;				// 反射ベクトルからV値の算出
	float4 emissive = emission.Sample(samp, float2(u, 1 - v));

	// 最終色
	color.xyz += emissive.xyz * specColor;
	color.xyz *= ligParam.w;
	return color;
}