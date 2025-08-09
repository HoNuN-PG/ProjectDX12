
struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float2 z : TEXCOORD1;
};

float4 main(PS_IN input) : SV_TARGET
{
    float4 depth = float4(input.z.x, input.z.y, 1, 1);
    return depth;
}
