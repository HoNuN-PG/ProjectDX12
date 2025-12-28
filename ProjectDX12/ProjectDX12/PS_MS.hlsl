
struct PS_IN
{
    float4 Position     : SV_Position;
    float3 Normal       : NORMAL0;
    uint   MeshletIndex : COLOR0;
};

float4 main(PS_IN input) : SV_TARGET
{
    float4 output = float4(1, 1, 1, 1);

    output.xyz = float3(
            float(input.MeshletIndex & 1),
            float(input.MeshletIndex & 3) / 4,
            float(input.MeshletIndex & 7) / 8);

    return output;
}