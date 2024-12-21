//#define NUMBER_OF_LIGHTS_ALLOWED 8 
//#define MAX_ANIMATION_BONES 64 
//#define USE_LIGHTS
//#define USE_NOISE

#define PI 3.14159265358979323846f

cbuffer CommonBuffer : register(b0)
{
    float4x4 worldToClipSpaceMatrix;
    float4x4 clipToWorldSpaceMatrix;
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
    float4x4 projMatrixInverse;
    float4x4 viewMatrixInverse;
    float4 cameraPosition;
    uint2 clientResolution;
    float currentTime;
    float deltaTime;
    float2 nearFarPlane;
    float2 PADDING2;
}

SamplerState defaultSampler : register(s0);
SamplerState fullscreenSampler : register(s2);
SamplerState shadowSampler : register(s3);
SamplerState SSAOSampler : register(s4);
SamplerState WrappingSampler : register(s5);
SamplerState bilinearSampler : register(s7);
SamplerState pointSampler : register(s8);

float2 UVtoNDC(const float2 uv)
{
    return (uv * 2.0f - 1.0f) * float2(1, -1);
}

float4 WorldPositionFromDepth(const float depth, float2 uv)
{
    float4 clipSpacePos = float4(UVtoNDC(uv), depth, 1.0f);
    
    float4 viewSpacePosition = mul(projMatrixInverse, clipSpacePos);
    
    viewSpacePosition /= viewSpacePosition.w;
    
    return mul(viewMatrixInverse, viewSpacePosition);
}

float4 UnpackNormal(float4 normal)
{
    float3 workingNormal = float3(normal.rg, 1.0f);

    workingNormal = 2.0f * workingNormal - 1.0f;
    workingNormal.z = sqrt(1 - saturate(workingNormal.x * workingNormal.x + workingNormal.y * workingNormal.y));
    workingNormal = normalize(workingNormal);
    return float4(workingNormal, 1.0f);
}

float UnpackNormalZ(const float x, const float y)
{
    return sqrt(1.0f - saturate(x * x + y * y));
}

int GetNumMips(TextureCube cubeTex)
{
    int iWidth = 0;
    int iheight = 0;
    int numMips = 0;
    cubeTex.GetDimensions(0, iWidth, iheight, numMips);
    return numMips;
}

float3x3 Convert4x4To3x3(const float4x4 aMatrix)
{
    return float3x3(aMatrix._11, aMatrix._12, aMatrix._13,
                    aMatrix._21, aMatrix._22, aMatrix._23,
                    aMatrix._31, aMatrix._32, aMatrix._33);
}