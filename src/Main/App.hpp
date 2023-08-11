#ifndef APP_HEADER
#define APP_HEADER

#include "Window.hpp"
#include "Device.hpp"
#include "Game_Object.hpp"
#include "Renderer.hpp"
#include "Render_System.hpp"
#include "Camera.hpp"
#include "Keyboard_Controller.hpp"
#include "Descriptors.hpp"

#include <memory>

namespace Divine
{
    struct GlobalUBO
    {
        glm::mat4 ProjectionView{1.0f};
        glm::vec3 LightDirection = glm::normalize(glm::vec3(1.0f, -3.0f, -1.0));
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
        std::vector<DivineGameObject> m_GameObjects;
    };
}

#endif