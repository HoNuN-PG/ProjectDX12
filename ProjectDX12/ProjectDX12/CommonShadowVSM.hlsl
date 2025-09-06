
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
    int idx = 2;
    if (area < param.cascadeArea.x)
    {
        idx = 0;
    }
    else if (area < param.cascadeArea.y)
    {
        idx = 1;
    }
    
	// 深度の取得
    float zInLVP = param.reciever[idx].z;
    
	// ライトビュースクリーンからUVへ
    float2 shadowUV = param.reciever[idx].xy / param.reciever[idx].w;
    shadowUV *= float2(0.5f, -0.5f);
    shadowUV += 0.5f;
    
    // シャドウマップ
    float2 zInShadowMap;
    switch (idx)
    {
        case (0):
            zInShadowMap = bokeh_shadowMap1.Sample(samp, shadowUV).rg;
            break;
        case (1):
            zInShadowMap = bokeh_shadowMap2.Sample(samp, shadowUV).rg;
            break;
        default:
            zInShadowMap = bokeh_shadowMap3.Sample(samp, shadowUV).rg;
            break;
    }

	// 遮蔽判定
    if (zInLVP >= 0 && zInLVP <= 1.0f)
    {
        if (shadowUV.x >= 0.0f && shadowUV.x <= 1.0f
			&& shadowUV.y >= 0.0f && shadowUV.y <= 1.0f)
        {
            if (zInLVP > zInShadowMap.r + 0.0025f)
            {
                float depth_sq = zInShadowMap.r * zInShadowMap.r;
                float variance = max(zInShadowMap.g - depth_sq, 0.0025f);
                variance = pow(variance, 1.5f);
                float md = zInLVP - zInShadowMap.r;
                float rate = variance / (variance + md * md);
                shadow = 1 - rate;
            }
        }
    }
    return shadow;
}

#endif