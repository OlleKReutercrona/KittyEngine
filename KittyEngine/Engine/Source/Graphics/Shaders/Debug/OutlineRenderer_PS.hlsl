#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);

float4 main(PixelInput aInput) : SV_TARGET
{
    float4 result = colourTex.Sample(fullscreenSampler, aInput.texCoord).rgba;

    float factor = 0.002f;
    factor += sin(currentTime) / 1000.0f;

    bool isFilled = result.a > 0.1f;
    
    bool doDiscard = false;

    if (isFilled)
    {
       doDiscard = true;
    }
    else
    {
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                float2 offset = float2(i, j) * factor;
                float2 uv = aInput.texCoord.xy + offset;
                if (!(uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1))
                {
                    float4 color = colourTex.SampleLevel(fullscreenSampler, uv, 0.0f);;
                    if (color.a > 0.1f)
                    {
                        result = float4(
                            1.0f,
                            1.0f,
                            0.67058823529f,
                            1.0f
                        );
                    }
                }
                
            }
        }
        
        doDiscard = true;
        
    }
    
    if (doDiscard)
    {
        discard;
    }
    return result;
}

