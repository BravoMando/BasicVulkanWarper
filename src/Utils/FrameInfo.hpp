#ifndef FRAMEINFO_HEADER
#define FRAMEINFO_HEADER

#include "Camera.hpp"
#include "Game_Object.hpp"

#include <vulkan/vulkan.h>

namespace Divine
{

#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 Position{}; // ignore w
        glm::vec4 Color{};    // w is intensity
    };

    struct GlobalUBO
    {
        glm::mat4 Projection{1.0f};
        glm::mat4 View{1.0f};
        glm::vec4 AmbientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is indtensity
        PointLight PointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        DivineGameObject::Map &gameObjects;
    };

}

#endif