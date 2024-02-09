#include "vk/Pipeline.h"

namespace vk
{
    Pipeline::Pipeline(Device::Ptr device, DescriptorSetLayout::Ptr descriptorSet, PipelineInfo info)
        : mDevice(device)
    {
        CreatePipeline(descriptorSet, info);
    }
    Pipeline::~Pipeline()
    {
        if (mPipeline != nullptr)
        {
            vkDestroyPipeline(mDevice->GetLogicalDevice(), mPipeline, nullptr);
        }
    }

    void Pipeline::CreatePipeline(DescriptorSetLayout::Ptr descriptorSet, PipelineInfo info)
    {
        // 着色器
        std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfoList;
        for (auto &&i : info.ShaderModuleList)
        {
            VkPipelineShaderStageCreateInfo ShaderStageCreateInfo{};
            ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            ShaderStageCreateInfo.pName = "main";
            ShaderStageCreateInfo.module = i->GetShaderModule();
            ShaderStageCreateInfo.stage = i->GetShaderStage();
            ShaderStageCreateInfoList.push_back(ShaderStageCreateInfo);
        }

        // 视口
        VkExtent2D Extent = mDevice->GetSwapchainImageExtent();
        VkViewport Viewport{};
        Viewport.x = 0.0f;
        Viewport.y = 0.0f;
        Viewport.width = Extent.width;
        Viewport.height = Extent.height;
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;
        // 剪裁
        VkRect2D Scissor{};
        Scissor.offset = {0, 0};
        Scissor.extent = Extent;
        // 视口与剪裁
        VkPipelineViewportStateCreateInfo ViewportStateCreateInfo{};
        ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        ViewportStateCreateInfo.viewportCount = 1;
        ViewportStateCreateInfo.pViewports = &Viewport;
        ViewportStateCreateInfo.scissorCount = 1;
        ViewportStateCreateInfo.pScissors = &Scissor;

        // 光栅化
        VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo{};
        RasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        RasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        RasterizationStateCreateInfo.lineWidth = 1.0f;
        RasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        RasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        RasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;

        // 多重采样
        VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo{};
        MultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        MultisampleStateCreateInfo.rasterizationSamples = mDevice->GetMsaaSampleCount();
        MultisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
        MultisampleStateCreateInfo.minSampleShading = 0.2f;

        // 顶点输入描述
        VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo{};
        VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        VertexInputStateCreateInfo.pVertexBindingDescriptions = &info.VertexInputBindingDescription;
        VertexInputStateCreateInfo.vertexAttributeDescriptionCount = info.VertexInputAttributeDescriptionList.size();
        VertexInputStateCreateInfo.pVertexAttributeDescriptions = info.VertexInputAttributeDescriptionList.data();

        // 图元装配
        VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo{};
        InputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        InputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        InputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        // 深度与模板测试
        VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo{};
        DepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        DepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        DepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        DepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        DepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        // 附件颜色混合
        VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
        ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        ColorBlendAttachmentState.blendEnable = VK_TRUE;
        ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        // 全局颜色混合，基于位运算
        VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo{};
        ColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        ColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        ColorBlendStateCreateInfo.attachmentCount = 1;
        ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentState;

        // 模型图形管线创建信息
        VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo{};
        GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        GraphicsPipelineCreateInfo.layout = descriptorSet->GetPipelineLayout();
        GraphicsPipelineCreateInfo.renderPass = mDevice->GetRenderPass();
        GraphicsPipelineCreateInfo.subpass = 0;
        GraphicsPipelineCreateInfo.stageCount = ShaderStageCreateInfoList.size();
        GraphicsPipelineCreateInfo.pStages = ShaderStageCreateInfoList.data();
        GraphicsPipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
        GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationStateCreateInfo;
        GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleStateCreateInfo;
        GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
        GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyStateCreateInfo;
        GraphicsPipelineCreateInfo.pDepthStencilState = &DepthStencilStateCreateInfo;
        GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
        vkCreateGraphicsPipelines(mDevice->GetLogicalDevice(), nullptr, 1, &GraphicsPipelineCreateInfo, nullptr, &mPipeline);
    }
} // namespace vk
