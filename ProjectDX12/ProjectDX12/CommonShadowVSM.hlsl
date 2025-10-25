
#ifndef ___COMMON_SHADOW_VSM_HLSL___
#define ___COMMON_SHADOW_VSM_HLSL___

#include "CommonShadowParam.hlsl"

// シャドウマップ
Texture2D bokeh_shadowMap1 : register(t10);
Texture2D bokeh_shadowMap2 : register(t11);
Texture2D bokeh_shadowMap3 : register(t12);

float CalcShadowVSM(CalcShadowParam param, SamplerState samp)
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
            zInShadowMap = bokeh_shadowMap1.Sample(samp, shadowUV);
            break;
        case 1:
            zInShadowMap = bokeh_shadowMap2.Sample(samp, shadowUV);
            break;
        case 2:
            zInShadowMap = bokeh_shadowMap3.Sample(samp, shadowUV);
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
            float depth_sq = zInShadowMap.r * zInShadowMap.r;
            float variance = max(zInShadowMap.g - depth_sq, DEPTH_THRESHOLD);
            float md = zInLVP - zInShadowMap.r;
            float rate = variance / (variance + md * md);
            shadow = 1 - rate;
        }
    }
    return shadow;
}

#endif