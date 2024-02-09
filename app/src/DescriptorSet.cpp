#include "vk/DescriptorSet.h"

namespace vk
{
    DescriptorSet::DescriptorSet(Device::Ptr device, DescriptorSetLayout::Ptr descriptorSetLayout)
        : mDevice(device), mDescriptorSetLayout(descriptorSetLayout)
    {
        CreateDescriptorSet();
    }
    DescriptorSet::~DescriptorSet()
    {
    }

    void DescriptorSet::CreateDescriptorSet()
    {
        mDescriptorSet.resize(mDevice->GetSwapchainImageCount());
        std::vector<VkDescriptorSetLayout> DescriptorSetLayoutList(mDevice->GetSwapchainImageCount(), mDescriptorSetLayout->GetDescriptorSetLayout());
        VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo{};
        DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        DescriptorSetAllocateInfo.descriptorPool = mDescriptorSetLayout->GetDescriptorPool();
        DescriptorSetAllocateInfo.descriptorSetCount = DescriptorSetLayoutList.size();
        DescriptorSetAllocateInfo.pSetLayouts = DescriptorSetLayoutList.data();
        if (vkAllocateDescriptorSets(mDevice->GetLogicalDevice(), &DescriptorSetAllocateInfo, mDescriptorSet.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor set!");
        }
    }
} // namespace vk
