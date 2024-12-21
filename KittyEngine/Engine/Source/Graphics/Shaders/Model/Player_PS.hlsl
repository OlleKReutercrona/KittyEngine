#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
Texture2D effectsTex : register(t3);

struct GBufferOutput
{
    float4 worldPosition : SV_TARGET0;
    float4 albedo : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_TARGET3;
    float4 effects : SV_TARGET4;
    float4 ambientOcclusionAndCustom : SV_TARGET5;
};

cbuffer PlayerRenderBuffer : register(b7)
{
    float timeInvulnerable;
    float timeOutsideBattleArea;

    float padding1;
    float padding2;
};

GBufferOutput main(PixelInput aInput)
{
    float2 scaledUV = aInput.texCoord;
    scaledUV.x = scaledUV.x * aInput.attributes.z;
    scaledUV.y = scaledUV.y * aInput.attributes.w;
    float4 albedo = colourTex.Sample(defaultSampler, scaledUV).rgba;

    if (albedo.a < 0.5f)
    {
        discard;
    }

    float3 normal = float3(normalTex.Sample(defaultSampler, scaledUV).rg, 1.0f);

    normal = 2.0f * normal - 1.0f;
    normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
    normal = normalize(normal);


    float3x3 TBN = float3x3(
		normalize(aInput.tangent.xyz),
		normalize(-aInput.bitan.xyz),
		normalize(aInput.normal.xyz)
	);
    TBN = transpose(TBN);
    const float3 pixelNormal = normalize(mul(TBN, normal));

    float4 material = materialTex.Sample(defaultSampler, scaledUV);
    float ambientOcclusion = material.r;


    if (timeOutsideBattleArea > 0.0f)
    {
	    float outsideAreaTime = timeOutsideBattleArea / 3.0f;

        float progressToDie = 1.0f - saturate(outsideAreaTime);
        float s = sin(pow(progressToDie * 7.0f, 1.5f));
        s = pow(s, 2.0f);

        if (s > 0.0f)
        {
            albedo.rgb = lerp(albedo.rgb, float3(0.5f, 0.05f, 0.1f), s / 2.0f);
        }
    }

    //if (timeInvulnerable >= 0.0f)
    //{
    //    float invulnerableTimer = timeInvulnerable / 1.0f;
    //
    //    float invulnerablePulse = sin(invulnerableTimer * 15.0f);
    //    if (invulnerablePulse > 0.0f)
    //    {
    //        albedo.rgb = lerp(albedo.rgb, float3(0.2f, 0.2f, 0.2f), invulnerablePulse / 2.0f);
    //    }
    //}


    GBufferOutput output;
    output.worldPosition = aInput.worldPos;
    output.albedo = float4(albedo.rgb / albedo.a, 1.0f);
    output.normal = float4(0.5f + 0.5f * pixelNormal, 1.0f);
    output.material = float4(material.rgb, 1.0f);

    output.ambientOcclusionAndCustom = float4(ambientOcclusion, float3(0.5f + 0.5f * aInput.normal.xyz));
    output.effects = effectsTex.Sample(defaultSampler, scaledUV).rgba;
    output.effects = 0.25f;//effectsTex.Sample(defaultSampler, scaledUV).rgba;
    return output;
}

