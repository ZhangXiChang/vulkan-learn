#pragma once
#include "Origin.h"
#include "Device.h"

namespace vk
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(Device::Ptr device, uint32_t descriptorSetCount, std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList,
                            VkPushConstantRange *pushConstantRange);
        ~DescriptorSetLayout();

        using Ptr = std::shared_ptr<DescriptorSetLayout>;
        static Ptr MakePtr(Device::Ptr device, uint32_t descriptorSetCount, std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList,
                           VkPushConstantRange *pushConstantRange)
        {
            return std::make_shared<DescriptorSetLayout>(device, descriptorSetCount, descriptorSetLayoutBindingList, pushConstantRange);
        }

    private:
        Device::Ptr mDevice;
        // 描述符集布局
        VkDescriptorSetLayout mDescriptorSetLayout = nullptr;
        // 描述符池
        VkDescriptorPool mDescriptorPool = nullptr;
        // 渲染管线布局
        VkPipelineLayout mPipelineLayout = nullptr;

    private:
        void CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList);
        void CreateDescriptorPool(uint32_t descriptorSetCount, std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList);
        void CreatePipelineLayout(VkPushConstantRange *pushConstantRange);

    public:
        VkDescriptorSetLayout GetDescriptorSetLayout() { return mDescriptorSetLayout; }
        VkDescriptorPool GetDescriptorPool() { return mDescriptorPool; }
        VkPipelineLayout GetPipelineLayout() { return mPipelineLayout; }
    };
} // namespace vk
