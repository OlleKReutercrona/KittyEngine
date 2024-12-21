#include "Commons/DeferredCommons.hlsli"
#include "Commons/PBRFunctions.hlsli"

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
};

Texture2D directionalLightShadowMap : register(t14);

float4 main(DeferredVertexToPixel aInput) : SV_TARGET
{
    const float2 uv = aInput.position.xy / clientResolution.xy;
    const float3 worldPosition = worldPositionTex.Sample(defaultSampler, uv).rgb;
    const float3 albedo = colourTex.Sample(defaultSampler, uv).rgb;
    const float4 effects = effectsTex.Sample(defaultSampler, uv).rgba;
    const float4 ambientOcclusionAndCustom = ambientOcclusionTex.Sample(defaultSampler, uv).rgba;
    float2 SSAO = SSAOTexture.Sample(defaultSampler, uv).ra;
    if (SSAO.g == 0.0)
        SSAO.r = 1.0f;

    const float3 normal = normalize(2.f * normalTex.Sample(defaultSampler, uv).xyz - 1.f);

    // vN should be normalized object space normal
    float3 vN = normalize(ambientOcclusionAndCustom.gba);

    // vN_unit should be normalized world space normals
    const float3 vN_unit = normalize(normal);

    
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

    // Shadows

    float4 directionalLightProjectedPositionTemp = mul(directionalLightCameraTransform, float4(worldPosition, 1.0f));
    float3 directionLightProjectedPosition = directionalLightProjectedPositionTemp.xyz / directionalLightProjectedPositionTemp.w;

    float shadowFactor = 1.0f;
    if (clamp(directionLightProjectedPosition.x, -1.0f, 1.0f) == directionLightProjectedPosition.x &&
        clamp(directionLightProjectedPosition.y, -1.0f, 1.0f) == directionLightProjectedPosition.y)
    {
        const float computedZ = directionLightProjectedPosition.z;
        // Need no bias when frontface culling
        const float bias = 0.000001f;

        float totalFactor = 0.0f;

        // Filter kernel for [PCF] percentage-closer filtering eg. (3x3)
        const int numSamples = 9;
        // Offset scale decides how much the shadow edge is moved for "blurring"
        const float offsetScale = 0.0005f;
        [unroll]
        for (int i = -numSamples / 2; i <= numSamples / 2; ++i)
        {
            [unroll]
            for (int j = -numSamples / 2; j <= numSamples / 2; ++j)
            {
                const float2 sampleOffset = float2(i, j) / float(numSamples);
                const float2 sampleUV = 0.5f + float2(0.5f, -0.5f) * (directionLightProjectedPosition.xy + sampleOffset * offsetScale);
                const float shadowMapZ = directionalLightShadowMap.Sample(shadowSampler, sampleUV).r;

                totalFactor += (computedZ < shadowMapZ) ? 1.0f : 0.0f;
            }
        }

        shadowFactor = totalFactor / float(numSamples * numSamples);
    }

    const float metalness = material.b;
    const float roughness = material.g;

    const float3 specular = lerp((float3) 0.0005f, albedo.rgb, metalness);
    const float3 colour = lerp((float3) 0.0f, albedo.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - worldPosition);

    float3 directionalLight = EvaluateDirectionalLight(colour, specular, normal,
    roughness, directionalLightColour, directionalLightDirection, toEye) * (directionalLightIntensity * UNITY_LIGHT_DIRECTIONAL_INTENSITY_DIFFERENCE);

    // return float4(worldPosition,1.0f);

    // float ao = (SSAO + ambientOcclusionAndCustom.r) / 2.0f;
    // float ao = clamp(SSAO + ambientOcclusionAndCustom.r, 0.0f, 1.0f);

    float3 ambiance = EvaluateAmbiance(
                        cubemapTexture, normal, ambientOcclusionAndCustom.gba,
                        toEye, roughness, SSAO.r,
                        colour, specular);

    float emissive = effects.r;

    float3 litColour = directionalLight * max(shadowFactor, 0.5f) + ambientLightIntensity * ambiance;
    float3 finalColour = lerp(litColour, albedo, emissive);

    
    // return float4(toEye.xyz, 1.0f);

    float factor = 0.002f;
    factor += sin(currentTime) / 1000.0f;


    bool outline = true; // ambientOcclusionAndCustom.g > 0.1f;
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
    
    return float4(finalColour, 1.0f);
}