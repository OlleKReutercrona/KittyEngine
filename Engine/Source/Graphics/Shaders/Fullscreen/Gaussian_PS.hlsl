#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);

cbuffer GaussianBuffer : register(b6)
{
    float guassianDirection;
    float guassianQuality;
    float guassianSize;
    float gaussianTreshold;
}

float4 main(PostProcessVertexToPixel input) : SV_TARGET
{
    float twoPi = 2 * PI;
   
    float2 radius = guassianSize / clientResolution;

    float4 colour = colourTex.Sample(fullscreenSampler, input.uv);
    
    if (colour.x > gaussianTreshold || colour.y > gaussianTreshold || colour.z > gaussianTreshold)
    {
        // Do nothing
    }
    else
    {
        colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
        
    }
    
    for (float d = 0.0f; d < twoPi; d += (twoPi / guassianDirection))
    {
        for (float i = 1.0 / guassianQuality; i < 1.0f; i += (1.0 / guassianQuality))
        {
            colour += colourTex.Sample(fullscreenSampler, input.uv + float2(cos(d), sin(d)) * radius * i);


        }
    }
    
    colour /= guassianQuality * guassianDirection - 15.0f;
    
    return colour;
}