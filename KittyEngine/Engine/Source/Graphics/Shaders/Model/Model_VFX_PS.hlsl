#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"


Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
Texture2D effectsTex : register(t3);

struct VFXPixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;

    float4 centerWorldPos : CENTERWORLDPOS;
    float4 minWorldPos : MINWORLDPOS;
    float4 maxWorldPos : MAXWORLDPOS;

    float4 worldUPDir : WORLDUP;
};

cbuffer VFXInstance : register(b6) //6 is arbitrary, need to sync up with other graphics
{
    float4 color;
    float2 uvScroll;
    float2 uvScale;
    float4 bloomAttributes;
}

struct MultiPixelOutput
{
    float4 color1 : SV_Target0;
    float4 color2 : SV_Target1;
};

MultiPixelOutput main(VFXPixelInput aInput)
{

    float2 scaledUV = aInput.texCoord;
    scaledUV.x = scaledUV.x * aInput.attributes.z * uvScale.x;
    scaledUV.y = scaledUV.y * aInput.attributes.w * uvScale.y;

    float4 albedo = colourTex.Sample(defaultSampler, scaledUV).rgba;
    float3 normal = normalTex.Sample(defaultSampler, scaledUV).wyz;

    float ambientOcclusion = normal.b;
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

    MultiPixelOutput output;

    float4 multipliedColour = float4(color.rgb * color.a, color.a);
    float4 multipliedAlbedo = float4(albedo.rgb, albedo.a);
    float4 col = multipliedAlbedo.rgba * multipliedColour.rgba;
    float4 res = float4(col.rgb, col.a);

    output.color1 = res;
    if (bloomAttributes.x > 0.0f)
    {
        float4 s_multipliedColour = float4(color.rgb * 1.0f, 1.0f);
        float4 s_multipliedAlbedo = float4(albedo.rgb, 1.0f);
        float4 s_col = multipliedAlbedo.rgba * multipliedColour.rgba;
        float4 s_res = float4(s_col.rgb, s_col.a);
        output.color2 = s_res;
    }
    else
    {
        output.color2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    return output; 
}