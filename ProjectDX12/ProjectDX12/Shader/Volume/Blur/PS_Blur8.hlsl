
struct PS_IN
{
    float4 pos  : SV_POSITION;
    float4 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float4 tex2 : TEXCOORD2;
    float4 tex3 : TEXCOORD3;
    float4 tex4 : TEXCOORD4;
    float4 tex5 : TEXCOORD5;
    float4 tex6 : TEXCOORD6;
    float4 tex7 : TEXCOORD7;
    float2 uv   : TEXCOORD8;
};

cbuffer Gauss : register(b0)
{
    float4 weight[2]; // 重み
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
	// ズレた座標からサンプリングした値にガウスウェイトを乗算
    float4 Color;
    Color =  weight[0].x * tex.Sample(samp, input.tex0.xy);
    Color += weight[0].y * tex.Sample(samp, input.tex1.xy);
    Color += weight[0].z * tex.Sample(samp, input.tex2.xy);
    Color += weight[0].w * tex.Sample(samp, input.tex3.xy);
    Color += weight[1].x * tex.Sample(samp, input.tex4.xy);
    Color += weight[1].y * tex.Sample(samp, input.tex5.xy);
    Color += weight[1].z * tex.Sample(samp, input.tex6.xy);
    Color += weight[1].w * tex.Sample(samp, input.tex7.xy);

    Color += weight[0].x * tex.Sample(samp, input.tex0.zw);
    Color += weight[0].y * tex.Sample(samp, input.tex1.zw);
    Color += weight[0].z * tex.Sample(samp, input.tex2.zw);
    Color += weight[0].w * tex.Sample(samp, input.tex3.zw);
    Color += weight[1].x * tex.Sample(samp, input.tex4.zw);
    Color += weight[1].y * tex.Sample(samp, input.tex5.zw);
    Color += weight[1].z * tex.Sample(samp, input.tex6.zw);
    Color += weight[1].w * tex.Sample(samp, input.tex7.zw);

    return float4(Color.xyz, 1);
}