
#include "CommonLighting.hlsl"
#include "CommonShadow.hlsl"

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 posWS : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 reciever[3] : TEXCOORD1;
};

cbuffer Camera : register(b0)
{
    CameraParam CameraParams;
}

cbuffer Light : register(b1)
{
    LightParam LightParams;
}

cbuffer ShadowReciever : register(b2)
{
    ShadowRecieverParam ShadowRecieverParams;
}

cbuffer Grid : register(b3)
{
    float GridSize;
    float SubGridSize;
    float GridWidth;
    float pad1;
}

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

float CalcGrid(float2 value)
{
    float2 deriv = fwidth(value);
    float2 drawWidth = max(GridWidth, deriv);
    float2 LineAA = deriv * 1.5f;
    float2 gridUV = 1.0 - abs(frac(value) * 2.0f - 1.0f);
    float2 grid2 = smoothstep(drawWidth + LineAA, drawWidth - LineAA, gridUV);
    grid2 *= saturate(GridWidth / drawWidth);
    float grid = lerp(grid2.x, 1, grid2.y);
    
    return grid;
}

float4 main(PS_IN input) : SV_TARGET
{
    float3 color = float3(1, 1, 1);
    float subgrid = CalcGrid(input.posWS.xz * GridSize);
    float grid = CalcGrid(input.posWS.xz * SubGridSize);
    color = lerp(CalcBackgroundColor(), CalcSubGridColor(), subgrid);
    color = lerp(color, CalcGridColor(), grid);
    
    CalcShadowParam param;
    param.posWS = input.posWS;
    param.camPos = CameraParams.CameraPos;
    param.camForward = CameraParams.CameraForward;
    param.cascadeArea = ShadowRecieverParams.CascadeAreas;
    param.reciever[0] = input.reciever[0];
    param.reciever[1] = input.reciever[1];
    param.reciever[2] = input.reciever[2];
    float shadow = CalcShadow(param, samp);
    
    color.xyz = lerp(color.xyz, color.xyz * 0.5f, shadow);
    
    return float4(color, 1);
}