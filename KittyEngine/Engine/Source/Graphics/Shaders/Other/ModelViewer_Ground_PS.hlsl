#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
Texture2D effectsTex : register(t3);

struct GBufferOutput
{
    float4 worldPosition : SV_TARGET0;
    float4 albedo : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_TARGET3;
    float4 effects : SV_TARGET4;
    float4 ambientOcclusionAndCustom : SV_TARGET5;
    //float4 depth : SV_TARGET6;
};

GBufferOutput main(PixelInput aInput)
{
    float2 scaledUV = aInput.texCoord;
    scaledUV.x = scaledUV.x * 25.0f;
    scaledUV.y = scaledUV.y * 25.0f;
    float4 albedo = colourTex.Sample(defaultSampler, scaledUV).rgba;
    const float3 pixelNormal = -aInput.normal;

    GBufferOutput output;
    output.worldPosition = aInput.worldPos;
    output.albedo = float4(albedo.rgb, 1.0f);
    output.normal = float4(0.5f + 0.5f * pixelNormal, 1.0f);
    output.material = float4(1.0f,1.0f,1.0f, 1.0f);

    output.ambientOcclusionAndCustom = float4(1.0f, aInput.normal.xyz); // gba are unused, put whatever data you want here!
    output.ambientOcclusionAndCustom.g = aInput.attributes.x;
    output.effects = effectsTex.Sample(defaultSampler, scaledUV).rgba;
    return output;
}

