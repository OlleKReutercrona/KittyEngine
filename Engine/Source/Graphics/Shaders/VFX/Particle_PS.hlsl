#include "Commons/common.hlsli"
#include "Commons/spriteCommon.hlsli"

Texture2D cool : register(t0);

float4 main(SpritePixelInput input) : SV_TARGET
{
    float4 pos = input.position;
    float2 uv = input.uv;

    if (flipX)
    {
        uv.x = 1.0f - uv.x;
    }

    if (flipY)
    {
        uv.y = 1.0f - uv.y;
    }


    if (uv.x < input.textureRegion[0] || uv.x > input.textureRegion[2])
    {
        discard;
    }
    if (uv.y < input.textureRegion[1] || uv.y > input.textureRegion[3])
    {
        discard;
    }

    float2 workingUV = (uv - input.textureRegion.xy) / (input.textureRegion.zw - input.textureRegion.xy);

    float2 newUV = {
        input.spriteBounds[0] + (input.spriteBounds[2] - input.spriteBounds[0]) * workingUV.x,
        input.spriteBounds[1] + (input.spriteBounds[3] - input.spriteBounds[1]) * workingUV.y                    
    };

    float4 color;
    if(effectType == 0)
    {
        color = cool.Sample(fullscreenSampler, newUV).rgba;
    }
    else
    {
        color = cool.Sample(fullscreenSampler, workingUV).rgba;
    }

    color.r *= input.color.r;
    color.g *= input.color.g;
    color.b *= input.color.b;
    color.a *= input.color.a;

    // if (color.a <= 0.5f)
    // {
    //    discard;
    // }

    // if (color.a < 1.0f)
    // {
    //    // float factor = smoothstep(0.0f, 1.0f, color.a) * 10.0f;
    //    // const float grayscale = (color.r + color.g + color.b) / factor;

    // 	// // Set RGB components to grayscale value, keeping the alpha channel intact
    //    // color.rgb = float3(grayscale, grayscale, grayscale);
    // }

    switch (effectType)
    {
        case 0: // None
    		{
                // This is probably where
                // all particle sprites go
                // unless Assar wants a cool
                // effect on them.
                
                break;
            }
        case 1: // Bars
    		{
                if (uv.x < input.spriteBounds[0] ||
                    uv.x > input.spriteBounds[2] ||
                    uv.y < input.spriteBounds[1] ||
                    uv.y > input.spriteBounds[3])
                {

                	// Calculate grayscale value (average of RGB components)
                    const float grayscale = (color.r + color.g + color.b) / 20.0f;
					// Set RGB components to grayscale value, keeping the alpha channel intact
                    color.rgb = float3(grayscale, grayscale, grayscale);
                    color.a = 0.0f;
                }
                break;
            }
        case 2: // Circle
    		{
                const float distance = length(uv - float2(0.5f, 0.5f));
                if (distance > sqrt(0.5f) * spriteBounds[0])
                {
                    color.a = 0.0f;
                }
                break;
            }
        case 3: // Cooldown
	        {
                const float factor = input.spriteBounds[0];
                const float2 center = float2(0.5f, 0.5f);
                const float2 offset = uv - center;
                const float minAngle = -PI;
                const float maxAngle = minAngle + factor * 2 * PI;
                float angle = atan2(offset.x, offset.y);

                if (angle < minAngle)
                {
                    angle += 2 * PI;
                }
                if (angle > maxAngle)
                {
                    angle -= 2 * PI;
                }

                if (angle > minAngle && angle < maxAngle)
                {
					// Calculate grayscale value (average of RGB components)
                    const float grayscale = (color.r + color.g + color.b) / 20.0f;
					// Set RGB components to grayscale value, keeping the alpha channel intact
                    color.rgb = float3(grayscale, grayscale, grayscale);
                }
                break;
            }
        case 4: // JamesBond
    		{
                const float2 center = float2((input.spriteBounds.x + input.spriteBounds.z) * 0.5f, (input.spriteBounds.y + input.spriteBounds.w) * 0.5f);
                const float radiusX = 0.5f * (input.spriteBounds.z - input.spriteBounds.x);
                const float radiusY = 0.5f * (input.spriteBounds.w - input.spriteBounds.y);
                const float2 normalizedUV = (uv - center) / float2(radiusX, radiusY);
                const float distance = length(normalizedUV);

                if (distance > 1.0f)
                {
                    color.a = 0.0f;
                }
                break;
            }
        case 5: // SpinningSquare
	        {
                const float2 center = float2(0.5f, 0.5f);
                const float radius = 0.2f;
                const float angle = input.spriteBounds[0] * 2 * PI;
                const float2 rotatedUV = float2(cos(angle) * (uv.x - center.x) - sin(angle) * (uv.y - center.y) + center.x,
                		sin(angle) * (uv.x - center.x) + cos(angle) * (uv.y - center.y) + center.y);

                if (rotatedUV.x < center.x - radius ||
					rotatedUV.x > center.x + radius ||
					rotatedUV.y < center.y - radius ||
					rotatedUV.y > center.y + radius)
                {
                    color.a = 0.0f;
                }
                break;
            }
            case 6: // Resources
            {
                float fadeLength = 0.01;
                if (input.spriteBounds[1] < 0.01f)
                {
                    fadeLength = 0.0f;
                }
                float amp = 0.035f;
                float frequency = 0.09f;
                float speed = 3.0f;
                float dist = amp * sin(currentTime + uv.x) * uv.y * sin(currentTime * uv.x * frequency + currentTime * speed);
                color.a *= smoothstep(input.spriteBounds[1] - fadeLength, input.spriteBounds[1] + fadeLength, uv.y + dist);
                
                if (uv.y < input.spriteBounds[1])
                {
                    color.a = 0.0f;
                }

                break;
            }
        default:
            break;
    }
    
    color.rgb *= color.a;
    return color;
}