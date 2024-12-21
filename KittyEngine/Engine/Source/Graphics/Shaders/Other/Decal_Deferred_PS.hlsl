#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"

//gbuffer texture
Texture2D gWorldPosTex : register(t0);
Texture2D gAlbedoTex :   register(t1);
Texture2D gNormalTex :   register(t2);
Texture2D gMaterialTex : register(t3);
Texture2D gEffectsTex :  register(t4);
Texture2D gOATex :       register(t5);
Texture2D gDepthTex :    register(t6);

//material texture
Texture2D colourTex :    register(t8);
Texture2D normalTex :    register(t9);
Texture2D materialTex :  register(t10);
Texture2D effectsTex :   register(t11);

cbuffer DecalTransform : register(b1)
{
    matrix objectToWorld;
    matrix worldToObject;
    float4 textureIntensities;
}

//cbuffer DecalBuffer : register(b11)
//{
//    float4x4 viewMatrix;
//    float4x4 projectionMatrix;
//}

struct GBufferOutput
{
    float4 worldPosition : SV_TARGET0;
    float4 albedo : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_TARGET3;
    float4 effects : SV_TARGET4;
    float4 ambientOcclusionAndCustom : SV_TARGET5;
    //float4 depth : SV_TARGET6;
};

struct DecalPixelInput
{
    float4 worldPos : POSITION;
    float4 viewPos : VIEWPOSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;
};

GBufferOutput main(DecalPixelInput aInput)
{
    GBufferOutput output;
   
    //calculate uv to use in gbuffer textures, such that the screen position of this pixel samples the same texel in the gbuffer textures
    float2 depthUV = aInput.position.xy / float2(clientResolution.x, clientResolution.y);
    depthUV += float2(0.5f / clientResolution.x, 0.5f / clientResolution.y);

    float4 worldPos = gWorldPosTex.SampleLevel(defaultSampler, depthUV, 0);


    float4 worldObjectPos = mul(worldToObject, worldPos);
    float2 decalUV = worldObjectPos.xy;
    decalUV += float2(0.5f, 0.5f);
    decalUV.y = 1.0f - decalUV.y;
    output.worldPosition = worldPos;

    float4 tex = colourTex.Sample(defaultSampler, decalUV).rgba;
    if (tex.a == 0.0f)
    {
        discard;
    }

    float4 gAlbedo = gAlbedoTex.SampleLevel(defaultSampler, depthUV, 0);
    float4 gMaterial = gMaterialTex.SampleLevel(defaultSampler, depthUV, 0);
    float4 gEffects = gEffectsTex.SampleLevel(defaultSampler, depthUV, 0);

    float3 decalNormal = (normalTex.Sample(defaultSampler, decalUV).rgb);
    decalNormal = 2.0f * decalNormal - 1.0f;
    decalNormal.z = sqrt(1 - saturate(decalNormal.x * decalNormal.x + decalNormal.y * decalNormal.y));
    decalNormal = normalize(decalNormal);

    float4 decalMaterial = materialTex.Sample(defaultSampler, decalUV).rgba;
    float4 decalEffects = effectsTex.Sample(defaultSampler, decalUV).rgba;

    const float pixelScale = 1.0f;
    const float pixelToUVX = pixelScale * 1.0f / clientResolution.x;
    const float pixelToUVY = pixelScale * 1.0f / clientResolution.y;

    float4 gNormal = 2.0f * gNormalTex.Sample(defaultSampler, depthUV) - 1.0f;

    float3 worldPosOffsetX = gWorldPosTex.Sample(defaultSampler, depthUV + float2(pixelToUVX, 0.0f)).rgb;
    float3 worldPosOffsetY = gWorldPosTex.Sample(defaultSampler, depthUV + float2(0.0f, pixelToUVY)).rgb;

    float3 xDiff = worldPosOffsetX - worldPos.xyz;
    float3 yDiff = worldPosOffsetY - worldPos.xyz;

    float3 evaluatedTangent = normalize(xDiff);
    float3 evaluatedBitangent = normalize(yDiff);

    //convert decal normal from tangent space to world space
    float3x3 TBN = float3x3(
        evaluatedTangent.xyz,
        -evaluatedBitangent.xyz,
        gNormal.xyz
    );
    TBN = transpose(TBN);
    float3 pixelNormal = normalize(mul(TBN, decalNormal));
    pixelNormal = 0.5f + 0.5f * pixelNormal;
    float3 resultNormal = pixelNormal;//normalize(pixelNormal + gNormal.xyz);

    if (
        0.5f - abs(worldObjectPos.x) < 0 ||
        0.5f - abs(worldObjectPos.y) < 0 ||
        0.5f - abs(worldObjectPos.z) < 0 ||
        worldPos.w == 0.0f
    )
    {
    	discard;
    }


    float4 resultAlbedo = lerp(gAlbedo, tex, tex.a);
    output.albedo = resultAlbedo;
    output.normal = float4(resultNormal, 1.0f);
    output.material = gMaterial;
    output.effects = gEffects;
    output.ambientOcclusionAndCustom = float4(0.0f, 0.0f, 0.0f, 1.0f);

    return output;
}
