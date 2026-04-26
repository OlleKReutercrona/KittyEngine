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
    float4 colour = colourTex.Sample(fullscreenSampler, input.uv).rgba;
    
    float intensity = dot(colour.rgb, bloomSampleTreshold);
    if (intensity > 1.0f)
    {
        return colour;
    }
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}