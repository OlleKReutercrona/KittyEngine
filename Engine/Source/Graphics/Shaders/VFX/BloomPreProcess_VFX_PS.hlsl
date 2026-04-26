#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

cbuffer FullScreenBuffer : register(b1)
{
    float2 CARedOffset;
    float2 CAGreenOffset;
    
    float2 CABlueOffset;
    float CAMultiplier;
    float boundSaturation;
    
    float3 ColourCorrection;
    float boundContrast;
    
    float4 boundTint;
    
    float4 boundBlackPoint;
    
    float boundExposure;
    float3 bloomSampleTreshold;
}

Texture2D colourTex : register(t0);

float4 main(PostProcessVertexToPixel input) : SV_TARGET
{
    return colourTex.Sample(fullscreenSampler, input.uv).rgba;
}