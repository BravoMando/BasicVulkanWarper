#version 450

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 Projection;
    mat4 View;
    vec4 AmbientLightColor; // w is indtensity
    vec3 LightPosition;
    vec4 LightColor; // w is intensity
} ubo;

void main()
{
    float dis = sqrt(dot(fragOffset, fragOffset));
    if(dis >= 1.0)
        discard;

    outColor = vec4(ubo.LightColor.xyz, 1.0);
}

