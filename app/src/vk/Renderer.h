#pragma once
#include "Origin.h"
#include "Device.h"
#include "Gui.h"
#include "Pipeline.h"
#include "DescriptorSet.h"
#include "ModelBuffer.h"

namespace vk
{
    class Renderer
    {
    public:
        Renderer(Device::Ptr device);
        ~Renderer();

        using Ptr = std::shared_ptr<Renderer>;
        static Ptr MakePtr(Device::Ptr device) { return std::make_shared<Renderer>(device); }

    private:
        Device::Ptr mDevice;
        // 命令缓冲区
        std::vector<VkCommandBuffer> mCommandBufferList;
        // 围栏
        std::vector<VkFence> mFenceList;
        // 信号
        std::vector<VkSemaphore> mStartRenderList;
        std::vector<VkSemaphore> mSubmitPresentList;
        // 多帧渲染资源索引
        uint32_t mCurrentIndex = 0;

    private:
        void AllocateCommandBuffer();
        void CreateSyncObjects();

        // 绑定渲染管线
        void BindPipeline(VkPipeline pipeline);
        // 绑定顶点缓冲区命令
        void BindVertexBuffer(Buffer::Ptr vertexBuffer);
        // 绑定顶点索引缓冲区命令
        void BindIndexBuffer(Buffer::Ptr vertexIndexBuffer);
        // 绑定描述符集命令
        void BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
        // 使用带顶点索引的渲染图形命令
        void DrawIndexed(uint32_t vertexIndexCount);

    public:
        void Render(std::function<void(uint32_t)> drawOperations);

        void Draw(ModelBuffer::Ptr modelBuffer, DescriptorSet::Ptr descriptorSet, Pipeline::Ptr pipeline);
        void DrawGUI(Gui::Ptr gui);
    };
} // namespace vk
