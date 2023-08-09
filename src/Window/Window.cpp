#include "Window.hpp"

namespace Divine
{
    Window::Window(int w, int h, const std::string &name)
        : m_Width(w), m_Height(h), m_Name(name)
    {
        InitWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate();
    }

    void Window::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_WindowHandle = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_WindowHandle, this);
        glfwSetFramebufferSizeCallback(m_WindowHandle, Window::FrameBufferResizeCallback);
    }

    void Window::FrameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto MyWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        MyWindow->m_FrameBufferResizeFlag = true;
        MyWindow->m_Width = width;
        MyWindow->m_Height = height;
    }

    VkResult Window::CreateWindowSurface(VkInstance instance, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface)
    {
        return glfwCreateWindowSurface(instance, m_WindowHandle, pAllocator, pSurface);
    }
}