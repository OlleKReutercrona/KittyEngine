#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"
#include "Commons/DeferredCommons.hlsli"

Texture2D displacementTex : register(t4);
Texture2D bakedDisplacement : register(t5);

cbuffer ShellTexBuf : register(b7)
{
    matrix shellViewMatrix;
    matrix shellProjectionMatrix;
    matrix modelToWorld;

    float totalHeight;
    int shellCount;
    float thickness;
    float density;
    
    float noiseMin;
    float noiseMax;
    float aoExp;

    float padding;

    float3 bottomColour;
    int shellIndexOffset;

    float3 topColour;
    int unused; //used by the CPU but not the GPU

    float2 UVMin;
    float2 UVMax;
}

float hash(uint n)
{
	// integer hash copied from Hugo Elias
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0x1376312589U;
    return float(n & uint(0x7fffffffU)) / float(0x7fffffff);
}

struct GBufferOutput
{
    float4 worldPosition : SV_TARGET0;
    float4 albedo : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float4 material : SV_TARGET3;
    float4 effects : SV_TARGET4;
    float4 ambientOcclusionAndCustom : SV_TARGET5;
};

GBufferOutput main(PixelInput aInput) : SV_TARGET
{
    const float shellIndex = aInput.attributes.x;
    const float wShellIndex = shellIndex;
    const float wShellCount = shellCount;

    const float h = wShellIndex / wShellCount;
    float wh = wShellIndex / wShellCount;

    float displacementU = lerp(UVMin.x, UVMax.x, 1.0f - aInput.texCoord.x);
    float displacementV = lerp(1.0f - UVMax.y, 1.0f - UVMin.y, 1.0f - aInput.texCoord.y);

    float2 displacementUV = float2(displacementU, displacementV);

    float4 attr = displacementTex.Sample(fullscreenSampler, displacementUV);
    float4 baked = bakedDisplacement.Sample(defaultSampler, displacementUV).rgba;
    baked.r = 0.0f;

    attr = max(attr, baked);
    
    if (attr.a > 0)
    {
        if (attr.z > 0.0f) { discard; }

        if ((1.0f - attr.x) < h)
        {
            discard;
        }
        if ((1.0f - attr.y) < h)
        {
            discard;
        }
        wh += max(attr.x, attr.y);
    }

	float2 uv = aInput.texCoord;

    float2 newUV = uv * density;
    float2 localUV = frac(newUV) * 2 - 1;
    float localDistanceFromCenter = length(localUV);
    uint2 tid = newUV;
    uint seed = tid.x + 100 * tid.y + 100 * 10;

    float rand = lerp(noiseMin, noiseMax, hash(seed));
    int outsideThickness = (localDistanceFromCenter) > (thickness * (rand - wh));

    float4 directionToMiddle = float4(-localUV.x, 0.0f, localUV.y, 1.0f);
    directionToMiddle = normalize(directionToMiddle);

    float4 up = float4(0.0f, 1.0f, 0.0f, 1.0f);

    float4 normal = lerp(directionToMiddle, up, 1.0f);
    //float4 normal = directionToMiddle;
    //normal.y = lerp(normal.y, up.y, h * 0.5f + 0.5f);
    normal = normalize(normal);


    if (outsideThickness && wShellIndex > 0)
        discard;
				
    float4 bcol = float4(bottomColour.rgb, 1.0f);
    float4 tcol = float4(topColour.rgb, 1.0f);
    float4 col = lerp(bcol, tcol, h);


    float ambientOcclusion = pow(h, aoExp);
    ambientOcclusion = saturate(ambientOcclusion);
    if (shellIndex == 0)
    {
        ambientOcclusion = 1.0f - aoExp * 2.0f;
    }


    GBufferOutput output;

    output.worldPosition = aInput.worldPos;
    output.material = float4(0.0f, 1.0f, 0.0f, 1.0f);
    //output.normal = 0.5f + float4(0.0f, 1.0f, 0.0f, 1.0f) / 2.0f;

    output.normal = 0.5f + normal / 2.0f;

    //output.normal = directionToMiddle;

    output.albedo = float4(col.rgb * ambientOcclusion, 1.0f);
    output.effects = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.ambientOcclusionAndCustom = float4(0.0f, 0.0f, 0.0f, 0.0f);

    return output;

    //return float4(col.rgb * ambientOcclusion, 1.0f);
}

