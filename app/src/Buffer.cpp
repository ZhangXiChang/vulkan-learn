#include "vk/Buffer.h"

namespace vk
{
    Buffer::Buffer(Device::Ptr device, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        : mDevice(device), mBufferSize(bufferSize)
    {
        CreateBuffer(usage, properties);
    }
    Buffer::~Buffer()
    {
        if (mBuffer != nullptr)
        {
            vkDestroyBuffer(mDevice->GetLogicalDevice(), mBuffer, nullptr);
        }
        if (mMemory != nullptr)
        {
            vkFreeMemory(mDevice->GetLogicalDevice(), mMemory, nullptr);
        }
    }

    void Buffer::CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        if (!mDevice->CreateBuffer(mBufferSize, usage, properties, &mBuffer, &mMemory))
        {
            return;
        }
    }
    bool Buffer::WriteBuffer(Buffer::Ptr buffer)
    {
        return mDevice->CopyBuffer(buffer->GetBuffer(), mBuffer, buffer->GetBufferSize());
    }
    bool Buffer::WriteHostData(void *data)
    {
        void *TempData;
        if (vkMapMemory(mDevice->GetLogicalDevice(), mMemory, 0, mBufferSize, 0, &TempData) != VK_SUCCESS)
        {
            return false;
        }
        memcpy(TempData, data, mBufferSize);
        vkUnmapMemory(mDevice->GetLogicalDevice(), mMemory);
        return true;
    }
    bool Buffer::WriteData(void *data)
    {
        Buffer::Ptr TempBuffer = Buffer::New(mDevice, mBufferSize,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //
        if (!TempBuffer->WriteHostData(data))
        {
            return false;
        }
        if (!WriteBuffer(TempBuffer))
        {
            return false;
        }
        return true;
    }
} // namespace vk
