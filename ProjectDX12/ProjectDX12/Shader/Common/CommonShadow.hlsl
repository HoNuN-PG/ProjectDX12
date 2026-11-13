
#ifndef ___COMMON_SHADOW_HLSL___
#define ___COMMON_SHADOW_HLSL___

#include "CommonShadowParam.hlsl"

// シャドウマップ
Texture2D shadowMap1 : register(t10);
Texture2D shadowMap2 : register(t11);
Texture2D shadowMap3 : register(t12);

float CalcShadow(CalcShadowParam param, SamplerState samp)
{
	// シャドウの強さ
    float shadow = 0;

	// ピクセル方向へのベクトルとカメラの正面方向の内積から、使用するシャドウマップを選択
    float3 toPixel = param.posWS.xyz - param.camPos.xyz;
    float area = dot(param.camForward.xyz, toPixel);
	// カスケードの選択
    int idx = (area < param.cascadeArea.x) ? 0 : (area < param.cascadeArea.y) ? 1 : 2;
    
    // ライトビュースクリーンからUVへ
    float2 shadowUV = param.reciever[idx].xy / param.reciever[idx].w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
    
    [branch]
    if (!(shadowUV.x >= 0.0f && shadowUV.x <= 1.0f && shadowUV.y >= 0.0f && shadowUV.y <= 1.0f))
    {
        return 0;
    }
    
    // シャドウマップ
    float2 zInShadowMap;
    switch (idx)
    {
        case 0:
            zInShadowMap = shadowMap1.Sample(samp, shadowUV);
            break;
        case 1:
            zInShadowMap = shadowMap2.Sample(samp, shadowUV);
            break;
        case 2:
            zInShadowMap = shadowMap3.Sample(samp, shadowUV);
            break;
        default:
            break;
    }
    
	// 深度の取得
    float zInLVP = param.reciever[idx].z;

	// 遮蔽判定
    if (zInLVP >= 0 && zInLVP <= 1.0f)
    {
        if (zInLVP > zInShadowMap.r + DEPTH_THRESHOLD)
        {
            shadow = 1;
        }
    }
    return shadow;
}

#endif