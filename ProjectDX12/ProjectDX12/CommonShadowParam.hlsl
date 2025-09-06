
#ifndef ___COMMON_SHADOW_PARAM_HLSL___
#define ___COMMON_SHADOW_PARAM_HLSL___

struct ShadowRecieverParam
{
    float4x4 LVP[3];
    float4 CascadeAreas;
};

struct CalcShadowParam
{
    float3 posWS;
    float4 reciever[3];
    float3 camPos;
    float3 camForward;
    float4 cascadeArea;
};

#endif