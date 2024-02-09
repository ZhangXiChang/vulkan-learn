#pragma once
#include "Origin.h"
#include "Device.h"

namespace vk
{
    class Buffer
    {
    public:
        Buffer(Device::Ptr device, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~Buffer();

        using Ptr = std::shared_ptr<Buffer>;
        static Ptr MakePtr(Device::Ptr device, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        {
            return std::make_shared<Buffer>(device, bufferSize, usage, properties);
        }

    private:
        Device::Ptr mDevice;
        // 缓冲区
        size_t mBufferSize = 0;
        VkBuffer mBuffer = nullptr;
        VkDeviceMemory mMemory = nullptr;

    private:
        void CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    public:
        bool WriteBuffer(Buffer::Ptr buffer);
        bool WriteHostData(void *data);
        bool WriteData(void *data);

        VkBuffer GetBuffer() { return mBuffer; }
        VkDeviceMemory GetMemory() { return mMemory; }
        size_t GetBufferSize() { return mBufferSize; }
    };
} // namespace vk
