
#include "CommonMeshlet.hlsl"

struct MatrixWVP
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};

struct CULL_INFO
{
    float4 planes[6];
};

struct MESHLET_INFO
{
    uint MeshletCount;
    float3 pad1;
};

struct CULL_DATA
{
    float4 BoundingSphere;
    uint NormalCone;
    float ApexOffset;
};

groupshared PAYLOAD Payload;

ConstantBuffer<MatrixWVP> WVP : register(b0);
ConstantBuffer<CULL_INFO> CullInfo : register(b1);
ConstantBuffer<MESHLET_INFO> MeshletInfo : register(b2);

StructuredBuffer<CULL_DATA> CullData : register(t0);

// 可視性チェック
bool IsVisible(CULL_DATA CullData)
{
    float4 center = mul(float4(CullData.BoundingSphere.xyz, 1.0f), WVP.World);
    
    for (int i = 0; i < 6; i++)
    {
        if (dot(center, CullInfo.planes[i]) < -CullData.BoundingSphere.w) return false;
    }
    
    return true;
}

[numthreads(LANE_COUNT, 1, 1)]
void main(uint dtid : SV_DispatchThreadID)
{
    bool visible = false;

    if (dtid < MeshletInfo.MeshletCount)
    {
        visible = IsVisible(CullData[dtid]);
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        Payload.MeshletIndices[index] = dtid;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, Payload);
}