#include "vk/DescriptorSetLayout.h"

namespace vk
{
    DescriptorSetLayout::DescriptorSetLayout(Device::Ptr device, uint32_t descriptorSetCount, std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList,
                                             VkPushConstantRange *pushConstantRange)
        : mDevice(device)
    {
        CreateDescriptorSetLayout(descriptorSetLayoutBindingList);
        CreateDescriptorPool(descriptorSetCount, descriptorSetLayoutBindingList);
        CreatePipelineLayout(pushConstantRange);
    }
    DescriptorSetLayout::~DescriptorSetLayout()
    {
        // 渲染管线布局
        if (mPipelineLayout != nullptr)
        {
            vkDestroyPipelineLayout(mDevice->GetLogicalDevice(), mPipelineLayout, nullptr);
        }
        // 描述符池
        if (mDescriptorPool != nullptr)
        {
            vkDestroyDescriptorPool(mDevice->GetLogicalDevice(), mDescriptorPool, nullptr);
        }
        // 描述符集布局
        if (mDescriptorSetLayout != nullptr)
        {
            vkDestroyDescriptorSetLayout(mDevice->GetLogicalDevice(), mDescriptorSetLayout, nullptr);
        }
    }

    void DescriptorSetLayout::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList)
    {
        VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo{};
        DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        DescriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindingList.size();
        DescriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindingList.data();
        if (vkCreateDescriptorSetLayout(mDevice->GetLogicalDevice(), &DescriptorSetLayoutCreateInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }
    void DescriptorSetLayout::CreateDescriptorPool(uint32_t descriptorSetCount, std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList)
    {
        std::vector<VkDescriptorPoolSize> DescriptorPoolSizeList;
        for (auto &&i : descriptorSetLayoutBindingList)
        {
            VkDescriptorPoolSize DescriptorPoolSize{};
            DescriptorPoolSize.type = i.descriptorType;
            DescriptorPoolSize.descriptorCount = i.descriptorCount;
            DescriptorPoolSizeList.push_back(DescriptorPoolSize);
        }
        VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo{};
        DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        DescriptorPoolCreateInfo.poolSizeCount = DescriptorPoolSizeList.size();
        DescriptorPoolCreateInfo.pPoolSizes = DescriptorPoolSizeList.data();
        DescriptorPoolCreateInfo.maxSets = mDevice->GetSwapchainImageCount() * descriptorSetCount;
        if (vkCreateDescriptorPool(mDevice->GetLogicalDevice(), &DescriptorPoolCreateInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }
    void DescriptorSetLayout::CreatePipelineLayout(VkPushConstantRange *pushConstantRange)
    {
        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
        PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        PipelineLayoutCreateInfo.setLayoutCount = 1;
        PipelineLayoutCreateInfo.pSetLayouts = &mDescriptorSetLayout;
        if (pushConstantRange != nullptr)
        {
            PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            PipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRange;
        }
        if (vkCreatePipelineLayout(mDevice->GetLogicalDevice(), &PipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }
} // namespace vk
