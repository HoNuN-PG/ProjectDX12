
struct VS_IN
{
    float3 pos  : POSITION0;
    float2 uv   : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos  : SV_POSITION;
    float2 uv   : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;
    float4 wvp;
    wvp.xy = input.pos.xy * 2;
    wvp.z = 0;
    wvp.w = 1;
    output.pos = wvp;
    output.uv = input.uv;
    return output;
}