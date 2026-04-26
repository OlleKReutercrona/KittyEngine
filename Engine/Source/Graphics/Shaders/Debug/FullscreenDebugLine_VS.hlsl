#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

DebugLinePSInputType main(DebugLineVSInputType input)
{
    DebugLinePSInputType output;
    
    output.position = input.position;
    output.color = input.color;
    
    return output;
}