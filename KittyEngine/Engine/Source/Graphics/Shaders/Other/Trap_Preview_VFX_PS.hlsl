#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"

cbuffer Transform : register(b1)
{
    matrix objectToWorld;
    matrix objectToClip;
}

Texture2D colourTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D materialTex : register(t2);
Texture2D effectsTex : register(t3);

Texture2D worldPositionTexG : register(t4);
Texture2D colourTexG : register(t5);
Texture2D normalTexG : register(t6);
Texture2D materialTexG : register(t7);
Texture2D effectsTexG : register(t8);
Texture2D ambientOcclusionTexG : register(t9);
Texture2D depthTexG : register(t10);
Texture2D SSAOTextureG : register(t11);

cbuffer VFXInstance : register(b6) //6 is arbitrary, need to sync up with other graphics
{
    float4 color;
    float2 uvScroll;
    float2 uvScale;
    float4 bloomAttributes;
}

struct MultiPixelOutput
{
    float4 color1 : SV_Target0;
    float4 color2 : SV_Target1;
};

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
};

MultiPixelOutput main(VFXPixelInput aInput)
{
    MultiPixelOutput output;
    float2 scaledUV = aInput.texCoord;
    scaledUV.x = scaledUV.x * aInput.attributes.z * uvScale.x;
    scaledUV.y = scaledUV.y * aInput.attributes.w * uvScale.y;

    float4 texture0 = colourTex.Sample(defaultSampler, scaledUV).rgba;

    float4 texture1a = normalTex.Sample(defaultSampler, scaledUV).rgba;

    float4 texture1 = texture1a; // * texture1b * texture1c * texture1d;

    float2 screenTexUV = aInput.position.xy / float2(2560.0f, 1440.0f);

    float2 centerUV = float2(0.5f, 0.5f);

    float4 vertexWorldPos = aInput.worldPos;
    float4 gBufferWorldPos = worldPositionTexG.Sample(defaultSampler, screenTexUV);
    //float4 gBufferCenterWorldPos = worldPositionTexG.Sample(defaultSampler, centerUV);


    const float distanceToGeometry = length(vertexWorldPos.xyz - gBufferWorldPos.xyz);
    const float distanceToCamera = length(vertexWorldPos.xyz - cameraPosition.xyz);
    const float geomToCamera = length(aInput.centerWorldPos.xyz - cameraPosition.xyz);




    //from 0 to 1, where along xyz is vertex compared to min and max world pos
    const float3 normalizedVertex = (vertexWorldPos.xyz - aInput.minWorldPos.xyz) / (aInput.maxWorldPos.xyz - aInput.minWorldPos.xyz);

	const float lineSpeed = 10.0f;
    const float lineFrequency = geomToCamera/50.0f;
	const float verticalLine = aInput.position.y;
    const float lineTime = lineSpeed * currentTime;

    const float drawLine = sin(lineTime + verticalLine * lineFrequency);

    float result = 1.0f;

    const float3 objectUP = abs(aInput.worldUPDir).xyz;

    float2 coolerUV;
    //use objectUP to calculate coolerUV, such that it is:
    //  normalizedVertex.x and normalizedVertex.z if objectUP is 0,1,0
    //  normalizedVertex.x and normalizedVertex.y if objectUP is 0,0,1
    //  normalizedVertex.y and normalizedVertex.z if objectUP is 1,0,0

    if(objectUP.x > objectUP.y && objectUP.x > objectUP.z)
    {
        coolerUV = float2(normalizedVertex.y, normalizedVertex.z);
    }
    else if(objectUP.y > objectUP.x && objectUP.y > objectUP.z)
    {
        coolerUV = float2(normalizedVertex.x, normalizedVertex.z);
    }
    else
    {
        coolerUV = float2(normalizedVertex.y, normalizedVertex.x);
    }

    coolerUV = coolerUV * 0.5f + 0.25f;

    float uvBasedLine = sin(coolerUV.x * 75.0f + lineTime);
    if (uvBasedLine < 0.0f)
    {
        uvBasedLine = 0.0f;
    }

    const float geomHighlight = distanceToGeometry < 0.05f ? 1.0f : 0.0f;

    output.color1 = color * clamp((uvBasedLine + geomHighlight), 0, 1);
    output.color2 = output.color1 * 0.25f;
        
    return output; 
}