static const uint MAX_LIGHTS = 16u; // Needs to be the same in Lighting.h


struct DirectionalLightData
{
    float3 directionalLightDirection;
    float directionalLightIntensity;
    float3 directionalLightColour;
    float ambientLightIntensity;
};

struct PointLightData
{
    float3 position;
    float intensity;
    float3 colour;
    float range;
    bool active;
    float3 padding;
};

struct SpotLightData
{
    float3 position;
    float intensity;
    float3 direction;
    float range;
    float3 colour;
    float innerAngle;
    float outerAngle;
    bool active;
    float2 padding;
};

cbuffer LightBuffer : register(b2)
{
    DirectionalLightData directionalLightData;
    PointLightData pointLights[MAX_LIGHTS];
    SpotLightData spotLights[MAX_LIGHTS];
    uint numberOfPointLights;
    uint numberOfSpotLights;
};