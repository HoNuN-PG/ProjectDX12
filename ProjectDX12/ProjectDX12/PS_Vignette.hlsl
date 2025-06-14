
struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer Vignette : register(b0)
{
    float4 color;
    float start;
    float range;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 albedo = float4(0, 0, 0, 1);
    albedo = tex.Sample(samp, pin.uv);

	// Å‘å‹——£
    const float max = length(float2(1.0f, 1.0f) - float2(0.5f, 0.5f));
	// ‰æ–Ê’†‰›‚©‚ç‚Ì‹——£
    float d = length(pin.uv - float2(0.5f, 0.5f));

	// ³‹K‰»
    d = (d / max);

	// •âŠÔ’l
    float f = 0.0f;
    f = d - start;
    f = f / range;
    f = saturate(f);
    f = pow(f, 2.0f);

	// •âŠÔ
    albedo = lerp(albedo, color, f);

    return float4(albedo.xyz, 1.0f);
}