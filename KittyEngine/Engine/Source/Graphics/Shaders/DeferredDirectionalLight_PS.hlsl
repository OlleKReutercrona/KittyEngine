#include "DeferredCommons.hlsli"
#include "PBRFunctions.hlsli"
#include "ShadowCommons.hlsli"

// needs:
// Fullscreen textures from gbuffer
// Active cameras transform

cbuffer DirectionalLight : register(b2)
{
    float3 directionalLightDirection;
    float directionalLightIntensity;
    float3 directionalLightColour;
    float ambientLightIntensity;
    float4x4 directionalLightCameraTransform;
    unsigned int shadowMapInformation;
    float3 padding;
};


float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
	const float2 uv = aInput.position.xy / clientResolution.xy;
    const float3 worldPosition = worldPositionTex.Sample(defaultSampler, uv).rgb;
    const float3 albedo = colourTex.Sample(defaultSampler, uv).rgb;
    const float4 effects = effectsTex.Sample(defaultSampler, uv).rgba;
    const float4 ambientOcclusionAndCustom = ambientOcclusionTex.Sample(defaultSampler, uv).rgba;
    const float SSAO = SSAOTexture.Sample(defaultSampler, uv).r;
    
    const float3 normal = normalize(2.f * normalTex.Sample(defaultSampler, uv).xyz - 1.f);
    //const float3 normal = normalTex.Sample(defaultSampler, uv).xyz;

    
    const float4 material = materialTex.Sample(defaultSampler, uv);

    if (material.a == 0.0f)
    {
        discard;
        ////calculate the direction of the "ray" from the camera to this pixel, for drawing the skybox. we need to use the view and projection matrices to do this.
        ////clipToWorldSpaceMatrix
        //float4 tempPos = aInput.position;
//
        //float4 vertexWorldPosition = mul(clipToWorldSpaceMatrix, tempPos);
        //float3 direction = vertexWorldPosition.xyz - cameraPosition.xyz;
        //float3 colour = cubemapTexture.Sample(defaultSampler, direction).rgb;
        //return vertexWorldPosition;
    }
    
    float shadowFactor = CalculateShadow(shadowMapInformation, directionalLightCameraTransform, worldPosition, 0.00005f, 10, 0.0075f);
    

	const float metalness = material.b;
    const float roughness = material.g;

	const float3 specular = lerp((float3)0.04f, albedo.rgb, metalness);
	const float3 colour = lerp((float3)0.0f, albedo.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition);

    float3 directionalLight = EvaluateDirectionalLight(colour, specular, normal,
    roughness, directionalLightColour, directionalLightDirection, toEye) * (directionalLightIntensity * UNITY_LIGHT_DIRECTIONAL_INTENSITY_DIFFERENCE);

    // return float4(worldPosition,1.0f);

    float ao = (SSAO + ambientOcclusionAndCustom.r) / 2.0f;
    // float ao = clamp(SSAO + ambientOcclusionAndCustom.r, 0.0f, 1.0f);
    float3 ambiance = EvaluateAmbiance(
                        cubemapTexture, normal, ambientOcclusionAndCustom.gba,
                        toEye, roughness, ao,
                        colour, specular);
    
    float emissive = effects.r;
    float3 finalColour = directionalLight * max(shadowFactor, 0.3f) * (1.0f - emissive) + colour * emissive; // + emissive * float3(0.0f,0.0f,0.0f);

    
    // return float4(toEye.xyz, 1.0f);

    float factor = 0.002f;
    factor += sin(currentTime) / 1000.0f;


    bool outline = true;// ambientOcclusionAndCustom.g > 0.1f;
    if (!outline)
    {
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                float2 offset = float2(i, j) * factor;
                float2 workingUV = uv + offset;
                if (!(workingUV.x < 0 || workingUV.x > 1 || workingUV.y < 0 || workingUV.y > 1))
                {
                    float4 outlineSecondary = ambientOcclusionTex.SampleLevel(fullscreenSampler, workingUV, 0.0f);
                    if (outlineSecondary.g > 0.1f)
                    {
                        finalColour += float3(1.0f, 0.0f, 0.0f);
                    }
                }
            }
        }        
    }
    
    return float4(finalColour + ambiance, 1.0f);
}