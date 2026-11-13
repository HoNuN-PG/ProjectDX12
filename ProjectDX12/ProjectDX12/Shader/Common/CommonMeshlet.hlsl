#ifndef ___COMMON_MESHLET_HLSL___
#define ___COMMON_MESHLET_HLSL___


#define LANE_COUNT (32)

struct PAYLOAD
{
    uint MeshletIndices[LANE_COUNT];
};

#endif