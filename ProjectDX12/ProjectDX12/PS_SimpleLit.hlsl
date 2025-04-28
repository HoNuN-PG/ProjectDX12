
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
    return input.color;
}