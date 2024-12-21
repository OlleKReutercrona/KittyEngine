#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D depthTexture : register(t1);
Texture2D textureToSample : register(t11);

cbuffer DistFogBuffer : register(b1)
{
    float4 fogColour;
    
    float maxDistance;
    float waterHeight;
    float fadeDistance;
    int power;
};

float4 main(PixelInput input) : SV_TARGET
{
    float3 colour = textureToSample.Sample(defaultSampler, input.texCoord).rgb;

    float depth = depthTexture.Sample(defaultSampler, input.texCoord).r;

    float4 worldPosition = WorldPositionFromDepth(depth, input.texCoord);
    
	// Distance fog
    float contribution = 0.0f;
    
    const float distance = length(worldPosition.rgb - cameraPosition.xyz);
        
    contribution = clamp(distance / maxDistance, 0.0f, 1.0f);
        
    contribution = pow(contribution, power);

    float atmosphereContribution = 1.0f;
    if (depth > 0.9999f)
    {  
        atmosphereContribution = 1.0f - clamp((worldPosition.y - waterHeight) / fadeDistance, 0.0f, 1.0f);

        atmosphereContribution = atmosphereContribution * atmosphereContribution * atmosphereContribution;
        
        if (atmosphereContribution < 0.01f)
        {
            discard;
        }
    }
    float3 colourMix = colour;
    colourMix = lerp(colour, fogColour.rgb, fogColour.a);           // Fade by alpha
    colourMix = lerp(colour, colourMix, contribution);              // Exponential fade
    colourMix = lerp(colour, colourMix, atmosphereContribution);    // Fade horizon
        

    float3 finalColour = colourMix;
    
    return float4(finalColour, 1.0f);
}