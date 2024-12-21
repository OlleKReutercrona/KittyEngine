#include "Commons/common.hlsli"

cbuffer Transform : register(b1)
{
    matrix worldToClip;
}

struct InstancedVertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;

    float4x4 transform : INSTANCE_TRANSFORM;
    float4 instanceAttributes : INSTANCE_ATTRIBUTES;
};

struct InstancedPixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;
};

InstancedPixelInput main(const InstancedVertexInput aInput)
{
    InstancedPixelInput output;

    const float3x3 objectToWorldRotation = float3x3(aInput.transform._11, aInput.transform._12, aInput.transform._13,
                                                    aInput.transform._21, aInput.transform._22, aInput.transform._23,
                                                    aInput.transform._31, aInput.transform._32, aInput.transform._33);
                                                    
    float4 pos = aInput.position;
    if (aInput.instanceAttributes.y == 1.0f)
    {
        pos += float4(sin(currentTime + aInput.transform._14), 0,cos(currentTime),0) * pos.y * 0.05f;
    }

    float4 worldPos = mul(aInput.transform, pos);
    
    output.worldPos = worldPos;

    output.position = mul(worldToClip, worldPos);


    output.texCoord = aInput.texCoord;
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.attributes = aInput.instanceAttributes;

    return output;
}