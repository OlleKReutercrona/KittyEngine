
#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D oldDisplacement : register(t4);
Texture2D bakedDisplacement : register(t5);

float3 PackNormal(float3 normal)
{
    return normal * 0.5f + 0.5f;
}

float4 main(PixelInput aInput) : SV_TARGET
{
    
    float2 screenBasedUV = (aInput.position / float2(1024, 1024));
    float4 baked = bakedDisplacement.Sample(defaultSampler, screenBasedUV).rgba;
    baked.r = 0.0f;
    float4 existing = max(
		oldDisplacement.Sample(defaultSampler, screenBasedUV),
		baked
    );


    //
    const float regrowthRate = 0.01f; 


    float4 col = float4(0.0f, 0.0f, 0.0f, 1.0f);

    col.r = existing.r * (1.0f - deltaTime * regrowthRate);


    return existing;

}

