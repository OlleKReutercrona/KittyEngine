
static const uint shadowAtlasResolution = 8192;
static const float shadowMinClamp = 0.3f;

Texture2D shadowAtlas : register(t14);
SamplerState shadowSampler : register(s3);

void UnpackShadowData(const uint aShadowMapInformation, out float2 startUV, out float2 endUV)
{
    const uint sizeMask = 15;
    const uint topXMask = 262128;
    const uint topYMask = 4294705152;
    
    uint sizeIndex = (aShadowMapInformation & sizeMask);
    uint topX = (aShadowMapInformation & topXMask) >> 4;
    uint topY = (aShadowMapInformation & topYMask) >> 18;
    
    uint size = 128 * pow(2, sizeIndex);
    
    float floatRes = float(shadowAtlasResolution);
    
    startUV = float2((float(topX) / floatRes), float(topY) / floatRes);
    endUV = float2((float(topX + size) / floatRes), float(topY + size) / floatRes);
}

float CalculateShadow(const int shadowMapInformation, const float4x4 cameraTransform, const float3 worldPosition, const float bias, const uint numberOfSamples, const float blurFactor)
{
    float2 shadowMapStartUVCoordinate;
    float2 shadowMapEndUVCoordinate;
    
    UnpackShadowData(shadowMapInformation, shadowMapStartUVCoordinate, shadowMapEndUVCoordinate);
    
    float4 tempProjectedPosition = mul(cameraTransform, float4(worldPosition, 1.0f));
    float3 projectedPosition = tempProjectedPosition.xyz / tempProjectedPosition.w;

    float shadowFactor = 1.0f;
    if (clamp(projectedPosition.x, -1.0f, 1.0f) == projectedPosition.x &&
        clamp(projectedPosition.y, -1.0f, 1.0f) == projectedPosition.y)
    {
        const float computedZ = projectedPosition.z;

        float totalFactor = 0.0f;

        // Filter kernel for [PCF] percentage-closer filtering eg. (3x3)
        const int numSamples = numberOfSamples;
        // Offset scale decides how much the shadow edge is moved for "blurring"
        const float offsetScale = blurFactor;

        for (int i = -numSamples / 2; i <= numSamples / 2; ++i)
        {
            for (int j = -numSamples / 2; j <= numSamples / 2; ++j)
            {
                const float2 sampleOffset = float2(i, j) / float(numSamples);
                const float2 sampleUV = 0.5f + float2(0.5f, -0.5f) * (projectedPosition.xy + sampleOffset * offsetScale);
                
                // Scales the UV to the shadow atlas
                float scaledX = lerp(shadowMapStartUVCoordinate.x, shadowMapEndUVCoordinate.x, sampleUV.x);
                float scaledY = lerp(shadowMapStartUVCoordinate.y, shadowMapEndUVCoordinate.y, sampleUV.y);
                
                const float2 scaledUV = float2(scaledX, scaledY);

                const float shadowMapZ = shadowAtlas.Sample(shadowSampler, scaledUV);

                totalFactor += (computedZ < shadowMapZ + bias) ? 1.0f : 0.0f;
            }
        }

        shadowFactor = totalFactor / float(numSamples * numSamples);
    }
    return shadowFactor;
}

float CalculateShadowNoBlurring(const int shadowMapInformation, const float4x4 cameraTransform, const float3 worldPosition, const float bias)
{
    float2 shadowMapStartUVCoordinate;
    float2 shadowMapEndUVCoordinate;
    
    UnpackShadowData(shadowMapInformation, shadowMapStartUVCoordinate, shadowMapEndUVCoordinate);
    
    float4 undividedPos = mul(cameraTransform, float4(worldPosition, 1.0f));
    float3 projectedPosition = undividedPos.xyz / undividedPos.w;
     
    float shadowFactor = 1.0f;
    
    if (clamp(projectedPosition.x, -1.0f, 1.0f) == projectedPosition.x 
        && clamp(projectedPosition.y, -1.0f, 1.0f) == projectedPosition.y)
    {
        const float computedZ = projectedPosition.z;
        const float bias = 0.0000005f;
        
        const float offsetScale = 0.0005f;
        
        const float2 sampleUV = 0.5f + float2(0.5, -0.5f) * projectedPosition.xy;

        float scaledX = lerp(shadowMapStartUVCoordinate.x, shadowMapEndUVCoordinate.x, sampleUV.x);
        float scaledY = lerp(shadowMapStartUVCoordinate.y, shadowMapEndUVCoordinate.y, sampleUV.y);
                
        const float2 scaledUV = float2(scaledX, scaledY);
        
        float2 shadowTexDimensions = float2(shadowAtlasResolution, shadowAtlasResolution);

        int2 uv = shadowTexDimensions * scaledUV;

        float shadowMapZ = shadowAtlas[uv].r;

        shadowFactor = (computedZ < shadowMapZ + bias) ? 1.0f : 0.0f;
    }
    return shadowFactor;   
}

#define PI 3.14159265358979323846f

float3 EvaluateSpotLight(float3 lightColor, float lightIntensity, float lightRange,
    float3 lightPos, float3 lightDir, float outerAngle, float innerAngle, float3 viewDir, float3 pixelPos)
{
    float3 toLight = lightPos.xyz - pixelPos.xyz;
    float lightDistance = length(toLight);
    toLight = normalize(toLight);

    float cosOuterAngle = cos(outerAngle);
    float cosInnerAngle = cos(innerAngle);
    float3 lightDirection = lightDir;

    // Determine if pixel is within cone.
    float theta = dot(toLight, normalize(-lightDirection));
	// And if we're in the inner or outer radius.
    float epsilon = cosInnerAngle - cosOuterAngle;
    float intensity = clamp((theta - cosOuterAngle) / epsilon, 0.0f, 1.0f);
    intensity *= intensity;

    return saturate(lightColor * intensity);
}