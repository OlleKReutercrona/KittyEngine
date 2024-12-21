
#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D oldDisplacement : register(t4);

cbuffer DisplacementBuffer : register(b5)
{
    int channelIndex;
    float fade;

    float padding1;
    float padding2;
}

float3 PackNormal(float3 normal)
{
    return normal * 0.5f + 0.5f;
}

float4 main(PixelInput aInput) : SV_TARGET
{
    float3 normal = aInput.normal;
    //normal.y = 0.0f;
    //normal = normalize(normal);
    //normal = PackNormal(normal)



    float2 screenBasedUV = (aInput.position / float2(1024, 1024));
    float4 existing = oldDisplacement.Sample(defaultSampler, screenBasedUV);
    float d = dot(normal, float3(0.0f, 1.0f, 0.0f));

    float3 ex;
    int useChannelIndex = abs(channelIndex);

    float3 channelMask = float3(
		useChannelIndex == 0,
		useChannelIndex == 1,
		useChannelIndex == 2
	);

    if (channelIndex < 0)
    {
        ex = min(channelMask * d, existing.rgb);
    }
    else
    {
		ex = max(channelMask * d, existing.rgb);
	    
    }


	if (ex.r > 1.0f) { ex.r = 1.0f; }
	if (ex.g > 1.0f) { ex.g = 1.0f; }
	if (ex.b > 1.0f) { ex.b = 1.0f; }


    float4 col = float4(ex, 1.0f);
    

    return col;
}

