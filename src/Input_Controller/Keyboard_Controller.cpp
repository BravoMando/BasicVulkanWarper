#include "Keyboard_Controller.hpp"

#include <limits>

namespace Divine
{
    void KeyboardController::MoveInPlaneXZ(GLFWwindow *window, float dt, DivineGameObject &gameObjects)
    {
        glm::vec3 rotate{0.0f};

        if (glfwGetKey(window, m_Keys.lookRight) == GLFW_PRESS)
            rotate.y += 1.0f;
        if (glfwGetKey(window, m_Keys.lookLeft) == GLFW_PRESS)
            rotate.y -= 1.0f;
        if (glfwGetKey(window, m_Keys.lookUp) == GLFW_PRESS)
            rotate.x += 1.0f;
        if (glfwGetKey(window, m_Keys.lookDown) == GLFW_PRESS)
            rotate.x -= 1.0f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
            gameObjects.m_ModelMatrix.rotation += m_TurnSpeed * dt * glm::normalize(rotate);

        gameObjects.m_ModelMatrix.rotation.x = glm::clamp(gameObjects.m_ModelMatrix.rotation.x, -1.5f, 1.5f); // radians
        gameObjects.m_ModelMatrix.rotation.y = glm::mod(gameObjects.m_ModelMatrix.rotation.y, glm::two_pi<float>());

        float yaw = gameObjects.m_ModelMatrix.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (glfwGetKey(window, m_Keys.moveForward) == GLFW_PRESS)
            moveDir += forwardDir;
        if (glfwGetKey(window, m_Keys.moveBackward) == GLFW_PRESS)
            moveDir -= forwardDir;
        if (glfwGetKey(window, m_Keys.moveRight) == GLFW_PRESS)
            moveDir += rightDir;
        if (glfwGetKey(window, m_Keys.moveLeft) == GLFW_PRESS)
            moveDir -= rightDir;
        if (glfwGetKey(window, m_Keys.moveUp) == GLFW_PRESS)
            moveDir += upDir;
        if (glfwGetKey(window, m_Keys.moveDown) == GLFW_PRESS)
            moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObjects.m_ModelMatrix.translation += m_MoveSpeed * dt * glm::normalize(moveDir);
        }
    }
}