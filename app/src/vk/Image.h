#pragma once
#include "Origin.h"
#include "Device.h"
#include "Buffer.h"

namespace vk
{
    class Image
    {
    public:
        struct ImageInfo
        {
            std::string Name;
            int Width;
            int Height;
            void *Data;
            void Free() { free(Data); }
        };

    public:
        Image(Device::Ptr device, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        ~Image();

        using Ptr = std::shared_ptr<Image>;
        static Ptr MakePtr(Device::Ptr device, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
        {
            return std::make_shared<Image>(device, width, height, usage, properties);
        }

        static ImageInfo OpenImageFile(std::string filePath);

    private:
        Device::Ptr mDevice;
        // 图像
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        uint32_t mMipLevels = 0;
        uint32_t mLayerCount = 0;
        VkImage mImage = nullptr;
        VkDeviceMemory mImageMemory = nullptr;
        VkImageView mImageView = nullptr;

    private:
        void CreateImageBuffer(VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

    public:
        bool WriteImage(Image::Ptr image);
        bool WriteBuffer(Buffer::Ptr buffer);
        bool WriteData(void *data);

        uint32_t GetMipLevels() { return mMipLevels; }
        uint32_t GetLayerCount() { return mLayerCount; }
        VkImage GetImage() { return mImage; }
        VkImageView GetImageView() { return mImageView; }
        uint32_t GetWidht() { return mWidth; };
        uint32_t GetHeight() { return mHeight; };
    };
} // namespace vk
