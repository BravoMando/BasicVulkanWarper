#version 450

layout (location = 0) in vec2 fragOffset;

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
    vec4 Position;
    vec4 Color;
    float radius;
} push;

void main()
{
    float dis = sqrt(dot(fragOffset, fragOffset));
    if(dis >= 1.0)
        discard;

    float k1 = 1 - dis;
    outColor = vec4((push.Color.xyz * push.Color.w) * k1, k1);
}
