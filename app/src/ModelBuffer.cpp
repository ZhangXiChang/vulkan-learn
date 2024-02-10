#include "vk/ModelBuffer.h"

namespace vk
{
    ModelBuffer::ModelBuffer(Device::Ptr device, void *vertexData, uint64_t vertexDataSize, void *indexData, uint64_t indexDataSize, uint32_t vertexIndexCount)
        : mDevice(device), mVertexIndexCount(vertexIndexCount)
    {
        CreateModel(vertexData, vertexDataSize, indexData, indexDataSize);
    }
    ModelBuffer::~ModelBuffer()
    {
    }

    void ModelBuffer::CreateModel(void *vertexData, uint64_t vertexDataSize, void *indexData, uint64_t indexDataSize)
    {
        // 创建顶点缓冲区
        mVertexBuffer = Buffer::New(mDevice, vertexDataSize,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //
        mVertexBuffer->WriteData(vertexData);
        // 创建索引缓冲区
        mVertexIndexBuffer = Buffer::New(mDevice, indexDataSize,
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //
        mVertexIndexBuffer->WriteData(indexData);
    }
} // namespace vk
