#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

DebugLinePSInputType main(DebugLineVSInputType input)
{
    DebugLinePSInputType output;
    
    float4x4 identityM = float4x4
        (1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1);
    
    float4 vertexObjectPos = float4(input.position.xyz, 1);
    float4 vertexWorldPos = mul(identityM, vertexObjectPos);
    float4 vertexClipPos = mul(worldToClipSpaceMatrix, vertexWorldPos);
    
    output.position = vertexClipPos;
    output.color = input.color;
    
    return output;
}