#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Divine
{
    class Camera
    {
    public:
        inline const glm::mat4 &GetViewMat() const { return m_ViewMatrix; }
        inline const glm::mat4 &GetProjectionMat() const { return m_ProjectionMatrix; }

        void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
        void SetPerspctiveProjection(float fovy, float aspect, float near, float far);

        void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = {0.0f, -1.0f, 0.0f});
        void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.0f, -1.0f, 0.0f});
        void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

    private:
        glm::mat4 m_ViewMatrix{1.0f};
        glm::mat4 m_ProjectionMatrix{1.0f};
    };
}

#endif