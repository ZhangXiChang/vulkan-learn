#include "vk/ShaderModule.h"

namespace vk
{
    ShaderModule::ShaderModule(Device::Ptr device, VkShaderStageFlagBits shaderStage, std::string shaderFilePath)
        : mDevice(device), mShaderStage(shaderStage)
    {
        CreateShaderModule(shaderFilePath);
    }
    ShaderModule::~ShaderModule()
    {
        vkDestroyShaderModule(mDevice->GetLogicalDevice(), mShaderModule, nullptr);
    }

    void ShaderModule::CreateShaderModule(std::string shaderFilePath)
    {
        if (!mDevice->CreateShaderModule(shaderFilePath, &mShaderModule))
        {
            return;
        }
    }
} // namespace vk
