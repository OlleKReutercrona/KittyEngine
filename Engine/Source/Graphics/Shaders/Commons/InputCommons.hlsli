struct PixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;
};

// ANIMATION STUFF
#define KE_MAX_BONES_IN_ARMATURE (128)

// Fat cbuffer.
cbuffer AnimationCBuffer : register(b4)
{
    float4x4 resultingMatrices[KE_MAX_BONES_IN_ARMATURE];
}

struct SkeletalVertexInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    
    float4 boneWeights : BONEWEIGHTS;
    int4 boneIndices : BONEINDICES;
};

// ANIMATION STUFF END

struct DebugLineVSInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct DebugLinePSInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct PixelOutput
{
    float4 color : SV_TARGET;
};

struct PostProcessVertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PostProcessVertexInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct SSAOPixelInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};