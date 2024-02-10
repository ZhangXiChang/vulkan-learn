#pragma once
#include "Origin.h"
#include "Window.h"
#include "Device.h"

namespace vk
{
    class Camera
    {
    public:
        Camera(Device::Ptr device, float yaw, float pitch, float x, float y, float z, float focalLength, float proximalPoint, float farPoint);
        ~Camera();

        using Ptr = std::shared_ptr<Camera>;
        static Ptr New(Device::Ptr device, float yaw, float pitch, float x, float y, float z, float focalLength, float proximalPoint, float farPoint)
        {
            return std::make_shared<Camera>(device, yaw, pitch, x, y, z, focalLength, proximalPoint, farPoint);
        }

    private:
        float mYaw = 0.0f;
        float mPitch = 0.0f;
        glm::vec3 mCameraPos{};
        glm::vec3 mCameraUp{};
        glm::vec3 mCameraFront{};
        glm::mat4 mProjectionMat{};
        glm::mat4 mViewMat{};
        glm::mat4 mInverseViewMat{};

    public:
        void Transform(Device::Ptr device, float yaw, float pitch, float x, float y, float z, float focalLength, float proximalPoint, float farPoint);
        void InputTick();

        glm::mat4 GetProjectionMat() { return mProjectionMat; }
        glm::mat4 GetViewMat() { return mViewMat; }
        glm::mat4 GetInverseViewMat() { return mInverseViewMat; }
        glm::vec2 GetView() { return glm::vec2(mYaw, mPitch); }
        glm::vec3 GetPosition() { return mCameraPos; }
    };
} // namespace vk
