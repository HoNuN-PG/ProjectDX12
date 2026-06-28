
// https://logicalbeat.co.jp/blog/8242/
// https://shikihuiku.github.io/post/wave_intrinsics1/

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

bool IsVisible(CULL_DATA CullData)
{
    float4 center = mul(float4(CullData.BoundingSphere.xyz, 1.0f), WVP.World);
    
    for (int i = 0; i < 6; i++)
    {
        // 座標と平面の距離の計算
        if (dot(center, CullInfo.planes[i]) < -CullData.BoundingSphere.w) return false;
    }
    
    return true;
}

[numthreads(LANE_COUNT, 1, 1)]
void main(uint dtid : SV_DispatchThreadID)
{
    bool visible = false;

    // 可視性チェック
    if (dtid < MeshletInfo.MeshletCount)
    {
        visible = IsVisible(CullData[dtid]);
    }

    // 可視性チェックが成功したメッシュレットをペイロードに格納
    if (visible)
    {
        uint index = WavePrefixCountBits(visible); // WAVE中で何番目に可視性チェックが成功したか
        Payload.MeshletIndices[index] = dtid; // 可視性チェックが成功した場合にメッシュレットのインデックスを格納
    }

    uint visibleCount = WaveActiveCountBits(visible); // WAVE中で可視性チェックが成功したActiveLaneの数
    DispatchMesh(visibleCount, 1, 1, Payload); // 可視性チェックが成功した数分のスレッドグループが生成される
}