#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
Texture2D effectsTex : register(t3);

float4 main(PixelInput aInput) : SV_Target0
{
    float2 scaledUV = aInput.texCoord;

    float3 directionalLight = { 0.0f, 0.0f, 0.0f };
    float4 albedo = colourTex.Sample(defaultSampler, scaledUV).rgba;
    float3 normal = float3(normalTex.Sample(defaultSampler, scaledUV).rg, 1.0f);

    if (albedo.a < 0.1f)
    {
        discard;
    }

    //float3 normal = normalTex.Sample(defaultSampler, scaledUV).wyz;
    //float ambientOcclusion = normal.z;
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

    float metalness = 0.0f;
    float roughness = 0.0f;
    //float emissive = 0.0f;

    roughness = material.g;
    metalness = material.b;

    float3 specular = lerp((float3) 0.04f, albedo.rgb, metalness);
    float3 col = lerp((float3) 0.0f, albedo.rgb, 1 - metalness);

    const float3 toEye = normalize(cameraPosition.xyz - aInput.worldPos.xyz);

    const float3 lightCol = float3(1.0f,1.0f,1.0f);
    const float3 lightDir = normalize(float3(-1.0f, 1.0f, -1.0f));
    const float lightIntensity = 5.0f;

    // Directional light
    directionalLight = EvaluateDirectionalLight(
        col, 
        specular, 
        pixelNormal,
        roughness, 
        lightCol, 
        lightDir, toEye
    ) * lightIntensity;

    float4 output;
    output = float4(saturate(directionalLight),1.0f);
    
    return output;
}