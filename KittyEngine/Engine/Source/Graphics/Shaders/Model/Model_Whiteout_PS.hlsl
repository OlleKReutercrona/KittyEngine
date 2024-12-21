#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
TextureCube textureCube;

float4 main(PixelInput aInput) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

