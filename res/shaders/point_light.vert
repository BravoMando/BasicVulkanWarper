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

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 Projection;
    mat4 View;
    vec4 AmbientLightColor; // w is indtensity
    vec3 LightPosition;
    vec4 LightColor; // w is intensity
} ubo;

const float LIGHT_RADIANS = 0.05;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];

    vec4 lightCenterInCameraSpace = ubo.View * vec4(ubo.LightPosition, 1.0);
    vec3 positionInCameraSpace = lightCenterInCameraSpace.xyz + LIGHT_RADIANS * vec3(fragOffset, 0.0);

    gl_Position = ubo.Projection * vec4(positionInCameraSpace, 1.0);
}