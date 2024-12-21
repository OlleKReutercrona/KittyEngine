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

GBufferOutput main(PixelInput aInput)
{
    float2 scaledUV = aInput.texCoord;
    scaledUV.x = scaledUV.x * aInput.attributes.z;
    scaledUV.y = scaledUV.y * aInput.attributes.w;
    
    float waterSpeed = 0.005f;
    
    const float3 toEye = normalize(cameraPosition.xyz - aInput.worldPos.xyz);
    const float dist = abs(dot(toEye, viewMatrix[2].xyz));
    const float2 p = aInput.worldPos.xz;

    float2 k0 = float2(3.0f, 5.0f);
    float2 k1 = float2(-4.0f, 5.0f);
    const float A = 0.002f;
    
    const float2 heightDerivative = k0 * sin(dot(p, k0) + currentTime) + k1 * sin(dot(p, k1) + currentTime);
    const float2 maxValue = float2(0.f, length(k0) + length(k1));
    const float2 offset = A * (maxValue + heightDerivative) / dist;
    
    scaledUV += float2(sin(currentTime * waterSpeed), cos(currentTime * waterSpeed)) + offset;
    
    float4 albedo = colourTex.Sample(defaultSampler, scaledUV).rgba;
    
    float waterHeight = aInput.worldPos.y;
    float foamDist = -4.87f;
    
    if (waterHeight > foamDist)
    {
        float3 white = float3(1, 1, 1);
        float alpha = smoothstep(0.0f, 0.05f, abs(waterHeight - foamDist));
        albedo += float4(white, 1) * alpha;
    }
    
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

    GBufferOutput output;
    output.worldPosition = aInput.worldPos;
    output.albedo = float4(albedo.rgb, 1.0f);
    output.normal = float4(0.5f + 0.5f * pixelNormal, 1.0f);
    output.material = float4(material.rgb, 1.0f);

    output.ambientOcclusionAndCustom = float4(ambientOcclusion, float3(0.5f + 0.5f * aInput.normal.xyz));
    output.effects = effectsTex.Sample(defaultSampler, scaledUV).rgba;
    return output;
}

