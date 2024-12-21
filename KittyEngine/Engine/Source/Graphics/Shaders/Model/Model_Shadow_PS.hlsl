#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
void main(PixelInput aInput)
{
    float alpha = colourTex.Sample(defaultSampler, aInput.texCoord).a;
    if (alpha < 0.5f) {discard;}

    return;
}