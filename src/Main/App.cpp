#include "App.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>

namespace Divine
{
    App::App()
    {
        up_GlobalPool = DescriptorPool::Builder(m_Device)
                            .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                            .Build();
        LoadGameObjects();
    }

    App::~App() {}

    void App::LoadGameObjects()
    {
        std::shared_ptr<Model> model = Model::CreateModelFromFile(m_Device, HOME_DIR "res/models/smooth_vase.obj");

        auto smooth = DivineGameObject::CreateGameObject();
        smooth.sp_Model = model;
        smooth.m_ModelMatrix.translation = {-0.5f, 0.5f, 0.0f};
        smooth.m_ModelMatrix.scale = {3.0f, 1.5f, 3.0f};
        m_GameObjects.emplace(smooth.GetID(), std::move(smooth));

        model = Model::CreateModelFromFile(m_Device, HOME_DIR "res/models/flat_vase.obj");
        auto flat = DivineGameObject::CreateGameObject();
        flat.sp_Model = model;
        flat.m_ModelMatrix.translation = {0.5f, 0.5f, 0.0f};
        flat.m_ModelMatrix.scale = {3.0f, 1.5f, 3.0f};
        m_GameObjects.emplace(flat.GetID(), std::move(flat));

        model = Model::CreateModelFromFile(m_Device, HOME_DIR "res/models/quad.obj");
        auto floor = DivineGameObject::CreateGameObject();
        floor.sp_Model = model;
        floor.m_ModelMatrix.translation = {0.0f, 0.5f, 0.0f};
        floor.m_ModelMatrix.scale = {3.0f, 1.0f, 3.0f};
        m_GameObjects.emplace(floor.GetID(), std::move(floor));

        std::vector<glm::vec3> lightColors = {
            {1.f, 1.f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}};

        for (int i = 0; i < lightColors.size(); ++i)
        {
            auto pointLight = DivineGameObject::MakePointLight(1.0f);
            pointLight.m_Color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.0f),
                                           (i * glm::two_pi<float>() / lightColors.size()),
                                           {0.f, -1.f, 0.f});
            pointLight.m_ModelMatrix.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            m_GameObjects.emplace(pointLight.GetID(), std::move(pointLight));
        }
    }

    void App::run()
    {
        std::vector<std::unique_ptr<Buffer>> ubos(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < ubos.size(); ++i)
        {
            ubos[i] = std::make_unique<Buffer>(m_Device,
                                               sizeof(GlobalUBO),
                                               1,
                                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            ubos[i]->Map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(m_Device)
                                   .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                   .Build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = ubos[i]->GetDescriptorBufferInfo();
            DescriptorWriter(*globalSetLayout, *up_GlobalPool)
                .WriteBuffer(0, &bufferInfo)
                .Build(globalDescriptorSets[i]);
        }

        RenderSystem renderSystem{m_Device,
                                  m_Renderer.GetSwapChainRenderPass(),
                                  globalSetLayout->GetDescriptorSetLayout()};

        PointLightSystem pointLightSystem{m_Device,
                                          m_Renderer.GetSwapChainRenderPass(),
                                          globalSetLayout->GetDescriptorSetLayout()};
        Camera camera{};
        // camera.SetViewDirection({ 0.0f,0.0f,0.0f }, { 0.5f,00.1f,1.0f });
        // camera.SetViewTarget({-0.5f, -2.0f, -2.0f}, {0.0f, 0.0f, 2.5f});

        auto viewerObject = DivineGameObject::CreateGameObject();
        viewerObject.m_ModelMatrix.translation.z = -2.5f;
        KeyboardController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();

            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.MoveInPlaneXZ(m_Window.GetWindowHandle(), frameTime, viewerObject);
            camera.SetViewYXZ(viewerObject.m_ModelMatrix.translation, viewerObject.m_ModelMatrix.rotation);

            float aspect = m_Renderer.GetAspectRatio();
            // camera.SetOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, 0.0f, 1.0f);
            camera.SetPerspctiveProjection(glm::radians(45.0f), aspect, 00.1f, 100.0f);

            if (auto commandBuffer = m_Renderer.BeginFrame()) // it may return a null pointer
            {
                auto frameIndex = m_Renderer.GetFrameIndex();
                FrameInfo frameInfo{frameIndex,
                                    frameTime,
                                    commandBuffer,
                                    camera,
                                    globalDescriptorSets[frameIndex],
                                    m_GameObjects};

                // update
                GlobalUBO ubo{};
                ubo.Projection = camera.GetProjectionMat();
                ubo.View = camera.GetViewMat();
                ubo.InverseView = camera.GetInverseViewMat();
                pointLightSystem.Update(frameInfo, ubo);
                ubos[frameIndex]->WriteToBuffer(reinterpret_cast<const void *>(&ubo));
                ubos[frameIndex]->Flush();

                // render
                m_Renderer.BeginSwapChainRenderPass(commandBuffer);
                pointLightSystem.Render(frameInfo);
                renderSystem.RenderGameObjects(frameInfo);
                m_Renderer.EndSwapChainRenderPass(commandBuffer);
                m_Renderer.EndFrame();
            }
        }

        vkDeviceWaitIdle(m_Device.GetDevice());
    }
}