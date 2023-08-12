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

        inline id_t GetID() const { return m_ID; }

    private:
        DivineGameObject(id_t objID)
            : m_ID{objID} {}

    public:
        std::shared_ptr<Model> sp_Model{};
        glm::vec3 m_Color{};
        TransformComponent m_ModelMatrix{};

    private:
        id_t m_ID;
    };
}

#endif