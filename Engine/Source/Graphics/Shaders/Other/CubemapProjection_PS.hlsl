#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

TextureCube cubeMap : register(t0);

PixelOutput main(PixelInput aInput)
{
    PixelOutput output;

    //start with equirectangular projection

    //const float PI_ = 3.14159265359;
    //float phi = aInput.texCoord.x * 2 * PI_;
    //float theta = aInput.texCoord.y * PI_;
    //float3 dir = float3(cos(phi) * sin(theta), cos(theta), sin(phi) * sin(theta));

    float2 a = aInput.texCoord * float2(PI, 0.5f * PI);
    float2 c = cos(a);
    float2 s = sin(a);

    float3 dir = float3(c.x * s.y, c.y, s.x * s.y);

    float4 color = cubeMap.SampleLevel(fullscreenSampler, dir, 0);

    output.color = color;
    return output;
}