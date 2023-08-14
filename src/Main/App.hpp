#ifndef APP_HEADER
#define APP_HEADER

#include "Window.hpp"
#include "Device.hpp"
#include "Game_Object.hpp"
#include "Renderer.hpp"
#include "Render_System.hpp"
#include "PointLight_System.hpp"
#include "Camera.hpp"
#include "Keyboard_Controller.hpp"
#include "Descriptors.hpp"

#include <memory>

namespace Divine
{
    struct GlobalUBO
    {
        glm::mat4 Projection{1.0f};
        glm::mat4 View{1.0f};
        glm::vec4 AmbientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is indtensity
        glm::vec3 LightPosition{-1.0f};
        alignas(16) glm::vec4 LightColor{1.0f}; // w is intensity
    };

    class App
    {
    public:
        App();
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void run();

    public:
        const int m_Width = 800;
        const int m_Height = 600;

    private:
        void LoadGameObjects();

    private:
        Window m_Window{m_Width, m_Height, "Vulkan Warper"};
        Device m_Device{m_Window};
        Renderer m_Renderer{m_Window, m_Device};
        std::unique_ptr<DescriptorPool> up_GlobalPool{};
        DivineGameObject::Map m_GameObjects;
    };
}

#endif