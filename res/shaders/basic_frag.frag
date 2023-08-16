#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 Position; // ignore w
    vec4 Color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 Projection;
    mat4 View;
    vec4 AmbientLightColor; // w is indtensity
    PointLight PointLights[10];
    int numLights;
} ubo;

layout (push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec3 diffuseLight = ubo.AmbientLightColor.xyz * ubo.AmbientLightColor.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for( int i = 0; i < ubo.numLights; ++i)
    {
        PointLight light = ubo.PointLights[i];
        vec3 directionToLight = light.Position.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(directionToLight, directionToLight);
        float cosAngleIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.Color.xyz * light.Color.w * attenuation;

        diffuseLight += intensity * cosAngleIncidence;
    }

    outColor = vec4(diffuseLight * fragColor, 1.0);
}