#include "Commons/DeferredCommons.hlsli"
#include "Commons/common.hlsli"

cbuffer DeferredVertexConstantBuffer : register(b2)
{
    float4 lightPosAndRange;
    bool isDirectional;
};

DeferredVertexToPixel main(DeferredVertexInput aInput)
{
    DeferredVertexToPixel output;

    output.position = aInput.position;

    if (!isDirectional)
    {
        float4x4 objectToWorld = float4x4(
        lightPosAndRange.w * 1.1f, 0, 0, lightPosAndRange.x,
        0, lightPosAndRange.w * 1.1f, 0, lightPosAndRange.y,
        0, 0, lightPosAndRange.w * 1.1f, lightPosAndRange.z,
        0, 0, 0, 1
        );

        output.position = mul(objectToWorld, aInput.position);
        //const float4 pos = aInput.position;// * lightPosAndRange.w;
        //output.position = pos + float4(lightPosAndRange.xyz, 0.0f);
    	output.position = mul(worldToClipSpaceMatrix, output.position);
    }

	return output;
}