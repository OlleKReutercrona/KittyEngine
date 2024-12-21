Texture2D worldPositionTex : register(t0);
Texture2D colourTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D materialTex : register(t3);
Texture2D effectsTex : register(t4);
Texture2D ambientOcclusionTex : register(t5);
Texture2D depthTex : register(t6);
Texture2D SSAOTexture : register(t7);

TextureCube cubemapTexture : register(t15);


//#define UNITY_LIGHT_INTENSITY_DIFFERENCE 66.66f
#define UNITY_LIGHT_POINT_INTENSITY_DIFFERENCE 10.66f
#define UNITY_LIGHT_SPOT_INTENSITY_DIFFERENCE 20.66f
#define UNITY_LIGHT_DIRECTIONAL_INTENSITY_DIFFERENCE 1.5f

struct DeferredVertexInput
{
    float4 position : POSITION;
};

struct DeferredVertexToPixel
{
    float4 position : SV_POSITION;
};