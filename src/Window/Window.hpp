#ifndef WINDOW_HEADER
#define WINDOW_HEADER

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace Divine
{
    class Window
    {
    public:
        Window(int w, int h, const std::string &name);
        ~Window();
        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        inline GLFWwindow *GetWindowHandle() const { return m_WindowHandle; }
        inline bool ShouldClose() const { return glfwWindowShouldClose(m_WindowHandle); }
        inline VkExtent2D GetExtent() const { return {static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height)}; }
        inline bool WasFrameBufferResizd() const { return m_FrameBufferResizeFlag; }
        inline void ResetFrameBufferResizeFlag() { m_FrameBufferResizeFlag = false; }

        VkResult CreateWindowSurface(VkInstance instance, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);

    private:
        void InitWindow();
        static void FrameBufferResizeCallback(GLFWwindow *window, int width, int height);

    private:
        int m_Width, m_Height;
        std::string m_Name;
        GLFWwindow *m_WindowHandle;
        bool m_FrameBufferResizeFlag = false;
    };
}

#endif