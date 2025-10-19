
struct VS_IN
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float2 uv : TEXCOORD8;
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
    output.tex0.xy = float2(0.0f, 1.0f / height);
    output.tex1.xy = float2(0.0f, 2.0f / height);
	// 反対方向にテクセルをずらした座標を格納
    output.tex0.zw = output.tex0.xy * -1.0f;
    output.tex1.zw = output.tex1.xy * -1.0f;
	// 基準テクセルを加算
    output.tex0 = output.tex0 * uvScale + float4(tex, tex);
    output.tex1 = output.tex1 * uvScale + float4(tex, tex);

    output.uv = tex;

    return output;
}