#include "vk/Renderer.h"

namespace vk
{
    Renderer::Renderer(Device::Ptr device)
        : mDevice(device)
    {
        AllocateCommandBuffer();
        CreateSyncObjects();
    }
    Renderer::~Renderer()
    {
        // 围栏
        for (auto &&i : mFenceList)
        {
            if (i != nullptr)
            {
                vkDestroyFence(mDevice->GetLogicalDevice(), i, nullptr);
            }
        }
        // 信号
        for (auto &&i : mStartRenderList)
        {
            if (i != nullptr)
            {
                vkDestroySemaphore(mDevice->GetLogicalDevice(), i, nullptr);
            }
        }
        for (auto &&i : mSubmitPresentList)
        {
            if (i != nullptr)
            {
                vkDestroySemaphore(mDevice->GetLogicalDevice(), i, nullptr);
            }
        }
    }

    void Renderer::AllocateCommandBuffer()
    {
        mCommandBufferList.resize(mDevice->GetSwapchainImageCount());
        VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
        CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        CommandBufferAllocateInfo.commandPool = mDevice->GetCommandPool();
        CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // 可以直接提交，但是不能被其他命令缓冲区调用。
        CommandBufferAllocateInfo.commandBufferCount = mCommandBufferList.size();
        if (vkAllocateCommandBuffers(mDevice->GetLogicalDevice(), &CommandBufferAllocateInfo, mCommandBufferList.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffer!");
        }
    }
    void Renderer::CreateSyncObjects()
    {
        mStartRenderList.resize(mDevice->GetSwapchainImageCount());
        mSubmitPresentList.resize(mDevice->GetSwapchainImageCount());
        mFenceList.resize(mDevice->GetSwapchainImageCount());

        VkSemaphoreCreateInfo SemaphoreCreateInfo{};
        SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo FenceCreateInfo{};
        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 初始点亮

        for (size_t i = 0; i < mDevice->GetSwapchainImageCount(); i++)
        {
            if (vkCreateSemaphore(mDevice->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &mStartRenderList[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create start rendering signal!");
            }
            if (vkCreateSemaphore(mDevice->GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &mSubmitPresentList[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create start rendering signal!");
            }
            if (vkCreateFence(mDevice->GetLogicalDevice(), &FenceCreateInfo, nullptr, &mFenceList[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create fence!");
            }
        }
    }
    void Renderer::Render(std::function<void(uint32_t)> drawOperations)
    {
        // 等待同步信号
        vkWaitForFences(mDevice->GetLogicalDevice(), 1, &mFenceList[mCurrentIndex], VK_TRUE, UINT64_MAX);
        // 重置同步信号状态
        vkResetFences(mDevice->GetLogicalDevice(), 1, &mFenceList[mCurrentIndex]);

        // 写入命令缓冲区
        {
            // 重置命令缓冲区
            vkResetCommandBuffer(mCommandBufferList[mCurrentIndex], 0);

            // 开始写入命令到命令缓冲区
            VkCommandBufferBeginInfo CommandBufferBeginInfo{};
            CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            vkBeginCommandBuffer(mCommandBufferList[mCurrentIndex], &CommandBufferBeginInfo);

            // 开始记录渲染步骤的命令
            std::array<VkClearValue, 2> ClearValueList{}; // 定义填充值，与附件参考一一对应
            ClearValueList[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            ClearValueList[1].depthStencil = {1.0f, 0};
            VkRenderPassBeginInfo RenderPassBeginInfo{};
            RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassBeginInfo.renderPass = mDevice->GetRenderPass();
            RenderPassBeginInfo.framebuffer = mDevice->GetFrameBuffer(mCurrentIndex);
            RenderPassBeginInfo.renderArea.offset = {0, 0};
            RenderPassBeginInfo.renderArea.extent = mDevice->GetSwapchainImageExtent();
            RenderPassBeginInfo.clearValueCount = ClearValueList.size();
            RenderPassBeginInfo.pClearValues = ClearValueList.data();
            vkCmdBeginRenderPass(mCommandBufferList[mCurrentIndex], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            drawOperations(mCurrentIndex);

            // 结束记录渲染步骤的命令
            vkCmdEndRenderPass(mCommandBufferList[mCurrentIndex]);
            // 结束写入命令到命令缓冲区
            vkEndCommandBuffer(mCommandBufferList[mCurrentIndex]);
        }

        // 获取交换链下一帧索引
        uint32_t FrameIndex = 0;
        vkAcquireNextImageKHR(mDevice->GetLogicalDevice(), mDevice->GetSwapchain(), UINT64_MAX, mStartRenderList[mCurrentIndex], nullptr, &FrameIndex);

        // 信号组
        VkSemaphore StartRenderS[] = {mStartRenderList[mCurrentIndex]};
        VkSemaphore SubmitPresentS[] = {mSubmitPresentList[mCurrentIndex]};

        // 提交命令缓冲区
        {
            VkPipelineStageFlags PipelineStageFlagsS[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            VkSubmitInfo SubmitInfo{};
            SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            SubmitInfo.waitSemaphoreCount = 1;
            SubmitInfo.pWaitSemaphores = StartRenderS;
            SubmitInfo.pWaitDstStageMask = PipelineStageFlagsS;
            SubmitInfo.commandBufferCount = 1;
            SubmitInfo.pCommandBuffers = &mCommandBufferList[mCurrentIndex];
            SubmitInfo.signalSemaphoreCount = 1;
            SubmitInfo.pSignalSemaphores = SubmitPresentS;
            vkQueueSubmit(mDevice->GetGraphicsQueue(), 1, &SubmitInfo, mFenceList[mCurrentIndex]);
        }
        // 提交结果到交换链
        {
            // 提交结果
            VkSwapchainKHR SwapchainS[] = {mDevice->GetSwapchain()};
            VkPresentInfoKHR PresentInfo{};
            PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            PresentInfo.waitSemaphoreCount = 1;
            PresentInfo.pWaitSemaphores = SubmitPresentS;
            PresentInfo.swapchainCount = 1;
            PresentInfo.pSwapchains = SwapchainS;
            PresentInfo.pImageIndices = &FrameIndex;
            vkQueuePresentKHR(mDevice->GetPresentQueue(), &PresentInfo);
        }
        mCurrentIndex = (mCurrentIndex + 1) % mDevice->GetSwapchainImageCount();
    }

    void Renderer::BindPipeline(VkPipeline pipeline)
    {
        vkCmdBindPipeline(mCommandBufferList[mCurrentIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
    void Renderer::BindVertexBuffer(Buffer::Ptr vertexBuffer)
    {
        VkBuffer VertexBufferS[] = {vertexBuffer->GetBuffer()};
        uint64_t VertexBufferOffsetS[] = {0};
        vkCmdBindVertexBuffers(mCommandBufferList[mCurrentIndex], 0, 1, VertexBufferS, VertexBufferOffsetS);
    }
    void Renderer::BindIndexBuffer(Buffer::Ptr vertexIndexBuffer)
    {
        vkCmdBindIndexBuffer(mCommandBufferList[mCurrentIndex], vertexIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
    void Renderer::BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
    {
        vkCmdBindDescriptorSets(mCommandBufferList[mCurrentIndex],
                                VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                1, &descriptorSet, 0, nullptr);
        //
    }
    void Renderer::DrawIndexed(uint32_t vertexIndexCount)
    {
        vkCmdDrawIndexed(mCommandBufferList[mCurrentIndex], vertexIndexCount, 1, 0, 0, 0);
    }
    void Renderer::Draw(ModelBuffer::Ptr modelBuffer, DescriptorSet::Ptr descriptorSet, Pipeline::Ptr pipeline)
    {
        // 绑定渲染管线
        BindPipeline(pipeline->GetPipeline());
        // 绑定顶点缓冲区命令
        BindVertexBuffer(modelBuffer->GetVertexBuffer());
        // 绑定顶点索引缓冲区命令
        BindIndexBuffer(modelBuffer->GetVertexIndexBuffer());
        // 绑定描述符集命令
        BindDescriptorSet(descriptorSet->GetPipelineLayout(), descriptorSet->GetDescriptorSet(mCurrentIndex));
        // 使用带顶点索引的渲染图形命令
        DrawIndexed(modelBuffer->GetVertexIndexCount());
    }
    void Renderer::DrawGUI(Gui::Ptr gui)
    {
        gui->Draw(mCommandBufferList[mCurrentIndex]);
    }
} // namespace vk
