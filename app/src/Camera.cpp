#include "vk/Camera.h"

namespace vk
{
    Camera::Camera(Device::Ptr device, float yaw, float pitch, float x, float y, float z, float focalLength, float proximalPoint, float farPoint)
    {
        Transform(device, yaw, pitch, x, y, z, focalLength, proximalPoint, farPoint);
    }
    Camera::~Camera()
    {
    }

    void Camera::Transform(Device::Ptr device, float yaw, float pitch, float x, float y, float z, float focalLength, float proximalPoint, float farPoint)
    {
        mYaw = yaw;
        mPitch = pitch;
        mCameraPos = {x, y, z};
        mCameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 Front;
        Front.x = cos(glm::radians(mPitch)) * sin(glm::radians(mYaw));
        Front.y = cos(glm::radians(mPitch)) * cos(glm::radians(mYaw));
        Front.z = sin(glm::radians(mPitch));
        mCameraFront = glm::normalize(Front);
        VkExtent2D Extent = device->GetSwapchainImageExtent();
        mProjectionMat = glm::perspective(glm::radians(focalLength), Extent.width / (float)Extent.height, proximalPoint, farPoint);
        mProjectionMat[1][1] *= -1;
    }
    void Camera::InputTick()
    {
        // 视角变换
        int Xrel = 0, Yrel = 0;
        Window::GetMouseRelativePos(&Xrel, &Yrel);
        float VisualAngleSpeed = 0.05f; // 视角变换灵敏度
        mYaw += Xrel * VisualAngleSpeed;
        mPitch += (Yrel * -1) * VisualAngleSpeed;
        if (mPitch > 89.999f)
        {
            mPitch = 89.999f;
        }
        if (mPitch < -89.999f)
        {
            mPitch = -89.999f;
        }
        glm::vec3 Front;
        Front.x = cos(glm::radians(mPitch)) * sin(glm::radians(mYaw));
        Front.y = cos(glm::radians(mPitch)) * cos(glm::radians(mYaw));
        Front.z = sin(glm::radians(mPitch));
        mCameraFront = glm::normalize(Front);
        // 相机移动
        uint8_t *KeyboardStatus = Window::GetKeyboardStatus();
        // 灵敏度矫正
        static std::chrono::steady_clock::time_point StartTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point CurrentTime = std::chrono::steady_clock::now();
        float TimeDifference = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;
        float MoveSpeed = TimeDifference * 3.0f; // 移动灵敏度
        if (KeyboardStatus[SDL_SCANCODE_W])
        {
            mCameraPos += mCameraFront * MoveSpeed;
        }
        if (KeyboardStatus[SDL_SCANCODE_S])
        {
            mCameraPos -= mCameraFront * MoveSpeed;
        }
        if (KeyboardStatus[SDL_SCANCODE_A])
        {
            mCameraPos -= glm::normalize(glm::cross(mCameraFront, mCameraUp)) * MoveSpeed;
        }
        if (KeyboardStatus[SDL_SCANCODE_D])
        {
            mCameraPos += glm::normalize(glm::cross(mCameraFront, mCameraUp)) * MoveSpeed;
        }
        if (KeyboardStatus[SDL_SCANCODE_SPACE])
        {
            mCameraPos += glm::vec3(0.0f, 0.0f, 1.0f) * MoveSpeed;
        }
        if (KeyboardStatus[SDL_SCANCODE_LSHIFT])
        {
            mCameraPos -= glm::vec3(0.0f, 0.0f, 1.0f) * MoveSpeed;
        }
        // 获取参数
        mViewMat = glm::lookAt(mCameraPos, mCameraPos + mCameraFront, mCameraUp);
        mInverseViewMat = glm::inverse(mViewMat);
    }
} // namespace vk
