#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

cbuffer Transform : register(b1)
{
    matrix objectToWorld;
    matrix objectToClip;
}

struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
};

PixelInput main(const VertexInput aInput)
{
    PixelInput output;

    const float3x3 objectToWorldRotation = Convert4x4To3x3(objectToWorld);

    float4 pos = float4(aInput.position.xyz, 0.0f);

    output.worldPos = aInput.position;
    output.position = mul(objectToClip, pos);
    output.position.z = output.position.w;

    //to reduce overdraw, force "infinitely" far away


    output.texCoord = aInput.texCoord;
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.attributes = float4(0.0f,0.0f,1.0f,1.0f);

    return output;
}