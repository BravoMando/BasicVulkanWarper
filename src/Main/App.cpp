#include "App.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>

namespace Divine
{
    App::App()
    {
        LoadGameObjects();
    }

    App::~App() {}

    void App::LoadGameObjects()
    {
        std::shared_ptr<Model> model = Model::CreateModelFromFile(m_Device, HOME_DIR "res/models/smooth_vase.obj");

        auto smooth = DivineGameObject::CreateGameObject();
        smooth.sp_Model = model;
        smooth.m_ModelMatrix.translation = {-0.25f, 0.0f, 2.5f};
        smooth.m_ModelMatrix.scale = {1.0f, 1.0f, 1.0f};
        m_GameObjects.push_back(std::move(smooth));

        model = Model::CreateModelFromFile(m_Device, HOME_DIR "res/models/flat_vase.obj");
        auto flat = DivineGameObject::CreateGameObject();
        flat.sp_Model = model;
        flat.m_ModelMatrix.translation = {0.25f, 0.0f, 2.5f};
        flat.m_ModelMatrix.scale = {1.0f, 1.0f, 1.0f};
        m_GameObjects.push_back(std::move(flat));
    }

    void App::run()
    {
        RenderSystem renderSystem{m_Device, m_Renderer.GetSwapChainRenderPass()};
        Camera camera{};
        // camera.SetViewDirection({ 0.0f,0.0f,0.0f }, { 0.5f,00.1f,1.0f });
        // camera.SetViewTarget({-0.5f, -2.0f, -2.0f}, {0.0f, 0.0f, 2.5f});

        auto viewerObject = DivineGameObject::CreateGameObject();
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
                m_Renderer.BeginSwapChainRenderPass(commandBuffer);
                renderSystem.RenderGameObjects(commandBuffer, m_GameObjects, camera);
                m_Renderer.EndSwapChainRenderPass(commandBuffer);
                m_Renderer.EndFrame();
            }
        }

        vkDeviceWaitIdle(m_Device.GetDevice());
    }
}