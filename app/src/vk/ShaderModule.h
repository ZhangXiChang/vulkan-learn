#pragma once
#include "Origin.h"
#include "Device.h"

namespace vk
{
    class ShaderModule
    {
    public:
        ShaderModule(Device::Ptr device, VkShaderStageFlagBits shaderStage, std::string shaderFilePath);
        ~ShaderModule();

        using Ptr = std::shared_ptr<ShaderModule>;
        static Ptr MakePtr(Device::Ptr device, VkShaderStageFlagBits shaderStage, std::string shaderFilePath)
        {
            return std::make_shared<ShaderModule>(device, shaderStage, shaderFilePath);
        }

    private:
        Device::Ptr mDevice;
        VkShaderModule mShaderModule = nullptr;
        VkShaderStageFlagBits mShaderStage;

    private:
        void CreateShaderModule(std::string shaderFilePath);

    public:
        VkShaderModule GetShaderModule() { return mShaderModule; }
        VkShaderStageFlagBits GetShaderStage() { return mShaderStage; }
    };
} // namespace vk
