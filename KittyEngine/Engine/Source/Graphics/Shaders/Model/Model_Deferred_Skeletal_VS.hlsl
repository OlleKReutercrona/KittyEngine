#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

cbuffer Transform : register(b1)
{
    matrix objectToWorld;
    matrix objectToClip;
}

cbuffer SkeletonBuffer : register(b4)
{
    float4x4 boneTransforms[128u]; // Array of bone transforms
};

struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
    uint4 boneIndices : BONEINDICES;
    float4 boneWeights : BONEWEIGHTS;
};

PixelInput main(const VertexInput aInput)
{
    PixelInput output;

	// Accumulate bone transformations based on bone indices and weights
    matrix finalTransform =
    {
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 }
    };


    for (int i = 0; i < 4; ++i)
    {
        const uint boneIndex = aInput.boneIndices[i];
        const float boneWeight = aInput.boneWeights[i];

		// Fetch the bone transform from the skeleton buffer
        const matrix boneTransform = boneTransforms[boneIndex];

        // Accumulate the transformed contribution from each bone
        finalTransform += boneWeight * boneTransform;
    }

    matrix sizeFixTransform = 
    {
        { 0.01f, 0, 0, 0 },
        { 0, 0.01f, 0, 0 },
        { 0, 0, 0.01f, 0 },
        { 0, 0, 0,  1.0f }
    };

    finalTransform = mul(sizeFixTransform, finalTransform);

	// Apply the final bone transform to the vertex position
    float4 worldPosition = mul(finalTransform, aInput.position);

    const float3x3 objectToWorldRotation = Convert4x4To3x3(objectToWorld);
    const float3x3 skinnedRotation = Convert4x4To3x3(finalTransform);

    const float3 vertexWorldNormal = mul(objectToWorldRotation, mul(skinnedRotation, aInput.normal));
    const float3 vertexWorldTangent = mul(objectToWorldRotation, mul(skinnedRotation, aInput.tan));
    const float3 vertexWorldBinormal = mul(objectToWorldRotation, mul(skinnedRotation, aInput.bitan));

    const float4 position = worldPosition;
    output.worldPos = mul(objectToWorld, position);
    output.position = mul(objectToClip, position);
    output.texCoord = aInput.texCoord;
    output.normal = vertexWorldNormal;
    output.tangent = vertexWorldTangent;
    output.bitan = vertexWorldBinormal;
    output.attributes = float4(0,0,1,1);

    return output;
}