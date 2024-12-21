#include "Commons/Light.hlsli"
#include "Commons/PBRFunctions.hlsli"

Texture2D colourTex : register(t0);

cbuffer Transform : register(b1)
{
    matrix objectToWorld;
    matrix objectToClip;
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

MultiPixelOutput main(VFXPixelInput aInput)
{
    MultiPixelOutput output;

    if (abs(aInput.normal.y) > 0.0f)
    {
        discard;
    }

    float4 outCol = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float distToEdgeX = 0.05f;
    float distToEdgeY = 0.05f;

    //float xScale = abs(aInput.maxWorldPos.x - aInput.centerWorldPos.x);
    //float yScale = abs(aInput.maxWorldPos.z - aInput.centerWorldPos.z);
    float yScale = length(objectToWorld._21_22_23);

    const float3x3 objectToWorldRotation = float3x3(
		normalize(float3(objectToWorld._11, objectToWorld._12, objectToWorld._13)),
        normalize(float3(objectToWorld._21, objectToWorld._22, objectToWorld._23)),
        normalize(float3(objectToWorld._31, objectToWorld._32, objectToWorld._33))
    );

    //distToEdgeX /= xScale;
    //distToEdgeY /= yScale;
    //
    //float dotSize = 0.05f;
    //
    //
    //float dotCountX = xScale / (dotSize);
    //float dotCountY = yScale / (dotSize);
    //
    //bool onEdgeX = false;
    //bool onEdgeY = false;
    //if (aInput.texCoord.x < distToEdgeX || aInput.texCoord.x > 1.0f - distToEdgeX)
    //{
    //    onEdgeX = true;
    //}
    //if (aInput.texCoord.y < distToEdgeY || aInput.texCoord.y > 1.0f - distToEdgeY)
    //{
    //    onEdgeY = true;
    //}
    //
    //outCol = onEdgeX || onEdgeY;
    //
    //float inDot = 0.0f;
    //if (onEdgeX)
    //{
    //    inDot = inDot + sin(sin(currentTime) + aInput.texCoord.y * dotCountY);
    //}
    //if (onEdgeY)
    //{
	//	inDot = inDot + sin(sin(currentTime) + -aInput.texCoord.x * dotCountX);
    //}

    //outCol = inDot;
    //outCol.rgb = color.rgb * outCol.rgb;
    //outCol.a = inDot;


    float3 testDir = normalize(mul(objectToWorldRotation, float3(0.0f, 0.0f, 1.0f)));

    float dir = sign(dot(testDir.xyz, aInput.normal) + 0.001f);

    float lineSteepness = 20.0f;
    float lineFUCK = 200.0f;
    
    float scrollTime = currentTime * 1.0f;
    
    float horiz = sin(
		((aInput.texCoord.x * PI) * lineFUCK) +
		((aInput.texCoord.y * PI) * lineSteepness) +
		(scrollTime * dir) * PI
    );

    float alpha = (horiz) * (horiz);

    alpha -= pow(aInput.vertexPos.y * 2.0f, 10);

    outCol.rgba = lerp(
		float4(0.0f, 0.0f, 0.0f, 0.0f),
		float4(0.2f, 0.2f, 0.3f, 1.0f),
		alpha
    );
    



    output.color1 = outCol;
    output.color2 = 0;

    return output;
}