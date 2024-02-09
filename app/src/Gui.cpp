#include "vk/Gui.h"

namespace vk
{
    Gui::Gui(Device::Ptr device, Window::Ptr window)
        : mDevice(device)
    {
        CreateGui(window);
    }
    Gui::~Gui()
    {
        // ImGui
        if (mImGuiDescriptorPool != nullptr)
        {
            vkDestroyDescriptorPool(mDevice->GetLogicalDevice(), mImGuiDescriptorPool, nullptr);
        }
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void Gui::CreateGui(Window::Ptr window)
    {
        // 创建ImGui上下文
        if (ImGui::CreateContext() == nullptr)
        {
            throw std::runtime_error("Failed to create ImGui context!");
        }
        if (window->GetIsHideMouse())
        {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        }

        // 使用Volk的Vulkan元加载
        if (!ImGui_ImplVulkan_LoadFunctions(
                [](const char *function_name, void *vulkan_instance)
                { return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance *>(vulkan_instance)), function_name); },
                mDevice->GetInstance()))
        {
            throw std::runtime_error("ImGUI using Volk's Vulkan meta load failed!");
        }

        // 初始化SDL2模块用于Vulkan模块
        if (!ImGui_ImplSDL2_InitForVulkan(window->GetWindow()))
        {
            throw std::runtime_error("Failed to initialize SDL2 module for Vulkan module!");
        }

        // 创建ImGui描述符池
        std::vector<VkDescriptorPoolSize> ImGuiDescriptorPoolSizeList =
            {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 100},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100},
            };

        VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo{};
        DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        DescriptorPoolCreateInfo.poolSizeCount = ImGuiDescriptorPoolSizeList.size();
        DescriptorPoolCreateInfo.pPoolSizes = ImGuiDescriptorPoolSizeList.data();
        DescriptorPoolCreateInfo.maxSets = 100;
        if (vkCreateDescriptorPool(mDevice->GetLogicalDevice(), &DescriptorPoolCreateInfo, nullptr, &mImGuiDescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGui descriptor pool!");
        }

        // 初始化Vulkan模块
        ImGui_ImplVulkan_InitInfo ImGuiInitInfo{};
        ImGuiInitInfo.Instance = mDevice->GetInstance();
        ImGuiInitInfo.PhysicalDevice = mDevice->GetPhysicalDevice();
        ImGuiInitInfo.Device = mDevice->GetLogicalDevice();
        ImGuiInitInfo.Queue = mDevice->GetGraphicsQueue();
        ImGuiInitInfo.QueueFamily = mDevice->GetGraphicsQueueFamilyIndex();
        ImGuiInitInfo.MinImageCount = mDevice->GetSwapchainMinImageCount();
        ImGuiInitInfo.ImageCount = mDevice->GetSwapchainImageCount();
        ImGuiInitInfo.DescriptorPool = mImGuiDescriptorPool;
        ImGuiInitInfo.MSAASamples = mDevice->GetMsaaSampleCount();
        ImGuiInitInfo.Subpass = 0;
        if (!ImGui_ImplVulkan_Init(&ImGuiInitInfo, mDevice->GetRenderPass()))
        {
            throw std::runtime_error("Failed to initialize ImGui's Vulkan module!");
        }

        // 加载ImGui字体纹理
        VkCommandBuffer CommandBuffer;
        if (!mDevice->CreateDisposableCommandBuffer(&CommandBuffer))
        {
            throw std::runtime_error("Failed to create buffer for loading ImGui font texture command line!");
        }
        if (!ImGui_ImplVulkan_CreateFontsTexture())
        {
            throw std::runtime_error("Failed to create ImGui font texture!");
        }
        if (!mDevice->EndDisposableCommandBuffer(&CommandBuffer))
        {
            throw std::runtime_error("Failed to end the command buffer used to load ImGui font texture!");
        }

        // 清除CPU端ImGui字体纹理缓存
        ImGui_ImplVulkan_DestroyFontsTexture();
    }
    void Gui::ProcessEvent(SDL_Event *event)
    {
        ImGui_ImplSDL2_ProcessEvent(event);
    }
    void Gui::Render(Window::Ptr window, std::function<void()> callback)
    {
        // ImGui创建新帧
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window->GetWindow());
        ImGui::NewFrame();
        callback();
        // ImGui渲染
        ImGui::Render();
    }
    void Gui::Draw(VkCommandBuffer commandBuffer)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
} // namespace vk
