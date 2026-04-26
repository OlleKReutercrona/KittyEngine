#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);

cbuffer BloomBuffer : register(b8)
{
    float bloomStep;
    float bloomTreshold;
    float bloomBlending;
    float padding2;
};

float4 main(PostProcessVertexToPixel input) : SV_TARGET
{
    float4 result;

    float2 scaledUV = input.uv * bloomStep;
    
    float2 pixelOffset = float2(ddx(scaledUV.x), ddy(scaledUV.y));
    
    float3 p00 = colourTex.Sample(fullscreenSampler, scaledUV + pixelOffset * float2(-1.0f, -1.0f)).rgb;
    float3 p01 = colourTex.Sample(fullscreenSampler, scaledUV + pixelOffset * float2(-1.0f, +1.0f)).rgb;
    float3 p10 = colourTex.Sample(fullscreenSampler, scaledUV + pixelOffset * float2(+1.0f, -1.0f)).rgb;
    float3 p11 = colourTex.Sample(fullscreenSampler, scaledUV + pixelOffset * float2(+1.0f, +1.0f)).rgb;
    
    result.rgb = bloomTreshold * (p00 + p01 + p10 + p11);
    result.a = bloomBlending;
    
    return result;
}