#pragma once
#include "Origin.h"
#include "Window.h"

namespace vk
{
    class Device
    {
    public:
        Device(Window::Ptr window);
        ~Device();

        using Ptr = std::shared_ptr<Device>;
        static Ptr New(Window::Ptr window) { return std::make_shared<Device>(window); }

    private:
        // Vulkan实例
        VkInstance mInstance = nullptr;
        // 窗口表面
        VkSurfaceKHR mSurface = nullptr;
        // 物理设备
        VkPhysicalDevice mPhysicalDevice = nullptr;
        // 逻辑设备
        VkDevice mLogicalDevice = nullptr;
        uint32_t mGraphicsQueueFamilyIndex = 0;
        uint32_t mPresentQueueFamilyIndex = 0;
        VkQueue mGraphicsQueue = nullptr;
        VkQueue mPresentQueue = nullptr;
        // 交换链
        VkSwapchainKHR mSwapchain = nullptr;
        uint32_t mSwapchainMinImageCount = 0;
        uint32_t mSwapchainImageCount = 0;
        VkFormat mSwapchainImageFormat{};
        VkExtent2D mSwapchainImageExtent{};
        std::vector<VkImage> mSwapchainImageList;
        std::vector<VkImageView> mSwapchainImageViewList;
        // 颜色缓冲区
        VkSampleCountFlagBits mMsaaSampleCount{};
        VkImage mColorImage = nullptr;
        VkDeviceMemory mColorMemory = nullptr;
        VkImageView mColorImageView = nullptr;
        // 深度缓冲区
        VkFormat mDepthFormat{};
        VkImage mDepthImage = nullptr;
        VkDeviceMemory mDepthMemory = nullptr;
        VkImageView mDepthImageView = nullptr;
        // 渲染流程
        VkRenderPass mRenderPass = nullptr;
        // 帧缓冲区
        std::vector<VkFramebuffer> mFrameBufferList;
        // 命令池
        VkCommandPool mCommandPool = nullptr;

    private:
        void VolkInit();
        void CreateInstance(Window::Ptr window);
        void CreateSurface(Window::Ptr window);
        void EnumerationPhysicalDevice();
        void CreateLogicalDevice();
        void CreateSwapchain(Window::Ptr window);
        void CreateFrameImageView();
        void CreateRenderPass();
        void CreateFrameBuffer();
        void CreateCommandPool();

    public:
        VkInstance GetInstance() { return mInstance; }
        VkPhysicalDevice GetPhysicalDevice() { return mPhysicalDevice; }
        VkDevice GetLogicalDevice() { return mLogicalDevice; }
        VkSwapchainKHR GetSwapchain() { return mSwapchain; }
        VkExtent2D GetSwapchainImageExtent() { return mSwapchainImageExtent; }
        VkRenderPass GetRenderPass() { return mRenderPass; }
        VkFramebuffer GetFrameBuffer(uint32_t frameIndex) { return mFrameBufferList[frameIndex]; }
        VkQueue GetGraphicsQueue() { return mGraphicsQueue; }
        VkQueue GetPresentQueue() { return mPresentQueue; }
        uint32_t GetSwapchainImageCount() { return mSwapchainImageCount; }
        VkCommandPool GetCommandPool() { return mCommandPool; }
        VkSampleCountFlagBits GetMsaaSampleCount() { return mMsaaSampleCount; }
        uint32_t GetGraphicsQueueFamilyIndex() { return mGraphicsQueueFamilyIndex; }
        uint32_t GetSwapchainMinImageCount() { return mSwapchainMinImageCount; }

        bool DeviceWaitIdle();
        bool AllocateMemory(VkMemoryRequirements memoryRequirements, uint32_t memoryTypeIndex, VkDeviceMemory *memory);
        bool CreateImageView(VkImage image, VkFormat format, VkImageViewType viewType,
                             VkImageAspectFlags aspectFlags, uint32_t levelCount, uint32_t layerCount, VkImageView *imageView);
        bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint64_t size);
        bool CopyImage(VkImage srcImage, VkImageAspectFlags srcAspectFlags, uint32_t srcMipLevel, uint32_t srcLayerCount, VkImageLayout srcImageLayout,
                       VkImage dstImage, VkImageAspectFlags dstAspectFlags, uint32_t dstMipLevel, uint32_t dstLayerCount, VkImageLayout dstImageLayout,
                       uint32_t width, uint32_t height);
        bool CopyBufferToImage(VkBuffer srcBuffer,
                               VkImage dstImage, VkImageAspectFlags dstAspectFlags, uint32_t dstMipLevel, uint32_t dstLayerCount, VkImageLayout dstImageLayout,
                               uint32_t width, uint32_t height);
        bool CreateDisposableCommandBuffer(VkCommandBuffer *commandBuffer);
        bool EndDisposableCommandBuffer(VkCommandBuffer *commandBuffer);
        void GetMaxUsableSampleCount(VkSampleCountFlagBits *sampleCount);
        bool EnumerationSupportedFormats(std::vector<VkFormat> formatList, VkImageTiling imageTiling, VkFormatFeatureFlags formatFeatureFlags, VkFormat *format);
        bool QueryMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags properties, uint32_t *memoryTypeIndex);
        bool CreateImage(uint32_t width, uint32_t height,
                         VkFormat format, VkImageType imageType, VkSampleCountFlagBits numSamples,
                         VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                         uint32_t mipLevels, uint32_t layerCount,
                         VkImage *image, VkDeviceMemory *imageMemory);
        bool CreateBuffer(uint64_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory);
        bool TransitionImageLayout(VkImage image, VkImageAspectFlags aspectFlags, uint32_t levelCount, uint32_t layerCount, VkImageLayout oldLayout, VkImageLayout newLayout);
        bool GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t levelCount);
        bool CreateShaderModule(std::string shaderFilePath, VkShaderModule *shaderModule);
    };
} // namespace vk
