#pragma once
#include "Origin.h"
#include "Device.h"
#include "Buffer.h"

namespace vk
{
    class ModelBuffer
    {
    public:
        template <typename TVertex>
        struct ModelInfo
        {
            std::vector<TVertex> Vertex;
            std::vector<uint32_t> VertexIndex;
            std::string ModelName;
        };

        template <typename TVertex>
        static void ProcessNode(const aiScene *scene, aiNode *node, std::vector<ModelInfo<TVertex>> *modelInfoList, std::function<ModelInfo<TVertex>(aiMesh *)> processMesh)
        {
            if (scene->HasMeshes())
            {
                for (size_t i = 0; i < node->mNumMeshes; i++)
                {
                    modelInfoList->push_back(processMesh(scene->mMeshes[node->mMeshes[i]]));
                }
            }
            for (size_t i = 0; i < node->mNumChildren; i++)
            {
                ProcessNode(scene, node->mChildren[i], modelInfoList, processMesh);
            }
        }

    public:
        ModelBuffer(Device::Ptr device, void *vertexData, uint64_t vertexDataSize, void *indexData, uint64_t indexDataSize, uint32_t vertexIndexCount);
        ~ModelBuffer();

        using Ptr = std::shared_ptr<ModelBuffer>;
        static Ptr New(Device::Ptr device, void *vertexData, uint64_t vertexDataSize, void *indexData, uint64_t indexDataSize, uint32_t vertexIndexCount)
        {
            return std::make_shared<ModelBuffer>(device, vertexData, vertexDataSize, indexData, indexDataSize, vertexIndexCount);
        }

    private:
        Device::Ptr mDevice;
        // 模型缓冲区
        Buffer::Ptr mVertexBuffer;
        Buffer::Ptr mVertexIndexBuffer;
        uint32_t mVertexIndexCount = 0;

    private:
        void CreateModel(void *vertexData, uint64_t vertexDataSize, void *indexData, uint64_t indexDataSize);

    public:
        Buffer::Ptr GetVertexBuffer() { return mVertexBuffer; }
        Buffer::Ptr GetVertexIndexBuffer() { return mVertexIndexBuffer; }
        uint32_t GetVertexIndexCount() { return mVertexIndexCount; }
    };
} // namespace vk
