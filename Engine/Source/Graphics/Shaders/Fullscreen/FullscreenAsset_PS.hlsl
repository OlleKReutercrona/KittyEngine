#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

cbuffer FSData : register(b9)
{
    float2 uvMin;
    float2 uvMax;
}

Texture2D colourTex : register(t0);

float4 main(PixelInput aInput) : SV_TARGET
{

    float2 workingUV = aInput.texCoord;
    workingUV.x = workingUV.x * (uvMax.x - uvMin.x) + uvMin.x;
    workingUV.y = workingUV.y * (uvMax.y - uvMin.y) + uvMin.y;

    float4 colour = colourTex.Sample(fullscreenSampler, workingUV).rgba;
    
    //if (colour.a < 0.1f)
    //{
    //    discard;
    //}

    return float4(colour);
}

