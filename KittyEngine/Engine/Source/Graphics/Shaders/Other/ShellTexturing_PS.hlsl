#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D displacementTex : register(t4);

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

float4 main(PixelInput aInput) : SV_TARGET
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
    
    if (attr.a > 0)
    {
        if ((1.0f - attr.x) < h)
        {
            discard;
        }
        wh += attr.x;
    }
    

    //float3 dispDir = displacement.xyz * 2.0f - 1.0f;

    //float edgeFactor = displacement.a;

    //if (edgeFactor <= 0)
    //{
    //    //discard;
    //}
    //
    //float displacementStrength = pow(edgeFactor, 2.0f) * 0.02f * -h;
    ////float displacementStrength = -h * edgeFactor * 0.1f;
    //
    //
	float2 uv = aInput.texCoord;
    //
    //uv.x += (dispDir.x * displacementStrength);
    //uv.y += (dispDir.y * displacementStrength);

    float2 newUV = uv * density;
    float2 localUV = frac(newUV) * 2 - 1;
    float localDistanceFromCenter = length(localUV);
    uint2 tid = newUV;
    uint seed = tid.x + 100 * tid.y + 100 * 10;

    float rand = lerp(noiseMin, noiseMax, hash(seed));
    int outsideThickness = (localDistanceFromCenter) > (thickness * (rand - wh));

	// This culls the pixel if it is outside the thickness of the strand, it also ensures that the base shell is fully opaque that way there aren't
	// any real holes in the mesh, although there's certainly better ways to do that
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

    
    return float4(col.rgb * ambientOcclusion, 1.0f);
    //return float4(displacementUV, 0.0f, 1.0f);
}

