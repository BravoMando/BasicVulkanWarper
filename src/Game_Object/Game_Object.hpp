#ifndef GAME_OBJECT_HEADER
#define GAME_OBJECT_HEADER

#include "Model.hpp"

#include <memory>
#include <unordered_map>

namespace Divine
{
    struct TransformComponent
    {
        glm::vec3 translation{};           // position offset
        glm::vec3 scale{1.0f, 1.0f, 1.0f}; // scale rate
        glm::vec3 rotation{};              // rotate radians

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 GetModelMat();
        glm::mat3 GetNormalMat();
    };

    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };

    class DivineGameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, DivineGameObject>;

        // can't copyable
        DivineGameObject(const DivineGameObject &) = delete;
        DivineGameObject &operator=(const DivineGameObject &) = delete;
        // can moveable
        DivineGameObject(DivineGameObject &&) = default;            // no const
        DivineGameObject &operator=(DivineGameObject &&) = default; // no const

    public:
        static DivineGameObject CreateGameObject();
        static DivineGameObject MakePointLight(
            float intensity = 10.0f,
            float radius = 0.1f,
            glm::vec3 color = glm::vec3(1.0f));

        inline id_t GetID() const { return m_ID; }

    private:
        DivineGameObject(id_t objID)
            : m_ID{objID} {}

    public:
        glm::vec3 m_Color{};
        TransformComponent m_ModelMatrix{};

        // optional component
        std::shared_ptr<Model> sp_Model = nullptr;
        std::unique_ptr<PointLightComponent> up_PointLight = nullptr;

    private:
        id_t m_ID;
    };
}

#endif