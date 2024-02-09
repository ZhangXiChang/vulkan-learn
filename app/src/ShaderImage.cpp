#include "vk/ShaderImage.h"

namespace vk
{
    ShaderImage::ShaderImage(Device::Ptr device, uint32_t width, uint32_t height, bool isWritePerFrame)
        : mDevice(device), mIsWritePerFrame(isWritePerFrame)
    {
        CreateShaderImage(width, height);
        CreateShaderSampler();
    }
    ShaderImage::~ShaderImage()
    {
        // 采样器
        if (mImageSampler != nullptr)
        {
            vkDestroySampler(mDevice->GetLogicalDevice(), mImageSampler, nullptr);
        }
    }

    void ShaderImage::CreateShaderImage(uint32_t width, uint32_t height)
    {
        if (mIsWritePerFrame)
        {
            mShaderImage.resize(mDevice->GetSwapchainImageCount());
            for (auto &&i : mShaderImage)
            {
                i = Image::MakePtr(mDevice, width, height,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                //
            }
        }
        else
        {
            mShaderImage.resize(1);
            mShaderImage[0] = Image::MakePtr(mDevice, width, height,
                                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            //
        }
    }
    void ShaderImage::CreateShaderSampler()
    {
        // 创建图像采样器
        VkPhysicalDeviceProperties PhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(mDevice->GetPhysicalDevice(), &PhysicalDeviceProperties);
        VkSamplerCreateInfo SamplerCreateInfo{};
        SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SamplerCreateInfo.anisotropyEnable = VK_TRUE;
        SamplerCreateInfo.maxAnisotropy = PhysicalDeviceProperties.limits.maxSamplerAnisotropy;
        SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        SamplerCreateInfo.compareEnable = VK_FALSE;
        SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        SamplerCreateInfo.mipLodBias = 0.0f;
        SamplerCreateInfo.minLod = 0.0f;
        SamplerCreateInfo.maxLod = mShaderImage[0]->GetMipLevels();
        vkCreateSampler(mDevice->GetLogicalDevice(), &SamplerCreateInfo, nullptr, &mImageSampler);
    }
    void ShaderImage::WriteDescriptorSet(std::vector<DescriptorSet::Ptr> descriptorSetList, uint32_t dstBinding)
    {
        for (auto &&j : descriptorSetList)
        {
            // 更新描述符
            for (size_t i = 0; i < mDevice->GetSwapchainImageCount(); i++)
            {
                VkDescriptorImageInfo SamplerImageInfo{};
                if (mIsWritePerFrame)
                {
                    SamplerImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    SamplerImageInfo.imageView = mShaderImage[i]->GetImageView();
                    SamplerImageInfo.sampler = mImageSampler;
                }
                else
                {
                    SamplerImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    SamplerImageInfo.imageView = mShaderImage[0]->GetImageView();
                    SamplerImageInfo.sampler = mImageSampler;
                }

                VkWriteDescriptorSet WriteDescriptorSet{};
                WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                WriteDescriptorSet.dstSet = j->GetDescriptorSet(i);
                WriteDescriptorSet.dstBinding = dstBinding;
                WriteDescriptorSet.dstArrayElement = 0;
                WriteDescriptorSet.descriptorCount = 1;
                WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                WriteDescriptorSet.pImageInfo = &SamplerImageInfo;
                vkUpdateDescriptorSets(mDevice->GetLogicalDevice(), 1, &WriteDescriptorSet, 0, nullptr);
            }
        }
    }
    bool ShaderImage::WriteData(uint32_t currentIndex, void *data)
    {
        if (!mShaderImage[currentIndex]->WriteData(data))
        {
            return false;
        }
        return true;
    }
    bool ShaderImage::AllWriteData(void *data)
    {
        for (auto &&i : mShaderImage)
        {
            if (!i->WriteData(data))
            {
                return false;
            }
        }
        return true;
    }
} // namespace vk
