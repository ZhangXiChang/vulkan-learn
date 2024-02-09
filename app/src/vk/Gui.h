#pragma once
#include "Origin.h"
#include "Device.h"

namespace vk
{
    class Gui
    {
    public:
        Gui(Device::Ptr device, Window::Ptr window);
        ~Gui();

        using Ptr = std::shared_ptr<Gui>;
        static Ptr MakePtr(Device::Ptr device, Window::Ptr window) { return std::make_shared<Gui>(device, window); }

        static void ProcessEvent(SDL_Event *event);
        static void Render(Window::Ptr window, std::function<void()> callback);

    private:
        Device::Ptr mDevice;
        VkDescriptorPool mImGuiDescriptorPool = nullptr;

    private:
        void CreateGui(Window::Ptr window);

    public:
        void Draw(VkCommandBuffer commandBuffer);
    };
} // namespace vk
