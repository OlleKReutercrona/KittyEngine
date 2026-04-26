#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

#include "../../../EngineAssets/LanguageDefinitions/hlslNoise.hlsli"

//previous render textures
Texture2D renderedFrame : register(t5);
Texture2D gbufferWorldPos : register(t6);
TextureCube skybox : register(t15);

cbuffer WaterBuffer : register(b7)
{
    float3 fogColour;
    float fogDensity;
    float3 causticColour;
    float causticStrengthFactor;
    float3 foamColour;
    float foamStrengthFactor;
}

struct WaterPixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 clipPos : TEXCOORD1;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;
};

float2 NDCToUV(float2 NDC)
{
    return NDC / 2.0f + 0.5f;
}

float2 GetUVWithOffset(float4 clip, float2 offset)
{
    float2 uv = (clip.xy + offset) / clip.w;
    uv = NDCToUV(uv);
    uv.y = 1.0f - uv.y;
    return uv;
}

float4 main(WaterPixelInput aInput) : SV_TARGET
{
    float2 scaledUV = aInput.texCoord;

    float2 screenUV = GetUVWithOffset(aInput.clipPos, 0);

    float time = currentTime * 0.25f;
    float function = 0;

    float noise = voronoiEX(scaledUV * 50.0f, time, function);
    noise = lerp(-1.0f, 1.0f, noise);
	
    
    float4 existing = 0.0f;

    float4 colAtPixel = renderedFrame.Sample(pointSampler, screenUV);
    float4 posAtPixel = gbufferWorldPos.Sample(pointSampler, screenUV);

    float depthAtPixel = distance(posAtPixel.xyz, cameraPosition.xyz);
    float depthAtPlane = distance(aInput.worldPos.xyz, cameraPosition.xyz);

    float depthDifference = depthAtPixel - depthAtPlane;


    float distanceToEdge = -1.0f;
    float3 refractedPos = posAtPixel.xyz;

    if (posAtPixel.w == 0.0f)
    {
        discard;
        posAtPixel = aInput.worldPos;
        posAtPixel.y = -2.0f;
    }

    float2 distanceToScreenEdge = float2(1.0f, 1.0f) - abs(screenUV * 2.0f - 1.0f);



    float3 diff = posAtPixel.xyz - aInput.worldPos.xyz;
    

    float distanceToSample = length(diff);
    if (diff.y > 0.0f)
    {
        distanceToSample = 0.0f;
    }
    
    const float distanceCutoff = 10.0f;
    const float distanceInfluence = 0.1f;

    float2 edgeInfluence = saturate(distanceToScreenEdge);
    const float warpStrength = saturate(depthDifference) * 0.1f;

    float2 sampleUV = GetUVWithOffset(
		aInput.clipPos,
		float2(noise * edgeInfluence.x, noise * edgeInfluence.y) * warpStrength
    );

    float4 colAtWarped = renderedFrame.Sample(pointSampler, sampleUV);
    float4 posAtWarped = gbufferWorldPos.Sample(pointSampler, sampleUV);

    refractedPos = posAtWarped.xyz;

    float depthAtWarped = distance(posAtWarped.xyz, cameraPosition.xyz);

    depthDifference = depthAtWarped - depthAtPlane;

    existing = colAtWarped;
    if (depthDifference <= 0.0f)
    {
        existing = colAtPixel;
        depthDifference = depthAtPixel - depthAtPlane;
        refractedPos = posAtPixel.xyz;
    }

    float fogFactor = exp2(-fogDensity * depthDifference);

	existing.rgb = lerp(fogColour, existing.rgb, fogFactor);


    float3 skyboxSampleDir = float3(0.0f, 1.0f, 0.0f);
    skyboxSampleDir.x += noise * 50.0f;
    skyboxSampleDir.y += noise * -50.0f;

    skyboxSampleDir.xy *= 0.01f;

    float3 toCamera = -(aInput.worldPos.xyz - cameraPosition.xyz);
    float3 ref = reflect(toCamera, skyboxSampleDir);
    ref = normalize(ref);

    float4 skyboxCol = skybox.Sample(fullscreenSampler, ref);

    float causticStrength = voronoiEX(refractedPos.xz, currentTime, function);
    causticStrength *= saturate(distanceToSample);
    causticStrength = causticStrength * causticStrength;

    existing.rgb = lerp(existing.rgb, causticColour, causticStrength * causticStrengthFactor);

    if (distanceToSample)
    {

        float foamTime = currentTime * 0.2f;

        float foamNoise = snoise(float2(sin(foamTime), cos(foamTime)) + scaledUV * 100.0f);
	    foamNoise = saturate(foamNoise);

        float foam = 2.0f;

        float maxFoamDist = 2.0f;
        float foamDist = 1.0f - saturate(distanceToSample / maxFoamDist);
        foamDist *= foamDist;
        foamDist *= foamDist;

        float foamFactor = saturate(foam * foamDist);


		existing.rgb = lerp(
			existing.rgb,
			foamColour.rgb,
			saturate(max(foamFactor, foamFactor*2.0f * foamNoise / pow(distanceToSample * 1.5f, 4))) * foamStrengthFactor
		);
        
    }


    //existing *= float4(0.85f, 0.85f, nosie, 1.0f);

    //existing.rgb += skyboxCol.rgb * distanceToSample * 0.1f;


    return float4(existing.rgb, 1.0f);
    //return float4(noiseTest, noiseTest, noiseTest, 1.0f);
}

