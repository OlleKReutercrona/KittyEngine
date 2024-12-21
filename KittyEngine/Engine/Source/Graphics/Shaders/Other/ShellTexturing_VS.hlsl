#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

cbuffer ShellTexBuf : register(b7)
{
    matrix shellViewMatrix;
    matrix shellProjectionMatrix;
    matrix modelToWorld;

    float totalHeight;
    int shellCount;
    float thickness;
    float density;
    
    float noiseMin;
    float noiseMax;
    float aoExp;

    float padding;

    float3 bottomColour;
    int shellIndexOffset;

    float3 topColour;
    int unused; //used by the CPU but not the GPU

    float2 UVMin;
    float2 UVMax;
}

struct ShellVertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;

    float3 normal : NORMAL;
    float3 tan : TANGENT;
    float3 bitan : BITANGENT;

    uint instanceID : SV_InstanceID;
};

PixelInput main(const ShellVertexInput aInput)
{
    PixelInput output;

    const float3x3 objectToWorldRotation = Convert4x4To3x3(modelToWorld);

    int shellIndex = aInput.instanceID + shellIndexOffset;

    const float offset = totalHeight / float(shellCount);

    float4 pos = aInput.position;
    pos.xyz += float4(aInput.normal, 0.0f) * offset * shellIndex;

    float3 velocity = float3(0.0f, 0.0f, 0.0f);

    const float scaledTime = currentTime * 0.5f;

    float3 wind = float3(1.0f, 0.0f, cos(scaledTime));


	//blend in the normal too so that it curves
    //velocity = lerp(velocity, -aInput.normal, 0.1f);

    //apply some gravity

    float shellProgress = float(shellIndex) / float(shellCount);

    const float gravity = 9.8f;
    const float gravityScale = 0.015f;
    
    float3 gravityEffect = float3(0.0f, -1.0f, 0.0f);
    gravityEffect *= gravity * gravityScale;
    
    //use the position to make the wind less uniform
    float positionFactor = pos.x * 100.0f + pos.z * 100.0f;
    
    const float windScalar = sin(scaledTime + positionFactor) * 0.005f;
    velocity += wind * windScalar;
    
    //velocity = normalize(velocity);
    velocity += gravityEffect;
    
    pos.xyz += float4(velocity * 0.5f * pow(shellProgress, 2), 0.0f);

    matrix worldToClip = mul(shellProjectionMatrix, shellViewMatrix);

    output.worldPos = mul(modelToWorld, pos);
    output.position = mul(worldToClip, output.worldPos);
    output.normal = mul(objectToWorldRotation, aInput.normal);
    output.tangent = mul(objectToWorldRotation, aInput.tan);
    output.bitan = mul(objectToWorldRotation, aInput.bitan);
    output.texCoord = aInput.texCoord;
    output.attributes = float4(float(shellIndex), 0.0f, 1.0f, 1.0f);

    return output;
}