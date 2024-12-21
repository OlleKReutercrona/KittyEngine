#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);

float4 main(PixelInput aInput) : SV_TARGET
{
    float4 colour = colourTex.Sample(fullscreenSampler, aInput.texCoord).rgba;
    if (colour.a < 0.1f)
    {
        discard;
        return colour;
    }

    return float4(colour);
}

