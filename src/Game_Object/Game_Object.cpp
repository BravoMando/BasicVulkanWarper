#include "Game_Object.hpp"

namespace Divine
{
    DivineGameObject DivineGameObject::CreateGameObject()
    {
        static id_t currentID = 0;

        return DivineGameObject{currentID++};
    }

    DivineGameObject DivineGameObject::MakePointLight(
        float intensity,
        float radius,
        glm::vec3 color)
    {
        auto obj = DivineGameObject::CreateGameObject();
        obj.m_Color = color;
        obj.m_ModelMatrix.scale.x = radius;

        obj.up_PointLight = std::make_unique<PointLightComponent>();
        obj.up_PointLight->lightIntensity = intensity;

        return obj;
    }

    glm::mat4 TransformComponent::GetModelMat()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);

        return glm::mat4{
            {scale.x * (c1 * c3 + s1 * s2 * s3),
             scale.x * (c2 * s3),
             scale.x * (c1 * s2 * s3 - c3 * s1),
             0.0f},
            {scale.y * (c3 * s1 * s2 - c1 * s3),
             scale.y * (c2 * c3),
             scale.y * (c1 * c3 * s2 + s1 * s3),
             0.0f},
            {scale.z * (c2 * s1),
             scale.z * (-s2),
             scale.z * (c1 * c2),
             0.0f},
            {translation.x, translation.y, translation.z, 1.0f}};
    }

    glm::mat3 TransformComponent::GetNormalMat()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
            {invScale.x * (c1 * c3 + s1 * s2 * s3),
             invScale.x * (c2 * s3),
             invScale.x * (c1 * s2 * s3 - c3 * s1)},
            {invScale.y * (c3 * s1 * s2 - c1 * s3),
             invScale.y * (c2 * c3),
             invScale.y * (c1 * c3 * s2 + s1 * s3)},
            {invScale.z * (c2 * s1),
             invScale.z * (-s2),
             invScale.z * (c1 * c2)}};
    }
}