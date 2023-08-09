#include "Model.hpp"
#include "utils.hpp"

#include <assert.h>
#include <string.h>

#include <iostream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std
{
    template <>
    struct hash<Divine::Model::Vertex>
    {
        size_t operator()(const Divine::Model::Vertex &vertex) const
        {
            size_t seed = 0;
            Divine::HashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);

            return seed;
        }
    };
}

namespace Divine
{
    std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    void Model::Builder::LoadModelFromFile(const std::string &FilePath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FilePath.c_str()))
            throw std::runtime_error(warn + err);

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                if (index.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]};

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]};
                }

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]};
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]};
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    Model::Model(Device &device, const Builder &builder)
        : r_Device{device}
    {
        CreateVertexBuffers(builder.vertices);
        CreateIndexBuffers(builder.indices);
    }

    Model::~Model()
    {
        vkDestroyBuffer(r_Device.GetDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(r_Device.GetDevice(), m_VertexBufferMemory, nullptr);

        if (m_HasIndexBuffer)
        {
            vkDestroyBuffer(r_Device.GetDevice(), m_IndexBuffer, nullptr);
            vkFreeMemory(r_Device.GetDevice(), m_IndexBufferMemory, nullptr);
        }
    }

    void Model::CreateVertexBuffers(const std::vector<Vertex> &vertices)
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 &&
               "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(Vertex) * m_VertexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        r_Device.CreateBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void *data;
        vkMapMemory(r_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(r_Device.GetDevice(), stagingBufferMemory);

        r_Device.CreateBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer,
            m_VertexBufferMemory);

        r_Device.CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

        vkDestroyBuffer(r_Device.GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(r_Device.GetDevice(), stagingBufferMemory, nullptr);
    }

    void Model::CreateIndexBuffers(const std::vector<uint32_t> &indices)
    {
        m_IndexCount = static_cast<uint32_t>(indices.size());

        m_HasIndexBuffer = m_IndexCount > 0;

        if (!m_HasIndexBuffer)
            return;

        VkDeviceSize bufferSize = sizeof(uint32_t) * m_IndexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        r_Device.CreateBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void *data;
        vkMapMemory(r_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(r_Device.GetDevice(), stagingBufferMemory);

        r_Device.CreateBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffer,
            m_IndexBufferMemory);

        r_Device.CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(r_Device.GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(r_Device.GetDevice(), stagingBufferMemory, nullptr);
    }

    void Model::Bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {m_VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_HasIndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void Model::Draw(VkCommandBuffer commandBuffer)
    {
        if (m_HasIndexBuffer)
            vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }

    std::unique_ptr<Model> Model::CreateModelFromFile(Device &device, const std::string &FilePath)
    {
        Builder builder{};
        builder.LoadModelFromFile(FilePath);
        std::cout << "\tVertex count: " << builder.vertices.size() << std::endl;

        return std::make_unique<Model>(device, builder);
    }
}