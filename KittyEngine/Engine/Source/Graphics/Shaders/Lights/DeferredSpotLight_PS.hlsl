#include "Commons/DeferredCommons.hlsli"
#include "Commons/PBRFunctions.hlsli"

cbuffer SpotLightData : register(b2)
{
    float3 lightPosition;
    float lightIntensity;
    float3 lightDirection;
    float lightRange;
    float3 lightColour;
    float lightInnerAngle;
    float lightOuterAngle;
    bool lightIsActive;
    float2 padding;
};

float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
    const float2 uv = aInput.position.xy / clientResolution.xy;
    const float3 worldPosition = worldPositionTex.Sample(defaultSampler, uv).rgb;
    const float3 albedo = colourTex.Sample(defaultSampler, uv).rgb;
    const float3 normal = normalize(2.0f * normalTex.Sample(defaultSampler, uv).xyz - 1.0f);
    const float4 material = materialTex.Sample(defaultSampler, uv);

    if (!lightIsActive || material.a == 0.0f)
    {
        discard;
    }

    const float metalness = material.b;
    const float roughness = material.g;

    const float3 specular = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 colour = lerp((float3) 0.0f, albedo.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition);

    float3 spotLight = EvaluateSpotLight(colour, specular, normal, roughness, lightColour, lightIntensity * UNITY_LIGHT_SPOT_INTENSITY_DIFFERENCE,
        lightRange, lightPosition, -lightDirection, lightOuterAngle, lightInnerAngle, toEye,
        worldPosition.xyz);

    return float4(spotLight.rgb, 1.0f);
}
