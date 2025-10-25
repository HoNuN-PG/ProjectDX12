
#include "CommonLighting.hlsl"

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

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

Texture2D<float4> albedoTex : register(t0); // アルベド
Texture2D<float4> normalTex : register(t1); // 法線
Texture2D<float4> depthTex  : register(t2); // 深度
SamplerState samp : register(s0);

cbuffer Camera : register(b0)
{
    CameraParam CameraParams;
}

cbuffer Light : register(b1)
{
    LightParam LightParams;
}

cbuffer vpInv : register(b2)
{
    float4x4 ViewProjInv;
}

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = albedoTex.Sample(samp, input.uv);

	// 法線
    float3 N = normalTex.Sample(samp, input.uv).xyz;
    
	// ワールド座標
	// 射影空間逆行列を用いてデプス値からワールド座標を計算
    float3 posWS = CalcWorldPosFromUVZ(input.uv, depthTex.Sample(samp, input.uv).r, ViewProjInv);

    // 拡散反射と鏡面反射を求める
    float lambert = CalcLambert(N, LightParams.LightDir.xyz) * LightParams.LightAdd.x + LightParams.LightAdd.y;

	// 色計算
    float3 finalColor = color.xyz * lambert;

    return float4(finalColor, color.a);
}