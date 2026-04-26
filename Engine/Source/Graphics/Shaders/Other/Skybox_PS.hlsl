#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/DeferredCommons.hlsli"
#include "Commons/InputCommons.hlsli"

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
    GBufferOutput output;
    float3 direction = aInput.worldPos.xyz;
    float3 colour = cubemapTexture.Sample(defaultSampler, direction).rgb;
    output.albedo = float4(colour, 1.0f);

    output.worldPosition = 0;
    output.normal = 0;
    output.material = float4(0, 0, 0, 1);
    output.ambientOcclusionAndCustom = float4(0, 0, 0, 0);

    output.effects = float4(1.0f, 0.0f, 0.0f, 1.0f);


    return output;
}