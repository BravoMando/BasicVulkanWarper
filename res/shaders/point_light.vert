#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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
    vec4 Position;
    vec4 Color;
    float radius;
} push;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];

    vec4 lightCenterInCameraSpace = ubo.View * push.Position;
    vec3 positionInCameraSpace = lightCenterInCameraSpace.xyz + push.radius * vec3(fragOffset, 0.0);

    gl_Position = ubo.Projection * vec4(positionInCameraSpace, 1.0);
}