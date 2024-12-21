//RWTexture2D<float4> gWorldPositionTexture :     register(t0);
//RWTexture2D<float4> gNormalTexture :            register(t1);
//RWTexture2D<float4> gAmbientOcclusionTexture :  register(t2);
//RWTexture2D<float4> depthTexture :              register(t3);
//RWTexture2D<float4> normalTexture :             register(t4);

Texture2D gWorldPositionTexture     : register(t0);
Texture2D gNormalTexture            : register(t1);
Texture2D gAmbientOcclusionTexture  : register(t2);
Texture2D depthTexture              : register(t3);
Texture2D normalTexture             : register(t4);

RWTexture2D<float4> outputTexture : register(u5);

#define NUM_SSAO_SAMPLES 16

cbuffer SSAOdata : register(b0)
{
    float4 SSAOSamples[NUM_SSAO_SAMPLES];
    
    float SSAORadius;
    int SSAONumOfSamples;
    int2 clientResolution;
    
    float4x4 worldToClipMatrix;
};

//StructuredBuffer<SSAOdata> mySSAOBufer : register(t6);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}