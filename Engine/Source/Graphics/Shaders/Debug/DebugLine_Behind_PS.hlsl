#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

PixelOutput main(DebugLineVSInputType input)
{
    PixelOutput output;
    
    output.color.xyz = input.color.xyz * 0.2f;
    output.color.a = 1.0f;

    return output;
}