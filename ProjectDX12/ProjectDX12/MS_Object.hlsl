
struct MatrixWVP
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};

struct MS_IN
{
    float3 Position;
    float3 Normal;
    float2 uv;
    float4 color;
};

struct MS_OUT
{
    float4 Position     : SV_Position;
    float3 Normal       : NORMAL0;
    uint MeshletIndex   : COLOR0;
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

ConstantBuffer<MatrixWVP> WVP : register(b0);

StructuredBuffer<MS_IN> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
StructuredBuffer<uint> UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);

uint3 UnpackPrimitive(uint primitive)
{
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(Meshlet m, uint index)
{
    return UnpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

uint GetVertexIndex(Meshlet m, uint localIndex)
{
    return UniqueVertexIndices[m.VertOffset + localIndex];
}

MS_OUT GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    MS_IN v = Vertices[vertexIndex];

    // 頂点処理
    MS_OUT mout;
    mout.Position = mul(float4(v.Position, 1), WVP.World);
    mout.Position = mul(float4(mout.Position), WVP.View);
    mout.Position = mul(float4(mout.Position), WVP.Proj);
    mout.Normal = mul(v.Normal.xyz, (float3x3) WVP.World);
    mout.MeshletIndex = meshletIndex;

    return mout;
}

[NumThreads(128, 1, 1)]                 // 最大128スレッド
[OutputTopology("triangle")]
void main(
    uint gtid   : SV_GroupThreadID,     // スレッドグループ内のスレッドID
    uint gid    : SV_GroupID,           // スレッドグループID
    out vertices MS_OUT verts[256],     // 頂点データ
    out indices uint3   tris[256]       // プリミティブごとの頂点番号
)
{
    Meshlet m = Meshlets[gid];

    // スレッドグループの頂点数とプリミティブ数を設定
    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.PrimCount)
    {
        tris[gtid] = GetPrimitive(m, gtid);
    }

    if (gtid < m.VertCount)
    {
        uint vertexIndex = GetVertexIndex(m, gtid);
        verts[gtid] = GetVertexAttributes(gid, vertexIndex);
    }
}
