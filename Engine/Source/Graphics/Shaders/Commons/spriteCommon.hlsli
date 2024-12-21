struct SpriteVertexInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    //float2 padding : GARBAGE;

    float4x4 transform : INSTANCE_TRANSFORM;
    float4 color : INSTANCE_COLOUR;
    float4 uvRect : INSTANCE_TEX_BOUNDS;
    float4 uvRegion : INSTANCE_TEX_REGION;
};

struct SpritePixelInput
{
    float4 position : SV_POSITION;
    float4 color : INSTANCE_COLOUR;
    float2 uv : TEXCOORD;
    float4 spriteBounds : BOUNDS;
    float4 textureRegion : REGION;
};

cbuffer SpriteRenderBuffer : register(b3)
{
    float4x4 completeTransform;
    float4 spriteBounds;
    int displayMode;
    int effectType;
    bool flipX = false;
    bool flipY = false;
};
