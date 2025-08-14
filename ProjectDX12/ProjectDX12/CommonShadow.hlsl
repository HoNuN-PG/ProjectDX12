
struct ShadowRecieverParam
{
    float4x4 LVP[3];
    float4 CascadeAreas;
};

struct CalcShadowParam
{
    float3 posWS;
    float4 reciever[3];
    float3 camPos;
    float3 camForward;
    float4 cascadeArea;
};

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
            zInShadowMap = shadowMap1.Sample(samp, shadowUV).rg;
            break;
        case (1):
            zInShadowMap = shadowMap2.Sample(samp, shadowUV).rg;
            break;
        default:
            zInShadowMap = shadowMap3.Sample(samp, shadowUV).rg;
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
                shadow = 1;
            }
        }
    }
    return shadow;
}