#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

cbuffer Transform : register(b0)
{
    matrix model;
    matrix modelViewProj;
}

struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float4 normal : NORMAL;
    float4 tan : TANGENT;
    float4 bitan : BITANGENT;
};

PixelInput main(const VertexInput aInput)
{
    PixelInput output;

    output.position = aInput.position;

    //we can use worldPos as a variable storing the direction of the ray from the camera to the pixel
    output.worldPos = aInput.position;
    //output.position = mul(modelViewProj, aInput.position);
    
    output.texCoord = aInput.texCoord;

    output.normal = aInput.normal.xyz;
    output.tangent = aInput.tan.xyz;
    output.bitan = aInput.bitan.xyz;
    
    output.attributes = float4(0.0f,0.0f,0.0f,0.0f);
    
    return output;
}