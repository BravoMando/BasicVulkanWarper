#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 Projection;
    mat4 View;
    vec4 AmbientLightColor; // w is indtensity
    vec3 LightPosition;
    vec4 LightColor; // w is intensity
} ubo;

layout (push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec3 directionToLight = ubo.LightPosition - fragPosWorld;
    float attenuation = 1.0f / dot(directionToLight, directionToLight);

    vec3 lightColor = ubo.LightColor.xyz * ubo.LightColor.w * attenuation;
    vec3 ambientLight = ubo.AmbientLightColor.xyz * ubo.AmbientLightColor.w;

    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}