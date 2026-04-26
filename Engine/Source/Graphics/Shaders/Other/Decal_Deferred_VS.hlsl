cbuffer DecalTransform : register(b1)
{
    matrix objectToWorld;
    matrix worldToObject;
    float4 textureIntensities;
}

cbuffer DecalBuffer : register(b11)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
}

struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;
};

struct DecalPixelInput
{
    float4 worldPos : POSITION;
    float4 viewPos : VIEWPOSITION;
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitan : BITANGENT;
    float4 attributes : ATTRIBUTES;
};



DecalPixelInput main(const VertexInput aInput)
{
    DecalPixelInput output;

    const float3x3 objectToWorldRotation = float3x3(objectToWorld._11, objectToWorld._12, objectToWorld._13,
                                                    objectToWorld._21, objectToWorld._22, objectToWorld._23,
                                                    objectToWorld._31, objectToWorld._32, objectToWorld._33);

    output.worldPos = mul(objectToWorld, aInput.position);
    
    output.viewPos = mul(viewMatrix, output.worldPos);
    output.position = mul(projectionMatrix, output.viewPos);


    output.texCoord = aInput.texCoord;
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.attributes = float4(0.0f,0.0f,1.0f,1.0f);

    return output;
}