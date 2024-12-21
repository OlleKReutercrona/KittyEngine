#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

PixelOutput main(DebugLineVSInputType input)
{
    PixelOutput output;
    
    output.color = input.color;

    return output;
}