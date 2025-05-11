
struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float z : TEXCOORD0;
};

struct PS_OUT
{
    float4 depth : SV_Target0; // デプス値
    float4 normal : SV_Target1; // 法線
};

PS_OUT main(PS_IN input) : SV_Target0
{
    PS_OUT output;

	// デプス値
    output.depth = float4(input.z, input.pos.z, 0, 1.0f);

	// 法線
    float3 N = input.normal.xyz;
    output.normal.xyz = (normalize(N) * 0.5f) + 0.5f; // 法線書き込み(0~1へ変換)
    output.normal.w = 1.0f;

    return output;
}
