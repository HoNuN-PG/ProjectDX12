
struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float2 uv : TEXCOORD8;
};

cbuffer Gauss : register(b0)
{
    float2 weight; // 重み
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
	// ズレた座標からサンプリングした値にガウスウェイトを乗算
    float4 Color;
    Color = weight.x * tex.Sample(samp, input.tex0.xy);
    Color += weight.y * tex.Sample(samp, input.tex1.xy);

    Color += weight.x * tex.Sample(samp, input.tex0.zw);
    Color += weight.y * tex.Sample(samp, input.tex1.zw);

    return float4(Color.xyz, 1);
}