#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
TextureCube textureCube;

float4 main(PixelInput aInput) : SV_TARGET
{
    float3 colour = colourTex.SampleLevel(defaultSampler, aInput.texCoord, 0).rgb;
    float3 normal = normalTex.SampleLevel(defaultSampler, aInput.texCoord, 0).wyz;
    float3 material = materialTex.SampleLevel(defaultSampler, aInput.texCoord, 0).rgb;

    float3 ambientLight = { 0.0f, 0.0f, 0.0f };
    float3 directionalLight = { 0.0f, 0.0f, 0.0f };
    float3 pointLight = { 0.0f, 0.0f, 0.0f };
    float3 spotLight = { 0.0f, 0.0f, 0.0f };
    float3 specular = { 0.0f, 0.0f, 0.0f };
    const float ambientOcclusion = material.r;

    float metalness = 0.0f;
    float roughness = 0.0f;
    //float emissive = 0.0f;

    normal = 2.0f * normal - 1.0f;
    normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
    normal = normalize(normal);

    float3x3 TBN = float3x3(
		normalize(aInput.tangent.xyz),
		normalize(-aInput.bitan.xyz),
		normalize(aInput.normal.xyz)
	);

	// Can save an instruction here by instead doing
	// normalize(mul(normal, TBN)); It works because
	// TBN is a 3x3 and therefore TBN^T is the same
	// as TBN^-1. However, it is considered good form to do this.
    TBN = transpose(TBN);
    normal = normalize(mul(TBN, normal));

    metalness = material.b;
    roughness = material.g;
    //emissive = material.b;

    specular = lerp((float3) 0.04f, colour.rgb, metalness);
    colour = lerp((float3) 0.0f, colour.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - aInput.worldPos.xyz);

    // Ambient light
    ambientLight = EvaluateAmbiance(textureCube, normal, aInput.normal.xyz, toEye, roughness, ambientOcclusion, colour, specular);

	// Directional light
    directionalLight = EvaluateDirectionalLight(colour, specular, normal,
    roughness, directionalLightData.directionalLightColour, -directionalLightData.directionalLightDirection, toEye) * directionalLightData.directionalLightIntensity;

    // Point lights
    for (uint pIndex = 0; pIndex < numberOfPointLights; ++pIndex)
    {
        if (!pointLights[pIndex].active)
        {
            continue;
        }

        pointLight += EvaluatePointLight(colour, specular, normal,
        roughness, pointLights[pIndex].colour, pointLights[pIndex].intensity,
        pointLights[pIndex].range, pointLights[pIndex].position, toEye, aInput.worldPos.xyz);

    }

	// Spot lights
    for (uint sIndex = 0; sIndex < numberOfSpotLights; ++sIndex)
    {
        if (!spotLights[sIndex].active)
        {
            continue;
        }

        spotLight += EvaluateSpotLight(colour, specular, normal, roughness, spotLights[sIndex].colour, spotLights[sIndex].intensity,
        spotLights[sIndex].range, spotLights[sIndex].position, spotLights[sIndex].direction, spotLights[sIndex].outerAngle, spotLights[sIndex].innerAngle, toEye,
        aInput.worldPos.xyz );
    }

	// Final colour
    //const float3 emissiveColour = colour * emissive;
    const float3 finalColour = saturate(ambientLight + directionalLight + pointLight + spotLight);
    // Tonemap
    return float4(finalColour, 1.0f);
}

