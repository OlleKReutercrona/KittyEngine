#include "Commons/common.hlsli"
#include "Commons/spriteCommon.hlsli"

Texture2D colourTex : register(t0);

float DegToRad(float degrees)
{
    return degrees * (3.14159265359f / 180.0f);
}

float Median(float a, float b, float c)
{
    return max(min(a, b), min(max(a, b), c));
}

float ScreenPxRange(float2 texCoord)
{
    const float2 pxRange = float2(2.0f, 2.0f);
    const float2 textureSize = float2(1024.0f, 1024.0f);

    float2 unitRange = float2(pxRange) / textureSize;
    float2 screenTexSize = float2(1.0f, 1.0f) / float2(clientResolution);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

cbuffer FontAttributeBuffer : register(b8)
{
    float4 textColour;

    float4 strokeColour;
    float strokeSize;
    float strokeSoftness;

    float padding1;
    float padding2;
}

float4 main(SpritePixelInput input) : SV_TARGET
{
    if (input.color.a <= 0.0f)
    {
        discard;
    }
    
    float2 uv = input.uv;

    if (uv.x < input.textureRegion[0] || uv.x > input.textureRegion[2])
    {
        discard;
    }
    if (uv.y < input.textureRegion[1] || uv.y > input.textureRegion[3])
    {
        discard;
    }

    float2 workingUV = (uv - input.textureRegion.xy) / (input.textureRegion.zw - input.textureRegion.xy);

    float2 newUV =
    {
        input.spriteBounds[0] + (input.spriteBounds[2] - input.spriteBounds[0]) * workingUV.x,
        input.spriteBounds[1] + (input.spriteBounds[3] - input.spriteBounds[1]) * workingUV.y                    
    };

    float4 mtsdf = colourTex.Sample(bilinearSampler, newUV);

    float d;
    const bool MULTI_CHANNEL = true;

    const float thickness = 0.5f;
    if (MULTI_CHANNEL)
    {
		d = Median(mtsdf.r, mtsdf.g, mtsdf.b) - 0.5f;
    }
    else
    {
        d = mtsdf.a - 0.5f;
    }

    float pxRange = ScreenPxRange(input.uv);
    const float2 textureSize = float2(1024.0f, 1024.0f);

    float pxSize = min(0.5 / pxRange * (fwidth(newUV.x) * textureSize.x + fwidth(newUV.y) * textureSize.y), 0.25);

    float screenPxDistance = ScreenPxRange(input.uv) * (d);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    float dist = d / fwidth(d) + 0.5f;
    float w = clamp(dist, 0.0f, 1.0f);

    float4 insideColour = float4(input.color.rgba);
    float4 outsideColour = float4(input.color.rgb, 0.0f);

    float4 strokeCol = float4(strokeColour.rgba);

    float4 shadowColour = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float4 result = lerp(outsideColour, insideColour, w);
    float4 shadow;

    const float shadowSmoothing = 8.0f / 16.0f;
    const bool dropShadow = false;
    if (dropShadow)
    {
        const float2 spriteDims = input.spriteBounds.zw - input.spriteBounds.xy;

        const float shadowAngle = currentTime * 360.0f;
        const float shadowDistance = 0.1f;
        const float2 shadowOffset = float2(
			cos(DegToRad(shadowAngle)),
			sin(DegToRad(shadowAngle))
        ) * spriteDims * shadowDistance;

        float2 shadowSamplePos = newUV + shadowOffset;
        shadowSamplePos.x = clamp(shadowSamplePos.x, input.spriteBounds.x, input.spriteBounds.z);
        shadowSamplePos.y = clamp(shadowSamplePos.y, input.spriteBounds.w, input.spriteBounds.y);

        float4 shadowSample = colourTex.Sample(bilinearSampler, shadowSamplePos);
        const float shadowAlpha = smoothstep(0.5f - shadowSmoothing, 0.5f + shadowSmoothing, shadowSample.a);
        shadow = float4(shadowColour.rgb, shadowAlpha * shadowColour.a);

        result = lerp(shadow, result, result.a);
    }

    result.a = opacity * textColour.a * input.color.a;
    return result;
}

