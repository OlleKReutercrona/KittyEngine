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

struct WaterPixelInput
{
    float4 worldPos : POSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 clipPos: TEXCOORD1;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;
};

float2 hash23(float3 p3)
{
    p3 = frac(p3 * float3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.xx + p3.yz) * p3.zy);
}

WaterPixelInput main(const VertexInput aInput)
{
    WaterPixelInput output;

    const float3x3 objectToWorldRotation = float3x3(objectToWorld._11, objectToWorld._12, objectToWorld._13,
                                                    objectToWorld._21, objectToWorld._22, objectToWorld._23,
                                                    objectToWorld._31, objectToWorld._32, objectToWorld._33);
                                                    
    float4 finalVertexPosition = aInput.position;

    float2 random = hash23(finalVertexPosition);

    float time = currentTime * 0.5f;
    finalVertexPosition.y += sin(time + random.x * 10.0f) * 0.5f * sin(time + random.y * 10.0f) * 0.5f;


    output.worldPos = mul(objectToWorld, finalVertexPosition);
    output.position = mul(objectToClip, finalVertexPosition);
    output.clipPos =  mul(objectToClip, finalVertexPosition);

    output.texCoord = aInput.texCoord;
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.attributes = 0;

    return output;
}