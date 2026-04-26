#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);

float4 main(PixelInput aInput) : SV_TARGET
{
    float2 uvFix = aInput.texCoord;

    float clientW = clientResolution.x;
    float clientH = clientResolution.y;
    float clientAspect = clientW / clientH;

    uvFix.y /= clientAspect;


    float4 colour = colourTex.Sample(fullscreenSampler, uvFix).rgba;

    if (colour.a < 0.1f)
    {
        discard;
    }

    return float4(colour);
    //return float4(aInput.texCoord.x, aInput.texCoord.y, 0.0f, 1.0f);

}

