#include "vk/ShaderBuffer.h"

namespace vk
{
    ShaderBuffer::ShaderBuffer(Device::Ptr device, size_t bufferSize, bool isWritePerFrame)
        : mDevice(device), mIsWritePerFrame(isWritePerFrame)
    {
        CreateShaderBuffer(bufferSize);
    }
    ShaderBuffer::~ShaderBuffer()
    {
    }

    void ShaderBuffer::CreateShaderBuffer(size_t bufferSize)
    {
        // 创建世界空间缓冲区
        if (mIsWritePerFrame)
        {
            mShaderBuffer.resize(mDevice->GetSwapchainImageCount());
            for (auto &&i : mShaderBuffer)
            {
                i = Buffer::New(mDevice, bufferSize,
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                //
            }
        }
        else
        {
            mShaderBuffer.resize(1);
            mShaderBuffer[0] = Buffer::New(mDevice, bufferSize,
                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            //
        }
    }
    void ShaderBuffer::WriteDescriptorSet(std::vector<DescriptorSet::Ptr> descriptorSetList, uint32_t dstBinding)
    {
        for (auto &&j : descriptorSetList)
        {
            // 更新描述符
            for (size_t i = 0; i < mDevice->GetSwapchainImageCount(); i++)
            {
                VkDescriptorBufferInfo DescriptorBufferInfo{};
                if (mIsWritePerFrame)
                {
                    DescriptorBufferInfo.buffer = mShaderBuffer[i]->GetBuffer();
                    DescriptorBufferInfo.offset = 0;
                    DescriptorBufferInfo.range = mShaderBuffer[i]->GetBufferSize();
                }
                else
                {
                    DescriptorBufferInfo.buffer = mShaderBuffer[0]->GetBuffer();
                    DescriptorBufferInfo.offset = 0;
                    DescriptorBufferInfo.range = mShaderBuffer[0]->GetBufferSize();
                }

                VkWriteDescriptorSet WriteDescriptorSet{};
                WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                WriteDescriptorSet.dstSet = j->GetDescriptorSet(i);
                WriteDescriptorSet.dstBinding = dstBinding;
                WriteDescriptorSet.dstArrayElement = 0;
                WriteDescriptorSet.descriptorCount = 1;
                WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;
                vkUpdateDescriptorSets(mDevice->GetLogicalDevice(), 1, &WriteDescriptorSet, 0, nullptr);
            }
        }
    }
    void ShaderBuffer::WriteData(uint32_t currentIndex, void *data)
    {
        mShaderBuffer[currentIndex]->WriteData(data);
    }
    void ShaderBuffer::AllWriteData(void *data)
    {
        for (auto &&i : mShaderBuffer)
        {
            i->WriteData(data);
        }
    }
} // namespace vk
