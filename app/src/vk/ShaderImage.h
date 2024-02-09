#pragma once
#include "Origin.h"
#include "Device.h"
#include "Image.h"
#include "DescriptorSet.h"

namespace vk
{
    class ShaderImage
    {
    public:
        ShaderImage(Device::Ptr device, uint32_t width, uint32_t height, bool isWritePerFrame);
        ~ShaderImage();

        using Ptr = std::shared_ptr<ShaderImage>;
        static Ptr MakePtr(Device::Ptr device, uint32_t width, uint32_t height, bool isWritePerFrame)
        {
            return std::make_shared<ShaderImage>(device, width, height, isWritePerFrame);
        }

    private:
        Device::Ptr mDevice;
        // 着色器图像
        std::vector<Image::Ptr> mShaderImage;
        // 采样器
        VkSampler mImageSampler = nullptr;
        bool mIsWritePerFrame = false;

    private:
        void CreateShaderImage(uint32_t width, uint32_t height);
        void CreateShaderSampler();

    public:
        void WriteDescriptorSet(std::vector<DescriptorSet::Ptr> descriptorSetList, uint32_t dstBinding);

        bool WriteData(uint32_t currentIndex, void *data);
        bool AllWriteData(void *data);
    };
} // namespace vk
