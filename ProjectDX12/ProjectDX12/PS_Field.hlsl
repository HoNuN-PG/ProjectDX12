struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
	float y: TEXCOORD1;
};

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float3 lightDir = float3(-1.0f, -1.0f, 1.0f);

	color.xyz = lerp(float3(0.2f, 1.0f, 0.1f),float3( 0.95f, 0.85f, 0.7f), saturate(pin.y / 6.5f));

    float3 N = normalize(pin.normal);
    float3 L = normalize(lightDir);
    color.rgb *= saturate(dot(N, -L) * 0.5f + 0.5f);

	// color.xyz = lerp(float3(0.2f, 0.8f, 1.0f), color.xyz, smoothstep(0.6, 0.61f, saturate(pin.y / 5)));

    return color;
}