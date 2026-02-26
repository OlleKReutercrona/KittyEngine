#include "DeferredCommons.hlsli"
#include "PBRFunctions.hlsli"
#include "ShadowCommons.hlsli"

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
    uint shadowMapInformation;
    float padding;
    float4x4 spotlightCameraTransform;
};

float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
    const float2 uv = aInput.position.xy / clientResolution.xy;
    const float3 worldPosition = worldPositionTex.Sample(defaultSampler, uv).rgb;
    const float3 albedo = colourTex.Sample(defaultSampler, uv).rgb;
    const float3 normal = normalize(2.0f * normalTex.Sample(defaultSampler, uv).xyz - 1.0f);
    const float4 material = materialTex.Sample(defaultSampler, uv);

    if (material.a == 0.0f)
    {
        discard;
    }

    // Shadows
    
    float shadowFactor = CalculateShadow(shadowMapInformation, spotlightCameraTransform, worldPosition, 0.000005f, 10, 0.0075f);
    
    const float metalness = material.b;
    const float roughness = material.g;

    const float3 specular = lerp((float3) 0.04f, albedo.rgb, metalness);
    const float3 colour = lerp((float3) 0.0f, albedo.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition);

    float3 spotLight = EvaluateSpotLight(colour, specular, normal, roughness, lightColour, lightIntensity * UNITY_LIGHT_SPOT_INTENSITY_DIFFERENCE,
        lightRange, lightPosition, -lightDirection, lightOuterAngle, lightInnerAngle, toEye,
        worldPosition.xyz);

    
    //return float4(1 - shadowFactor, 1 - shadowFactor, 0.0f, 1.0f);
    return float4(spotLight.rgb * max(shadowFactor, shadowMinClamp), 1.0f);
}
