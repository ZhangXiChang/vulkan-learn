#include "vk/Device.h"

namespace vk
{
    Device::Device(Window::Ptr window)
    {
        VolkInit();
        CreateInstance(window);
        CreateSurface(window);
        EnumerationPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain(window);
        CreateFrameImageView();
        CreateRenderPass();
        CreateFrameBuffer();
        CreateCommandPool();
    }
    Device::~Device()
    {
        // 命令池
        if (mCommandPool != nullptr)
        {
            vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
        }
        // 帧缓冲区
        for (auto &&i : mFrameBufferList)
        {
            if (i != nullptr)
            {
                vkDestroyFramebuffer(mLogicalDevice, i, nullptr);
            }
        }
        // 渲染流程
        if (mRenderPass != nullptr)
        {
            vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);
        }
        // 深度缓冲区
        if (mDepthImageView != nullptr)
        {
            vkDestroyImageView(mLogicalDevice, mDepthImageView, nullptr);
        }
        if (mDepthMemory != nullptr)
        {
            vkFreeMemory(mLogicalDevice, mDepthMemory, nullptr);
        }
        if (mDepthImage != nullptr)
        {
            vkDestroyImage(mLogicalDevice, mDepthImage, nullptr);
        }
        // 颜色缓冲区
        if (mColorImageView != nullptr)
        {
            vkDestroyImageView(mLogicalDevice, mColorImageView, nullptr);
        }
        if (mColorMemory != nullptr)
        {
            vkFreeMemory(mLogicalDevice, mColorMemory, nullptr);
        }
        if (mColorImage != nullptr)
        {
            vkDestroyImage(mLogicalDevice, mColorImage, nullptr);
        }
        // 交换链
        for (auto &&i : mSwapchainImageViewList)
        {
            if (i != nullptr)
            {
                vkDestroyImageView(mLogicalDevice, i, nullptr);
            }
        }
        if (mSwapchain != nullptr)
        {
            vkDestroySwapchainKHR(mLogicalDevice, mSwapchain, nullptr);
        }
        // 逻辑设备
        if (mLogicalDevice != nullptr)
        {
            vkDestroyDevice(mLogicalDevice, nullptr);
        }
        // 窗口表面
        if (mSurface != nullptr)
        {
            vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        }
        // Vulkan实例
        if (mInstance != nullptr)
        {
            vkDestroyInstance(mInstance, nullptr);
        }
    }

    void Device::VolkInit()
    {
        if (volkInitialize() != VK_SUCCESS)
        {
            throw std::runtime_error("Volk initialization failed!");
        }
    }
    void Device::CreateInstance(Window::Ptr window)
    {
        VkApplicationInfo ApplicationInfo{};
        ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        ApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        ApplicationInfo.pEngineName = "null";
        ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        ApplicationInfo.pApplicationName = "null";
        ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

        uint32_t InstanceExtensionCount = 0;
        if (SDL_Vulkan_GetInstanceExtensions(window->GetWindow(), &InstanceExtensionCount, nullptr) != SDL_TRUE)
        {
            throw std::runtime_error("Failed to get the extension of vulkan instance!");
        }
        std::vector<const char *> InstanceExtensionList(InstanceExtensionCount);
        SDL_Vulkan_GetInstanceExtensions(window->GetWindow(), &InstanceExtensionCount, InstanceExtensionList.data());
        VkInstanceCreateInfo InstanceCreateInfo{};
        InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
        InstanceCreateInfo.enabledExtensionCount = InstanceExtensionList.size();
        InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensionList.data();
        if (vkCreateInstance(&InstanceCreateInfo, nullptr, &mInstance) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create a vulkan instance!");
        }
        // Volk
        volkLoadInstance(mInstance);
    }
    void Device::CreateSurface(Window::Ptr window)
    {
        if (SDL_Vulkan_CreateSurface(window->GetWindow(), mInstance, &mSurface) != SDL_TRUE)
        {
            throw std::runtime_error("Failed to create window surface!");
        }
    }
    void Device::EnumerationPhysicalDevice()
    {
        uint32_t PhysicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &PhysicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> PhysicalDeviceList(PhysicalDeviceCount);
        if (vkEnumeratePhysicalDevices(mInstance, &PhysicalDeviceCount, PhysicalDeviceList.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to enumerate physical devices!");
        }
        mPhysicalDevice = PhysicalDeviceList[0];
        GetMaxUsableSampleCount(&mMsaaSampleCount);
    }
    void Device::CreateLogicalDevice()
    {
        // 枚举设备队列
        uint32_t QueueFamilyPropertieCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &QueueFamilyPropertieCount, nullptr);
        std::vector<VkQueueFamilyProperties> QueueFamilyPropertieList(QueueFamilyPropertieCount);
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &QueueFamilyPropertieCount, QueueFamilyPropertieList.data());
        std::optional<uint32_t> oGraphicsQueueFamilyIndex;
        std::optional<uint32_t> oPresentQueueFamilyIndex;
        uint32_t QueueFamilyIndex = 0;
        for (auto &&i : QueueFamilyPropertieList)
        {
            // 图形队列族
            if (i.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                oGraphicsQueueFamilyIndex = QueueFamilyIndex;
            }
            // 显示队列族
            VkBool32 IsSurfaceSupport = 0;
            vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, QueueFamilyIndex, mSurface, &IsSurfaceSupport);
            if (IsSurfaceSupport)
            {
                oPresentQueueFamilyIndex = QueueFamilyIndex;
            }
            QueueFamilyIndex++;
        }
        if (!(oGraphicsQueueFamilyIndex.has_value() && oPresentQueueFamilyIndex.has_value()))
        {
            throw std::runtime_error("No suitable queue family index found!");
        }
        mGraphicsQueueFamilyIndex = oGraphicsQueueFamilyIndex.value();
        mPresentQueueFamilyIndex = oPresentQueueFamilyIndex.value();

        // 创建逻辑设备
        VkPhysicalDeviceFeatures PhysicalDeviceFeatures{};
        PhysicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
        PhysicalDeviceFeatures.sampleRateShading = VK_TRUE;

        std::vector<const char *> DeviceExtensionList = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        std::vector<VkDeviceQueueCreateInfo> QueueCreateInfoList;
        std::set<uint32_t> sQueueFamilyIndexList = {
            mGraphicsQueueFamilyIndex,
            mPresentQueueFamilyIndex,
        };
        float QueuePrioritie = 1.0f;
        for (auto &&i : sQueueFamilyIndexList)
        {
            VkDeviceQueueCreateInfo QueueCreateInfo{};
            QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            QueueCreateInfo.pQueuePriorities = &QueuePrioritie;
            QueueCreateInfo.queueCount = 1;
            QueueCreateInfo.queueFamilyIndex = i;
            QueueCreateInfoList.push_back(QueueCreateInfo);
        }
        VkDeviceCreateInfo DeviceCreateInfo{};
        DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;
        DeviceCreateInfo.enabledExtensionCount = DeviceExtensionList.size();
        DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensionList.data();
        DeviceCreateInfo.queueCreateInfoCount = QueueCreateInfoList.size();
        DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfoList.data();
        if (vkCreateDevice(mPhysicalDevice, &DeviceCreateInfo, nullptr, &mLogicalDevice) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create logical device!");
        }
        // Volk
        volkLoadDevice(mLogicalDevice);

        // 获取队列句柄
        vkGetDeviceQueue(mLogicalDevice, mGraphicsQueueFamilyIndex, 0, &mGraphicsQueue);
        vkGetDeviceQueue(mLogicalDevice, mPresentQueueFamilyIndex, 0, &mPresentQueue);
    }
    void Device::CreateSwapchain(Window::Ptr window)
    {
        // 获取窗口表面颜色格式列表
        uint32_t formatCount = 0;
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to get the window surface color format!");
        }
        std::vector<VkSurfaceFormatKHR> SurfaceFormatList(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, SurfaceFormatList.data());

        // 获取窗口表面呈现模式列表
        uint32_t presentModeCount = 0;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to get the list of window surface rendering modes!");
        }
        std::vector<VkPresentModeKHR> PresentModeList(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, PresentModeList.data());

        // 获取窗口表面参数
        VkSurfaceCapabilitiesKHR SurfaceCapabilities;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &SurfaceCapabilities) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to get window surface parameters!");
        }

        // 枚举适合交换链的颜色格式
        VkSurfaceFormatKHR SwapchainFormat;
        std::optional<VkSurfaceFormatKHR> oSwapchainFormat;
        for (auto &&i : SurfaceFormatList)
        {
            if (i.format == VK_FORMAT_B8G8R8A8_SRGB && i.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                oSwapchainFormat = i;
                break;
            }
        }
        if (!oSwapchainFormat.has_value())
        {
            throw std::runtime_error("No suitable exchange chain format found!");
        }
        SwapchainFormat = oSwapchainFormat.value();

        // 枚举适合交换链的呈现模式
        VkPresentModeKHR SwapchainPresentMode;
        std::optional<VkPresentModeKHR> oSwapchainPresentMode;
        for (auto &&i : PresentModeList)
        {
            if (i == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                oSwapchainPresentMode = i;
                break;
            }
        }
        if (!oSwapchainPresentMode.has_value())
        {
            throw std::runtime_error("No suitable exchange chain program mode found!");
        }
        SwapchainPresentMode = oSwapchainPresentMode.value();

        // 矫正窗口表面参数以适应交换链
        VkExtent2D SwapchainExtent;
        uint32_t SwapchainMinImageCount;
        // 获取窗口大小
        int winWidth, winHeight;
        window->GetFrameBufferSize(&winWidth, &winHeight);
        VkExtent2D winExtent = {
            (uint32_t)winWidth,
            (uint32_t)winHeight,
        };
        // 钳制窗口范围在窗口表面限制范围内
        winExtent.width = std::clamp(winExtent.width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
        winExtent.height = std::clamp(winExtent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);
        SwapchainExtent = winExtent;
        // 钳制交换链最小帧缓冲数不能大于最大帧缓冲数
        SwapchainMinImageCount = SurfaceCapabilities.minImageCount;
        if (SurfaceCapabilities.maxImageCount > 0 && SwapchainMinImageCount > SurfaceCapabilities.maxImageCount)
        {
            SwapchainMinImageCount = SurfaceCapabilities.maxImageCount;
        }

        // 创建交换链
        VkSwapchainCreateInfoKHR SwapchainCreateInfo{};
        SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        SwapchainCreateInfo.oldSwapchain = nullptr;
        SwapchainCreateInfo.imageArrayLayers = 1;
        SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        SwapchainCreateInfo.clipped = VK_TRUE;
        SwapchainCreateInfo.surface = mSurface;
        SwapchainCreateInfo.imageFormat = SwapchainFormat.format;
        SwapchainCreateInfo.imageColorSpace = SwapchainFormat.colorSpace;
        SwapchainCreateInfo.presentMode = SwapchainPresentMode;
        SwapchainCreateInfo.imageExtent = SwapchainExtent;
        SwapchainCreateInfo.minImageCount = SwapchainMinImageCount;
        SwapchainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;

        // 设定呈现队列是否共享
        std::set<uint32_t> sQueueIndexList = {
            mGraphicsQueueFamilyIndex,
            mPresentQueueFamilyIndex,
        };
        std::vector<uint32_t> QueueIndexList;
        for (auto &&i : sQueueIndexList)
        {
            QueueIndexList.push_back(i);
        }
        if (QueueIndexList.size() == 1)
        {
            SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        else
        {
            SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            SwapchainCreateInfo.queueFamilyIndexCount = QueueIndexList.size();
            SwapchainCreateInfo.pQueueFamilyIndices = QueueIndexList.data();
        }
        if (vkCreateSwapchainKHR(mLogicalDevice, &SwapchainCreateInfo, nullptr, &mSwapchain) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create exchange chain!");
        }

        // 创建交换链视图
        uint32_t ImageCount = 0;
        if (vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &ImageCount, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to get the exchange chain image!");
        }
        mSwapchainImageList.resize(ImageCount);
        vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &ImageCount, mSwapchainImageList.data());
        for (auto &&i : mSwapchainImageList)
        {
            VkImageView imageView = nullptr;
            if (!CreateImageView(i, SwapchainFormat.format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, &imageView))
            {
                throw std::runtime_error("Failed to create exchange chain image view!");
            }
            mSwapchainImageViewList.push_back(imageView);
        }

        // 保存交换链信息
        mSwapchainMinImageCount = SwapchainMinImageCount;
        mSwapchainImageCount = ImageCount;
        mSwapchainImageFormat = SwapchainFormat.format;
        mSwapchainImageExtent = SwapchainExtent;
    }
    void Device::CreateFrameImageView()
    {
        // 创建颜色缓冲区
        if (!CreateImage(mSwapchainImageExtent.width, mSwapchainImageExtent.height,
                         mSwapchainImageFormat,
                         VK_IMAGE_TYPE_2D,
                         mMsaaSampleCount,
                         VK_IMAGE_TILING_OPTIMAL,
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         1, 1,
                         &mColorImage, &mColorMemory))
        {
            throw std::runtime_error("Failed to create frame color buffer!");
        }
        if (!CreateImageView(mColorImage, mSwapchainImageFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, &mColorImageView))
        {
            throw std::runtime_error("Failed to create frame color image view!");
        }

        // 枚举适合的深度格式
        if (!EnumerationSupportedFormats({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                         VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                         &mDepthFormat))
        {
            throw std::runtime_error("No suitable frame depth format found!");
        }
        // 创建深度缓冲区
        if (!CreateImage(mSwapchainImageExtent.width, mSwapchainImageExtent.height,
                         mDepthFormat,
                         VK_IMAGE_TYPE_2D,
                         mMsaaSampleCount,
                         VK_IMAGE_TILING_OPTIMAL,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         1, 1,
                         &mDepthImage, &mDepthMemory))
        {
            throw std::runtime_error("Failed to create frame depth buffer!");
        }
        if (!CreateImageView(mDepthImage, mDepthFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, &mDepthImageView))
        {
            throw std::runtime_error("Failed to create depth image view!");
        }
    }
    void Device::CreateRenderPass()
    {
        // 颜色附件描述
        VkAttachmentDescription ColorAttachmentDescription{};
        ColorAttachmentDescription.format = mSwapchainImageFormat;
        ColorAttachmentDescription.samples = mMsaaSampleCount;
        ColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        ColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ColorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 深度附件描述
        VkAttachmentDescription DepthAttachmentDescription{};
        DepthAttachmentDescription.format = mDepthFormat;
        DepthAttachmentDescription.samples = mMsaaSampleCount;
        DepthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        DepthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        DepthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        DepthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        DepthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        DepthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 交换链图像附件描述
        VkAttachmentDescription SwapchainImageAttachmentDescription{};
        SwapchainImageAttachmentDescription.format = mSwapchainImageFormat;
        SwapchainImageAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        SwapchainImageAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        SwapchainImageAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        SwapchainImageAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        SwapchainImageAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        SwapchainImageAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        SwapchainImageAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // 附件描述列表，与附件参考一一对应
        std::vector<VkAttachmentDescription> AttachmentDescriptionList = {
            ColorAttachmentDescription,
            DepthAttachmentDescription,
            SwapchainImageAttachmentDescription,
        };

        // 子流程颜色附件参考
        VkAttachmentReference ColorAttachmentReference{};
        ColorAttachmentReference.attachment = 0;
        ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 子流程深度附件参考
        VkAttachmentReference DepthAttachmentReference{};
        DepthAttachmentReference.attachment = 1;
        DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 交换链图像附件参考
        VkAttachmentReference SwapchainImageAttachmentReference{};
        SwapchainImageAttachmentReference.attachment = 2;
        SwapchainImageAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 子流程
        VkSubpassDescription SubpassDescription{};
        SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        SubpassDescription.colorAttachmentCount = 1;
        SubpassDescription.pColorAttachments = &ColorAttachmentReference;
        SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;
        SubpassDescription.pResolveAttachments = &SwapchainImageAttachmentReference;

        // 子流程列表
        std::vector<VkSubpassDescription> SubpassDescriptionList = {
            SubpassDescription,
        };

        // 子流程依赖关系
        VkSubpassDependency SubpassDependency{};
        SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency.dstSubpass = 0;
        SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        SubpassDependency.srcAccessMask = 0;
        SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // 子流程依赖关系列表
        std::vector<VkSubpassDependency> SubpassDependencyList = {
            SubpassDependency,
        };

        // 渲染目标和子渲染流程绑定的流程不在这里，而在命令录制模块进行。
        VkRenderPassCreateInfo RenderPassCreateInfo{};
        RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        RenderPassCreateInfo.attachmentCount = AttachmentDescriptionList.size();
        RenderPassCreateInfo.pAttachments = AttachmentDescriptionList.data();
        RenderPassCreateInfo.subpassCount = SubpassDescriptionList.size();
        RenderPassCreateInfo.pSubpasses = SubpassDescriptionList.data();
        RenderPassCreateInfo.dependencyCount = SubpassDependencyList.size();
        RenderPassCreateInfo.pDependencies = SubpassDependencyList.data();
        if (vkCreateRenderPass(mLogicalDevice, &RenderPassCreateInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Render channel creation failed!");
        }
    }
    void Device::CreateFrameBuffer()
    {
        // 创建帧缓冲区
        mFrameBufferList.resize(mSwapchainImageCount);
        for (size_t i = 0; i < mSwapchainImageCount; i++)
        {
            // 与渲染流程中的附件参考一一对应
            std::vector<VkImageView> AttachmentList = {
                mColorImageView,
                mDepthImageView,
                mSwapchainImageViewList[i],
            };
            VkFramebufferCreateInfo FramebufferCreateInfo{};
            FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            FramebufferCreateInfo.renderPass = mRenderPass;
            FramebufferCreateInfo.attachmentCount = AttachmentList.size();
            FramebufferCreateInfo.pAttachments = AttachmentList.data();
            FramebufferCreateInfo.width = mSwapchainImageExtent.width;
            FramebufferCreateInfo.height = mSwapchainImageExtent.height;
            FramebufferCreateInfo.layers = 1;
            if (vkCreateFramebuffer(mLogicalDevice, &FramebufferCreateInfo, nullptr, &mFrameBufferList[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Frame buffer creation failed!");
            }
        }
    }
    void Device::CreateCommandPool()
    {
        VkCommandPoolCreateInfo CommandPoolCreateInfo{};
        CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        CommandPoolCreateInfo.queueFamilyIndex = mGraphicsQueueFamilyIndex;
        CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(mLogicalDevice, &CommandPoolCreateInfo, nullptr, &mCommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    bool Device::DeviceWaitIdle()
    {
        if (vkDeviceWaitIdle(mLogicalDevice) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    bool Device::AllocateMemory(VkMemoryRequirements memoryRequirements, uint32_t memoryTypeIndex, VkDeviceMemory *memory)
    {
        VkMemoryAllocateInfo MemoryAllocateInfo{};
        MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        MemoryAllocateInfo.allocationSize = memoryRequirements.size;
        MemoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        if (vkAllocateMemory(mLogicalDevice, &MemoryAllocateInfo, nullptr, memory) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    bool Device::CreateImageView(VkImage image, VkFormat format, VkImageViewType viewType,
                                 VkImageAspectFlags aspectFlags, uint32_t levelCount, uint32_t layerCount, VkImageView *imageView)
    {
        VkImageViewCreateInfo imaviecreInfo{};
        imaviecreInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imaviecreInfo.image = image;
        imaviecreInfo.format = format;
        imaviecreInfo.viewType = viewType;
        imaviecreInfo.subresourceRange.aspectMask = aspectFlags;
        imaviecreInfo.subresourceRange.levelCount = levelCount;
        imaviecreInfo.subresourceRange.layerCount = layerCount;
        if (vkCreateImageView(mLogicalDevice, &imaviecreInfo, nullptr, imageView) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    bool Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint64_t size)
    {
        VkCommandBuffer CommandBuffer;
        if (!CreateDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }

        // 拷贝缓冲区命令
        VkBufferCopy BufferCopy{};
        BufferCopy.size = size;
        vkCmdCopyBuffer(CommandBuffer, srcBuffer, dstBuffer, 1, &BufferCopy);

        if (!EndDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }
        return true;
    }
    bool Device::CopyImage(VkImage srcImage, VkImageAspectFlags srcAspectFlags, uint32_t srcMipLevel, uint32_t srcLayerCount, VkImageLayout srcImageLayout,
                           VkImage dstImage, VkImageAspectFlags dstAspectFlags, uint32_t dstMipLevel, uint32_t dstLayerCount, VkImageLayout dstImageLayout,
                           uint32_t width, uint32_t height)
    {
        VkCommandBuffer CommandBuffer;
        if (!CreateDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }

        // 拷贝缓冲区命令
        VkImageCopy ImageCopy{};
        ImageCopy.srcOffset = {0, 0, 0};
        ImageCopy.srcSubresource.aspectMask = srcAspectFlags;
        ImageCopy.srcSubresource.baseArrayLayer = 0;
        ImageCopy.srcSubresource.mipLevel = srcMipLevel;
        ImageCopy.srcSubresource.layerCount = srcLayerCount;
        ImageCopy.dstOffset = {0, 0, 0};
        ImageCopy.dstSubresource.aspectMask = dstAspectFlags;
        ImageCopy.dstSubresource.baseArrayLayer = 0;
        ImageCopy.dstSubresource.mipLevel = dstMipLevel;
        ImageCopy.dstSubresource.layerCount = dstLayerCount;
        ImageCopy.extent = {width, height, 1};
        vkCmdCopyImage(CommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, 1, &ImageCopy);

        if (!EndDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }
        return true;
    }
    bool Device::CopyBufferToImage(VkBuffer srcBuffer,
                                   VkImage dstImage, VkImageAspectFlags dstAspectFlags, uint32_t dstMipLevel, uint32_t dstLayerCount, VkImageLayout dstImageLayout,
                                   uint32_t width, uint32_t height)
    {
        VkCommandBuffer CommandBuffer;
        if (!CreateDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }

        VkBufferImageCopy BufferImageCopy{};
        BufferImageCopy.bufferOffset = 0;
        BufferImageCopy.bufferRowLength = 0;
        BufferImageCopy.bufferImageHeight = 0;
        BufferImageCopy.imageOffset = {0, 0, 0};
        BufferImageCopy.imageSubresource.aspectMask = dstAspectFlags;
        BufferImageCopy.imageSubresource.baseArrayLayer = 0;
        BufferImageCopy.imageSubresource.mipLevel = dstMipLevel;
        BufferImageCopy.imageSubresource.layerCount = dstLayerCount;
        BufferImageCopy.imageExtent = {width, height, 1};
        vkCmdCopyBufferToImage(CommandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &BufferImageCopy);

        if (!EndDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }
        return true;
    }
    bool Device::CreateDisposableCommandBuffer(VkCommandBuffer *commandBuffer)
    {
        VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
        CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        CommandBufferAllocateInfo.commandPool = mCommandPool;
        CommandBufferAllocateInfo.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(mLogicalDevice, &CommandBufferAllocateInfo, commandBuffer) != VK_SUCCESS)
        {
            return false;
        }

        VkCommandBufferBeginInfo CommandBufferBeginInfo{};
        CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(*commandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    bool Device::EndDisposableCommandBuffer(VkCommandBuffer *commandBuffer)
    {
        if (vkEndCommandBuffer(*commandBuffer) != VK_SUCCESS)
        {
            return false;
        }

        VkSubmitInfo SubmitInfo{};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        SubmitInfo.commandBufferCount = 1;
        SubmitInfo.pCommandBuffers = commandBuffer;
        if (vkQueueSubmit(mGraphicsQueue, 1, &SubmitInfo, nullptr) != VK_SUCCESS)
        {
            return false;
        }

        if (vkQueueWaitIdle(mGraphicsQueue) != VK_SUCCESS)
        {
            return false;
        }
        vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, commandBuffer);
        return true;
    }
    void Device::GetMaxUsableSampleCount(VkSampleCountFlagBits *sampleCount)
    {
        VkPhysicalDeviceProperties PhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &PhysicalDeviceProperties);
        VkSampleCountFlags SampleCountFlags = PhysicalDeviceProperties.limits.framebufferColorSampleCounts & PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (SampleCountFlags & VK_SAMPLE_COUNT_64_BIT)
        {
            *sampleCount = VK_SAMPLE_COUNT_64_BIT;
            return;
        }
        if (SampleCountFlags & VK_SAMPLE_COUNT_32_BIT)
        {
            *sampleCount = VK_SAMPLE_COUNT_32_BIT;
            return;
        }
        if (SampleCountFlags & VK_SAMPLE_COUNT_16_BIT)
        {
            *sampleCount = VK_SAMPLE_COUNT_16_BIT;
            return;
        }
        if (SampleCountFlags & VK_SAMPLE_COUNT_8_BIT)
        {
            *sampleCount = VK_SAMPLE_COUNT_8_BIT;
            return;
        }
        if (SampleCountFlags & VK_SAMPLE_COUNT_4_BIT)
        {
            *sampleCount = VK_SAMPLE_COUNT_4_BIT;
            return;
        }
        if (SampleCountFlags & VK_SAMPLE_COUNT_2_BIT)
        {
            *sampleCount = VK_SAMPLE_COUNT_2_BIT;
            return;
        }
        *sampleCount = VK_SAMPLE_COUNT_1_BIT;
    }

    bool Device::EnumerationSupportedFormats(std::vector<VkFormat> formatList, VkImageTiling imageTiling, VkFormatFeatureFlags formatFeatureFlags, VkFormat *format)
    {
        for (auto &&i : formatList)
        {
            VkFormatProperties FormatProperties;
            vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, i, &FormatProperties);
            if (imageTiling == VK_IMAGE_TILING_LINEAR && (FormatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
            {
                *format = i;
                break;
            }
            if (imageTiling == VK_IMAGE_TILING_OPTIMAL && (FormatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
            {
                *format = i;
                break;
            }
        }
        if (format == nullptr)
        {
            return false;
        }
        return true;
    }
    bool Device::QueryMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags properties, uint32_t *memoryTypeIndex)
    {
        std::optional<uint32_t> oMemoryTypeIndex;
        VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &PhysicalDeviceMemoryProperties);
        for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                oMemoryTypeIndex = i;
                break;
            }
        }
        if (!oMemoryTypeIndex.has_value())
        {
            return false;
        }
        *memoryTypeIndex = oMemoryTypeIndex.value();
        return true;
    }
    bool Device::CreateImage(uint32_t width, uint32_t height,
                             VkFormat format, VkImageType imageType, VkSampleCountFlagBits numSamples,
                             VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                             uint32_t mipLevels, uint32_t layerCount,
                             VkImage *image, VkDeviceMemory *imageMemory)
    {
        VkImageCreateInfo ImageCreateInfo{};
        ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ImageCreateInfo.extent.width = width;
        ImageCreateInfo.extent.height = height;
        ImageCreateInfo.extent.depth = 1;
        ImageCreateInfo.imageType = imageType;
        ImageCreateInfo.format = format;
        ImageCreateInfo.samples = numSamples;
        ImageCreateInfo.tiling = tiling;
        ImageCreateInfo.usage = usage;
        ImageCreateInfo.mipLevels = mipLevels;
        ImageCreateInfo.arrayLayers = layerCount;
        ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(mLogicalDevice, &ImageCreateInfo, nullptr, image) != VK_SUCCESS)
        {
            return false;
        }

        // 获取合适的内存类型索引
        VkMemoryRequirements MemoryRequirements;
        vkGetImageMemoryRequirements(mLogicalDevice, *image, &MemoryRequirements);
        uint32_t MemoryTypeIndex = 0;
        if (!QueryMemoryTypeIndex(MemoryRequirements, properties, &MemoryTypeIndex))
        {
            return false;
        }

        // 分配内存
        if (!AllocateMemory(MemoryRequirements, MemoryTypeIndex, imageMemory))
        {
            return false;
        }

        // 绑定内存
        vkBindImageMemory(mLogicalDevice, *image, *imageMemory, 0);
        return true;
    }
    bool Device::CreateBuffer(uint64_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
    {
        // 创建缓冲区
        VkBufferCreateInfo BufferCreateInfo{};
        BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        BufferCreateInfo.size = bufferSize;
        BufferCreateInfo.usage = usage;
        BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(mLogicalDevice, &BufferCreateInfo, nullptr, buffer) != VK_SUCCESS)
        {
            return false;
        }

        // 获取合适的内存类型索引
        VkMemoryRequirements MemoryRequirements;
        vkGetBufferMemoryRequirements(mLogicalDevice, *buffer, &MemoryRequirements);
        uint32_t MemoryTypeIndex = 0;
        if (!QueryMemoryTypeIndex(MemoryRequirements, properties, &MemoryTypeIndex))
        {
            return false;
        }

        // 分配内存
        if (!AllocateMemory(MemoryRequirements, MemoryTypeIndex, bufferMemory))
        {
            return false;
        }

        // 绑定内存
        if (vkBindBufferMemory(mLogicalDevice, *buffer, *bufferMemory, 0) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
    bool Device::TransitionImageLayout(VkImage image, VkImageAspectFlags aspectFlags, uint32_t levelCount, uint32_t layerCount, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer CommandBuffer;
        if (!CreateDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }

        VkImageMemoryBarrier ImageMemoryBarrier{};
        ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ImageMemoryBarrier.image = image;
        ImageMemoryBarrier.subresourceRange.aspectMask = aspectFlags;
        ImageMemoryBarrier.subresourceRange.levelCount = levelCount;
        ImageMemoryBarrier.subresourceRange.layerCount = layerCount;
        ImageMemoryBarrier.oldLayout = oldLayout;
        ImageMemoryBarrier.newLayout = newLayout;

        VkPipelineStageFlags SrcStageFlags;
        VkPipelineStageFlags DstStageFlags;
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            ImageMemoryBarrier.srcAccessMask = 0;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            SrcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            DstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            SrcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            DstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            SrcStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            DstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            SrcStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            DstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            SrcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            DstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            return false;
        }

        vkCmdPipelineBarrier(CommandBuffer,
                             SrcStageFlags, DstStageFlags, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &ImageMemoryBarrier);
        //

        if (!EndDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }
        return true;
    }
    bool Device::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t levelCount)
    {
        // 检查物理设备的指定格式是否支持生成mip
        VkFormatProperties FormatProperties;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, imageFormat, &FormatProperties);
        if (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            return false;
        }

        VkCommandBuffer CommandBuffer;
        if (!CreateDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }

        // 图像内存屏障模板
        VkImageMemoryBarrier ImageMemoryBarrier{};
        ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        ImageMemoryBarrier.image = image;
        ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        ImageMemoryBarrier.subresourceRange.layerCount = 1;
        ImageMemoryBarrier.subresourceRange.levelCount = 1;

        // 为所有mip级别生成图像，图像级别从大到小排序，越大的mip级别越接近原图，最后一个是原图
        int32_t mipWidth = width;
        int32_t mipHeight = height;
        for (uint32_t i = 1; i < levelCount; i++)
        {
            // 配置基于第几个mip级别进行内存屏障
            ImageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;

            // 将图像布局从传输目标转为传输源，以及将访问掩码从传输写入转为传输读取
            ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(CommandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, // 不需要更改渲染管线阶段
                                 0, nullptr,
                                 0, nullptr,
                                 1, &ImageMemoryBarrier);
            //

            // 将前一级mip的图像缩小一半拷贝给后一级的mip
            VkImageBlit ImageBlit{};
            ImageBlit.srcOffsets[0] = {0, 0, 0};
            ImageBlit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            ImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ImageBlit.srcSubresource.mipLevel = i - 1;
            ImageBlit.srcSubresource.baseArrayLayer = 0;
            ImageBlit.srcSubresource.layerCount = 1;
            ImageBlit.dstOffsets[0] = {0, 0, 0};
            ImageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            ImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ImageBlit.dstSubresource.mipLevel = i;
            ImageBlit.dstSubresource.baseArrayLayer = 0;
            ImageBlit.dstSubresource.layerCount = 1;
            vkCmdBlitImage(CommandBuffer,
                           image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // 源图像和内存布局
                           image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // 目标图像和内存布局
                           1, &ImageBlit,
                           VK_FILTER_LINEAR); // 转换模式，例如线性滤波器
            //

            // 将图像布局从传输源转为仅着色器读取，以及将访问掩码从传输写入转为着色器读取
            ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(CommandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, // 将渲染管线阶段转换为着色器片元阶段
                                 0, nullptr,
                                 0, nullptr,
                                 1, &ImageMemoryBarrier);
            //

            // 获得下一mip级别的图像范围
            mipWidth /= 2;
            mipHeight /= 2;
        }
        // 将最后一个mip级别的图像从传输源转为仅着色器读取，以及将访问掩码从传输写入转为着色器读取
        ImageMemoryBarrier.subresourceRange.baseMipLevel = levelCount - 1;
        ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(CommandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &ImageMemoryBarrier);
        //

        if (!EndDisposableCommandBuffer(&CommandBuffer))
        {
            return false;
        }
        return true;
    }
    bool Device::CreateShaderModule(std::string shaderFilePath, VkShaderModule *shaderModule)
    {
        // 加载着色器代码
        std::vector<char> ShaderCode;
        std::fstream ShaderFile(shaderFilePath, std::ios::ate | std::ios::binary | std::ios::in);
        if (!ShaderFile.is_open())
        {
            return false;
        }
        size_t ShaderFileSize = ShaderFile.tellg();
        ShaderFile.seekg(0);
        ShaderCode.resize(ShaderFileSize, 0);
        ShaderFile.read(ShaderCode.data(), ShaderFileSize);
        ShaderFile.close();
        // 创建着色器模块
        VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
        ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ShaderModuleCreateInfo.codeSize = ShaderCode.size();
        ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(ShaderCode.data());
        if (vkCreateShaderModule(mLogicalDevice, &ShaderModuleCreateInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }
} // namespace vk
