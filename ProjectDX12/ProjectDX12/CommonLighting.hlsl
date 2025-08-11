
struct LightParam
{
    float4 LihgtPos;
    float4 LightDir;
    float4 LightColor;
    float4 LightAdd;
};

float CalcLambert(float3 normal, float3 ligDir)
{
    float3 L = normalize(-ligDir);
	float3 N = normalize(normal);
	return saturate(dot(L, N));
}