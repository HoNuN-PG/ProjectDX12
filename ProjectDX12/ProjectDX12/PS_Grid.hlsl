
// https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 posWS : POSITION0;
    float3 normal : NORMAL0;
    float3 normalLS : NORMAL1;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4x4 worldMatrix : TEXCOORD1;
};

cbuffer Grid : register(b0)
{
    float GridSize;
    float SubGridNum;
    float2 pad1;
    float4 ObjectPositionWS;
}
cbuffer Camera : register(b1)
{
    float4 camParam;
}
Texture2D tex : register(t0);
SamplerState samp : register(s0);

float3 CalcBackgroundColor()
{
    return float3(0.7f, 0.7f, 0.7f);
}

float3 CalcSubGridColor()
{
    return float3(0.3f, 0.3f, 0.3f);
}

float3 CalcGridColor()
{
    return float3(0.2f, 0.2f, 0.2f);
}

float2 CalcNormalRB(float3 normalLS)
{
    float r = saturate(lerp(-1.0f, 2.0f, normalLS.x));
    float b = saturate(lerp(-1.0f, 2.0f, normalLS.z));
    return float2(r,b);
}

float CalcGrid(float3 posWS, float3 normalLS, float4x4 world, float size)
{
    // Calc Local Space
    float3 f = abs(posWS) - abs(ObjectPositionWS.xyz);
    // x
    float3 px = mul(float4(1, 0, 0, 0), world).xyz;
    float x = dot(f, normalize(px));
    // y
    float3 py = mul(float4(0, 1, 0, 0), world).xyz;
    float y = dot(f, normalize(py));
    // z
    float3 pz = mul(float4(0, 0, 1, 0), world).xyz;
    float z = dot(f, normalize(pz));
    // MakeFloat3()
    float3 p = float3(x, y, z);
    
    // Scale
    p /= size;
    
    // TextureBlend
    float2 NormalRB = CalcNormalRB(abs(normalLS));
    float A = tex.Sample(samp, p.xz);
    float B = tex.Sample(samp, p.yz);
    float AlB = lerp(A, B, NormalRB.x);
    float C = tex.Sample(samp, p.xy);
    float AlBlC = lerp(AlB, C, NormalRB.y);
    
    return AlBlC;
}

float CalcMainGrid(float3 posWS, float3 normalLS, float4x4 world)
{
    return saturate(CalcGrid(posWS, normalLS, world, GridSize));
}

float CalcSubGrid(float3 posWS, float3 normalLS, float4x4 world)
{
    return saturate(CalcGrid(posWS, normalLS, world, GridSize / SubGridNum));
}

float4 main(PS_IN input) : SV_TARGET
{
    float3 color = float3(0, 0, 0);
    color = lerp(CalcBackgroundColor(), CalcSubGridColor(), CalcSubGrid(input.posWS, input.normalLS, input.worldMatrix));
    color = lerp(color, CalcGridColor(), CalcMainGrid(input.posWS, input.normalLS, input.worldMatrix));
    
    // <<< new
    float lineWidth = 0.02f;
    float2 uv = input.uv * 100;
    float2 uvDeriv = fwidth(uv);
    float2 drawWidth = max(lineWidth, uvDeriv);
    float2 LineAA = uvDeriv * 1.5f;
    float2 gridUV = 1.0 - abs(frac(uv) * 2.0f - 1.0f);
    float2 grid2 = smoothstep(drawWidth + LineAA, drawWidth - LineAA, gridUV);
    grid2 *= saturate(lineWidth / drawWidth);
    float grid = lerp(grid2.x, 1, grid2.y);
    color = lerp(CalcBackgroundColor(), CalcGridColor(), grid);
    // >>> new
    
    return float4(color, 1);
}