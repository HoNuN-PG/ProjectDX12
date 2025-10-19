
struct VS_IN
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
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

cbuffer ScreenParam : register(b0)
{
    float width;
    float height;
    float uvScale;
}

VS_OUT main(VS_IN input)
{
    VS_OUT output;

	// 座標計算
    float4 wvp;
    wvp.xy = input.pos.xy * 2;
    wvp.z = 0;
    wvp.w = 1;
    output.pos = wvp;

    float2 tex = input.uv;
	// 横方向にテクセルをずらした座標を格納
    output.tex0.xy = float2(1.0f / width, 0.0f);
    output.tex1.xy = float2(2.0f / width, 0.0f);
    output.tex2.xy = float2(3.0f / width, 0.0f);
    output.tex3.xy = float2(4.0f / width, 0.0f);
    output.tex4.xy = float2(5.0f / width, 0.0f);
    output.tex5.xy = float2(6.0f / width, 0.0f);
    output.tex6.xy = float2(7.0f / width, 0.0f);
    output.tex7.xy = float2(8.0f / width, 0.0f);
	// 反対方向にテクセルをずらした座標を格納
    output.tex0.zw = output.tex0.xy * -1.0f;
    output.tex1.zw = output.tex1.xy * -1.0f;
    output.tex2.zw = output.tex2.xy * -1.0f;
    output.tex3.zw = output.tex3.xy * -1.0f;
    output.tex4.zw = output.tex4.xy * -1.0f;
    output.tex5.zw = output.tex5.xy * -1.0f;
    output.tex6.zw = output.tex6.xy * -1.0f;
    output.tex7.zw = output.tex7.xy * -1.0f;
	// 基準テクセルを加算
    output.tex0 = output.tex0 * uvScale + float4(tex, tex);
    output.tex1 = output.tex1 * uvScale + float4(tex, tex);
    output.tex2 = output.tex2 * uvScale + float4(tex, tex);
    output.tex3 = output.tex3 * uvScale + float4(tex, tex);
    output.tex4 = output.tex4 * uvScale + float4(tex, tex);
    output.tex5 = output.tex5 * uvScale + float4(tex, tex);
    output.tex6 = output.tex6 * uvScale + float4(tex, tex);
    output.tex7 = output.tex7 * uvScale + float4(tex, tex);

    output.uv = tex;

    return output;
}