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
    mat4 InverseView;
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
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.InverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    for( int i = 0; i < ubo.numLights; ++i)
    {
        PointLight light = ubo.PointLights[i];
        vec3 directionToLight = light.Position.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(directionToLight, directionToLight);

        directionToLight = normalize(directionToLight);

        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.Color.xyz * light.Color.w * attenuation;

        diffuseLight += intensity * cosAngleIncidence;

        // specular lighting
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0.1, 1.0);
        blinnTerm = pow(blinnTerm, 512.0);

        specularLight += intensity * blinnTerm;
    }

    outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}