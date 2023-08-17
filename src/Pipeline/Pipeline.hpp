#ifndef PIPELINE_HEADER
#define PIPELINE_HEADER

#include "Device.hpp"

#include <string>
#include <vector>

namespace Divine
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        // VkViewport viewport;
        // VkRect2D scissor;
        // VkPipelineViewportStateCreateInfo viewportInfo;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStatesEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        uint32_t subpass = 0;
    };

    class Pipeline
    {
    public:
        Pipeline(
            Device &device,
            const std::string &vertFilePath,
            const std::string &fragFilePath,
            const PipelineConfigInfo &configInfo);
        ~Pipeline();
        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(VkCommandBuffer commandBuffer);

        static void DefaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
        static void EnableAlphaBlending(PipelineConfigInfo &configInfo);

    private:
        void CreateGraphicsPipeline(
            const std::string &vertFilePath,
            const std::string &fragFilePath,
            const PipelineConfigInfo &configInfo);

        void CreateShaderModule(const std::vector<char> &code, VkShaderModule *pModule);

    private:
        static std::vector<char> ReadFile(const std::string &filePath);

    private:
        Device &r_Device;
        VkShaderModule m_VertShaderModule;
        VkShaderModule m_FragShaderModule;
        VkPipeline m_GraphicsPipeline;
    };

}

#endif
