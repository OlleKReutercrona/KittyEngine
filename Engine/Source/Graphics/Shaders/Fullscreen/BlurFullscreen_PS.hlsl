#include "Commons/common.hlsli"
#include "Commons/DeferredCommons.hlsli"
#include "Commons/InputCommons.hlsli"


float4 main(PixelInput aInput) : SV_TARGET
{
    float2 textureSize = float2(0.0f, 0.0f);
    float mipsOut = 0.0f;
    
    SSAOTexture.GetDimensions(0, textureSize.x, textureSize.y, mipsOut);
    
    float2 texelSize = 1.0f / clientResolution;
    
    float result;
    
    const int numLoops = 4;

    for (int y = -numLoops / 2; y < numLoops / 2; ++y)
    {
        for (int x = -numLoops / 2; x < numLoops / 2; ++x)
        {
            float2 offset = float2(float(x), float(y)) * texelSize;
            result += SSAOTexture.Sample(SSAOSampler, aInput.texCoord + offset).r;
        }
    }
    
    // 16 = the number of loops above
    result = result / (numLoops * numLoops);
    
    return float4(result.rrr, 1.0f);
}
