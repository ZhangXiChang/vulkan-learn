#pragma once
#include "Origin.h"
#include "Device.h"
#include "DescriptorSetLayout.h"

namespace vk
{
    class DescriptorSet
    {
    public:
        DescriptorSet(Device::Ptr device, DescriptorSetLayout::Ptr descriptorSetLayout);
        ~DescriptorSet();

        using Ptr = std::shared_ptr<DescriptorSet>;
        static Ptr MakePtr(Device::Ptr device, DescriptorSetLayout::Ptr descriptorSetLayout) { return std::make_shared<DescriptorSet>(device, descriptorSetLayout); }

    private:
        Device::Ptr mDevice;
        // 描述符布局
        DescriptorSetLayout::Ptr mDescriptorSetLayout;
        // 描述符
        std::vector<VkDescriptorSet> mDescriptorSet;

    private:
        void CreateDescriptorSet();

    public:
        VkDescriptorSet GetDescriptorSet(uint32_t currentIndex) { return mDescriptorSet[currentIndex]; }
        VkPipelineLayout GetPipelineLayout() { return mDescriptorSetLayout->GetPipelineLayout(); }
    };
} // namespace vk
