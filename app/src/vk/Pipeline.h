#pragma once
#include "Origin.h"
#include "Device.h"
#include "DescriptorSetLayout.h"
#include "ShaderModule.h"

namespace vk
{
    class Pipeline
    {
    public:
        struct PipelineInfo
        {
            std::vector<ShaderModule::Ptr> ShaderModuleList;
            VkVertexInputBindingDescription VertexInputBindingDescription;
            std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionList;
        };

    public:
        Pipeline(Device::Ptr device, DescriptorSetLayout::Ptr descriptorSet, PipelineInfo info);
        ~Pipeline();

        using Ptr = std::shared_ptr<Pipeline>;
        static Ptr New(Device::Ptr device, DescriptorSetLayout::Ptr descriptorSet, PipelineInfo info) { return std::make_shared<Pipeline>(device, descriptorSet, info); }

    private:
        Device::Ptr mDevice;
        // 渲染管线
        VkPipeline mPipeline = nullptr;

    private:
        void CreatePipeline(DescriptorSetLayout::Ptr descriptorSet, PipelineInfo info);

    public:
        VkPipeline GetPipeline() { return mPipeline; }
    };
} // namespace vk
