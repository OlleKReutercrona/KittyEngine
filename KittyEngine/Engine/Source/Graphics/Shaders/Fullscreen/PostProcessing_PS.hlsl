#include "Commons/common.hlsli"
#include "Commons/InputCommons.hlsli"

Texture2D colourTex : register(t0);
Texture2D bloomTexture : register(t5);


cbuffer FullScreenBuffer : register(b1)
{
    float2 CARedOffset;
    float2 CAGreenOffset;
    
    float2 CABlueOffset;
    float CAMultiplier;
    float boundSaturation;
    
    float3 ColourCorrection;
    float boundContrast;
    
    float4 boundTint;
    
    float4 boundBlackPoint;
    
    float boundExposure;
    float3 bloomSampleTreshold;

    float vignetteSize;
    float vignetteFeatherThickness;
    float vignetteIntensity;
    bool vignetteShowMask;
    
    float toneMapIntensity;
    float3 padding;
}

float3 s_curve(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float3 tonemap_s_gamut3_cine(float3 c)
{
    // based on Sony's s gamut3 cine
    float3x3 fromSrgb = float3x3(
        +0.6456794776, +0.2591145470, +0.0952059754,
        +0.0875299915, +0.7596995626, +0.1527704459,
        +0.0369574199, +0.1292809048, +0.8337616753);

    float3x3 toSrgb = float3x3(
        +1.6269474099, -0.5401385388, -0.0868088707,
        -0.1785155272, +1.4179409274, -0.2394254004,
        +0.0444361150, -0.1959199662, +1.2403560812);

    return mul(toSrgb, s_curve(mul(fromSrgb, c)));
}

float4 main(PixelInput aInput) : SV_TARGET
{
    //float4 colour = colourTex.Sample(defaultSampler, aInput.texCoord).rgba;
    float4 colour = float4(0,0,0,0);
    
    // Chromatic Aberration
    if(CAMultiplier > 0.0f)
    {
        float2 uvOffset = aInput.texCoord - float2(0.5f, 0.5f);

        float strength = 1.0f;//sin(currentTime * 5) * 5;

        float strX = uvOffset.x * CAMultiplier * strength;
        float strY = uvOffset.y * CAMultiplier * strength;
        
        strY *= strY;
        strX *= strX;
        
        float2 redUV = float2
        (aInput.texCoord.x + strX * CARedOffset.x,
        aInput.texCoord.y + strY * CARedOffset.y);
        
        float2 greenUV = float2(
        aInput.texCoord.x + strX * CAGreenOffset.x,
        aInput.texCoord.y + strY * CAGreenOffset.y);
        
        float2 blueUV = float2(
        aInput.texCoord.x + strX * CABlueOffset.x,
        aInput.texCoord.y + strY * CABlueOffset.y);
        
        
        colour.r = colourTex.Sample(fullscreenSampler, redUV).r;
        colour.g = colourTex.Sample(fullscreenSampler, greenUV).g;
        colour.b = colourTex.Sample(fullscreenSampler, blueUV).b;
        colour.a = colourTex.Sample(fullscreenSampler, aInput.texCoord).a;
    }
    else
    {
        colour = colourTex.Sample(fullscreenSampler, aInput.texCoord);
    }
    
    // Bloom
    float4 bloomColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    {
        bloomColour = bloomTexture.Sample(fullscreenSampler, aInput.texCoord);
        bloomColour *= bloomColour.a;
    }
    colour += bloomColour;
    
    // return bloomColour;
    
    // Saturation
    float4 saturationColour;
    {
        float saturation = boundSaturation;
        float luminance = dot(float3(0.2126f, 0.7152f, 0.0722f), colour.rgb); // convert to grayscale
        saturationColour = luminance + saturation * (colour - luminance);
    }
    colour = saturationColour;
    
    // Exposure
    float4 exposureColour;
    {
        float exposure = boundExposure; // default value = 0.0f
        exposureColour = exp2(exposure) * colour;
    }
    colour = exposureColour;
    
    // Contrast
    float4 contrastColour;
    {
        float4 contrast = boundContrast;
        contrastColour = 0.18f * pow(colour / 0.18f, contrast); // 0.18 is perceived as 50% grey. Keep it constant
    }
    colour = contrastColour;
    
    // Tint
    float4 tintColour;
    {
        float4 tint = boundTint;
        tintColour = colour * tint;
    }
    colour = tintColour;
    
    // BlackPoint
    float4 blackPointColour;
    {
        float4 blackPoint = boundBlackPoint;
        blackPointColour = max(0.0f, colour - blackPoint);
    }
    colour = blackPointColour;
    
    // Colour Correcting
    float4 CCColour = colour;
    {
        CCColour.r *= ColourCorrection.x;
        CCColour.g *= ColourCorrection.y;
        CCColour.b *= ColourCorrection.z;
    }
    colour = CCColour;

    // Vignette
    float3 vignetteColourIgnore = float3(0.0f, 0.0f, 0.0f);
    float vignette = 1.0f;
    if(vignetteIntensity > 0.0f)
    {
        // screen size of the circle
        float size = vignetteSize;

        // Thickness of the circle
        float thickness = vignetteFeatherThickness;

        // Screen ratio
        float ratio = clientResolution.x / clientResolution.y;

        float2 center = float2(0.5f, 0.5f);
        float2 offset = float2(0.5f, -1.5f);


        float2 scaledUV = float2(aInput.texCoord  / float2(ratio, 1.0f));
        vignette = 1 - smoothstep(size - thickness, size, length(scaledUV - center)) * vignetteIntensity;
        if(vignetteShowMask)
        {
            return float4(vignette, vignette, vignette, 1.0f);
        }

        colour *= float4(vignette * (1.0 - vignetteColourIgnore.r), vignette * (1.0 - vignetteColourIgnore.g), vignette * (1.0 - vignetteColourIgnore.b), 1.0f);
    }
    
    colour.rgb = lerp(colour.rgb, tonemap_s_gamut3_cine(colour.rgb), clamp(toneMapIntensity, 0.0f, 1.0f));
    colour.a = 1.0f;
    
    if (colour.a < 0.1f)
    {
        discard;
        return colour;
    }

    return float4(colour);
}