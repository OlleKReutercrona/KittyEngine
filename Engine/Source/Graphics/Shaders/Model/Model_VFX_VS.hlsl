#include "Commons/common.hlsli"

cbuffer Transform : register(b1)
{
    matrix objectToWorld;
    matrix objectToClip;
}

cbuffer VFXInstance : register(b6) //6 is arbitrary, need to sync up with other graphics
{
    float4 color;
    float2 uvScroll;
    float2 uvScale;
    float4 bloomAttributes;
}


struct VFXPixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;

    float4 centerWorldPos : CENTERWORLDPOS;
    float4 minWorldPos : MINWORLDPOS;
    float4 maxWorldPos : MAXWORLDPOS;

    float4 worldUPDir : WORLDUP;
    float4 vertexPos : VERTEXPOS;
};

struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
};

VFXPixelInput main(const VertexInput aInput)
{
    VFXPixelInput output;

    const float3x3 objectToWorldRotation = float3x3(objectToWorld._11, objectToWorld._12, objectToWorld._13,
                                                    objectToWorld._21, objectToWorld._22, objectToWorld._23,
                                                    objectToWorld._31, objectToWorld._32, objectToWorld._33);

    output.centerWorldPos = mul(objectToWorld, float4(0.0f, 0.0f, 0.0f, 1.0f));
    output.minWorldPos = mul(objectToWorld, float4(-0.5f, -0.5f, -0.5f, 1.0f));
    output.maxWorldPos = mul(objectToWorld, float4(0.5f, 0.5f, 0.5f, 1.0f));

    output.worldPos = mul(objectToWorld, aInput.position);
    output.position = mul(objectToClip, aInput.position);
    
    output.texCoord = aInput.texCoord + uvScroll;

    output.worldUPDir = mul(objectToWorld, float4(0.0f, 1.0f, 0.0f, 0.0f));

    output.normal = mul(objectToWorldRotation, aInput.normal);
    //output.vNormal = aInput.normal;
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.attributes = float4(0.0f,0.0f,1.0f,1.0f);
    output.vertexPos = aInput.position;

    return output;
}