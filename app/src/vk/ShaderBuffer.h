#pragma once
#include "Origin.h"
#include "Device.h"
#include "Buffer.h"
#include "DescriptorSet.h"

namespace vk
{
    class ShaderBuffer
    {
    public:
        ShaderBuffer(Device::Ptr device, size_t bufferSize, bool isWritePerFrame);
        ~ShaderBuffer();

        using Ptr = std::shared_ptr<ShaderBuffer>;
        static Ptr MakePtr(Device::Ptr device, size_t bufferSize, bool isWritePerFrame) { return std::make_shared<ShaderBuffer>(device, bufferSize, isWritePerFrame); }

    private:
        Device::Ptr mDevice;
        // 着色器缓冲区
        std::vector<Buffer::Ptr> mShaderBuffer;
        bool mIsWritePerFrame = false;

    private:
        void CreateShaderBuffer(size_t bufferSize);

    public:
        void WriteDescriptorSet(std::vector<DescriptorSet::Ptr> descriptorSetList, uint32_t dstBinding);

        void WriteData(uint32_t currentIndex, void *data);
        void AllWriteData(void *data);
    };
} // namespace vk
