#ifndef FRAMEINFO_HEADER
#define FRAMEINFO_HEADER

#include "Camera.hpp"
#include <vulkan/vulkan.h>

namespace Divine
{
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
    };

}

#endif