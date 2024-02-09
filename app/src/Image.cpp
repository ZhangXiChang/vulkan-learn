#include "vk/Image.h"

namespace vk
{
    Image::Image(Device::Ptr device, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
        : mDevice(device), mWidth(width), mHeight(height)
    {
        CreateImageBuffer(usage, properties);
    }
    Image::~Image()
    {
        if (mImage != nullptr)
        {
            vkDestroyImage(mDevice->GetLogicalDevice(), mImage, nullptr);
        }
        if (mImageMemory != nullptr)
        {
            vkFreeMemory(mDevice->GetLogicalDevice(), mImageMemory, nullptr);
        }
        if (mImageView != nullptr)
        {
            vkDestroyImageView(mDevice->GetLogicalDevice(), mImageView, nullptr);
        }
    }

    Image::ImageInfo Image::OpenImageFile(std::string filePath)
    {
        ImageInfo imageData;
        imageData.Data = stbi_load(filePath.c_str(), &imageData.Width, &imageData.Height, nullptr, STBI_rgb_alpha);
        if (imageData.Data == nullptr)
        {
            return {};
        }
        imageData.Name = GetFileName(filePath);
        return imageData;
    }
    void Image::CreateImageBuffer(VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        // 获取长宽中的最大值，计算其可以被2整除多少次，然后计算不大于这个值的整数
        mMipLevels = std::floor(std::log2(std::max(mWidth, mHeight))) + 1;
        mLayerCount = 1;
        // 从CPU内存传输图像到GPU内存
        {
            if (!mDevice->CreateImage(mWidth, mHeight,
                                      VK_FORMAT_R8G8B8A8_SRGB,
                                      VK_IMAGE_TYPE_2D,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_IMAGE_TILING_OPTIMAL,
                                      usage, properties,
                                      mMipLevels, mLayerCount,
                                      &mImage, &mImageMemory))
            {
                return;
            }
            // 布局转换为传输目标位
            if (!mDevice->TransitionImageLayout(mImage, VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels, mLayerCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
            {
                return;
            }
            // 为所有mip生成图像，这会转换图像内存布局为着色器只读位
            if (!mDevice->GenerateMipmaps(mImage, VK_FORMAT_R8G8B8A8_SRGB, mWidth, mHeight, mMipLevels))
            {
                return;
            }
        }
        // 创建纹理图像视图
        mDevice->CreateImageView(mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels, mLayerCount, &mImageView);
    }
    bool Image::WriteImage(Image::Ptr image)
    {
        // 仅转换原图级别为传输目标位
        if (!mDevice->TransitionImageLayout(image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, image->GetMipLevels(), image->GetLayerCount(),
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL))
        {
            return false;
        }
        if (!mDevice->TransitionImageLayout(mImage, VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels, mLayerCount,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
        {
            return false;
        }
        // 仅拷贝mip原图级别到目标对应mip等级，然后重新生成mip，从而降低数据传输量
        mDevice->CopyImage(image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, 0, image->GetLayerCount(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           mImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, mLayerCount, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           image->GetWidht(), image->GetHeight());
        //
        // 复原布局
        if (!mDevice->TransitionImageLayout(image->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, image->GetMipLevels(), image->GetLayerCount(),
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
        {
            return false;
        }
        // 为所有mip重新生成图像，这会转换图像内存布局为着色器只读位
        if (!mDevice->GenerateMipmaps(mImage, VK_FORMAT_R8G8B8A8_SRGB, mWidth, mHeight, mMipLevels))
        {
            return false;
        }
        return true;
    }
    bool Image::WriteBuffer(Buffer::Ptr buffer)
    {
        // 布局转换为传输目标位
        if (!mDevice->TransitionImageLayout(mImage, VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels, mLayerCount,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
        {
            return false;
        }
        // 仅拷贝mip原图级别到目标对应mip等级，然后重新生成mip，从而降低数据传输量
        mDevice->CopyBufferToImage(buffer->GetBuffer(), mImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, mLayerCount, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mWidth, mHeight);
        // 为所有mip重新生成图像，这会转换图像内存布局为着色器只读位
        if (!mDevice->GenerateMipmaps(mImage, VK_FORMAT_R8G8B8A8_SRGB, mWidth, mHeight, mMipLevels))
        {
            return false;
        }
        return true;
    }
    bool Image::WriteData(void *data)
    {
        // 写入CPU内存
        uint64_t BufferSize = mWidth * mHeight * 4;
        Buffer::Ptr TempBuffer = Buffer::MakePtr(mDevice, BufferSize,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //
        TempBuffer->WriteHostData(data);
        // 从CPU内存传输图像到GPU内存
        if (!WriteBuffer(TempBuffer))
        {
            return false;
        }
        return true;
    }
} // namespace vk
