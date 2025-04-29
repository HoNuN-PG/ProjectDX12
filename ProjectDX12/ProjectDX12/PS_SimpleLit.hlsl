
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

cbuffer Camera : register(b0)
{
    float4 camParam;
}
cbuffer Camera : register(b1)
{
    float4 ligParam;
    float4 ligColor;
}
SamplerState samp : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
    return float4(ligColor.xyz,1);
}