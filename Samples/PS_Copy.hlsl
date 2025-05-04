
struct PS_IN
{
    float4 pos  : SV_POSITION;
    float2 uv   : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
    return tex.Sample(samp, input.uv);
}