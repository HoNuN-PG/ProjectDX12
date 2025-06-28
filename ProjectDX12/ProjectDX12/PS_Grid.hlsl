
// https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 posWS : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

cbuffer Grid : register(b0)
{
    float GridSize;
    float SubGridNum;
    float GridWidth;
    float pad1;
}

float3 CalcBackgroundColor()
{
    return float3(0.7f, 0.7f, 0.7f);
}

float3 CalcGridColor()
{
    return float3(0.2f, 0.2f, 0.2f);
}

float CalcGrid(float2 uv)
{
    float2 uvDeriv = fwidth(uv);
    float2 drawWidth = max(GridWidth, uvDeriv);
    float2 LineAA = uvDeriv * 1.5f;
    float2 gridUV = 1.0 - abs(frac(uv) * 2.0f - 1.0f);
    float2 grid2 = smoothstep(drawWidth + LineAA, drawWidth - LineAA, gridUV);
    grid2 *= saturate(GridWidth / drawWidth);
    float grid = lerp(grid2.x, 1, grid2.y);
    
    return grid;
}

float4 main(PS_IN input) : SV_TARGET
{
    float grid = CalcGrid(input.posWS.xz);    
    float3 color = lerp(CalcBackgroundColor(), CalcGridColor() * 0.75f, grid);
    
    return float4(color, 1);
}