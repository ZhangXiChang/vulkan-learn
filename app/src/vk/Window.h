#pragma once
#include "Origin.h"

namespace vk
{
    class Window
    {
    public:
        Window(std::string title, uint32_t width, uint32_t height, bool isFullScreen, bool isHideMouse);
        ~Window();

        using Ptr = std::shared_ptr<Window>;
        static Ptr New(std::string title, uint32_t width, uint32_t height, bool isFullScreen, bool isHideMouse)
        {
            return std::make_shared<Window>(title, width, height, isFullScreen, isHideMouse);
        }

        static void Init();
        static SDL_Rect GetDisplayBound(uint32_t displayIndex);
        static bool PollEvent(SDL_Event *Event);
        static uint32_t GetMouseRelativePos(int *xrel, int *yrel);
        static uint8_t *GetKeyboardStatus();

    private:
        SDL_Window *mWindow = nullptr;
        bool mIsFullScreen = false;
        bool mIsHideMouse = false;

    private:
        void CreateWindow(std::string title, uint32_t width, uint32_t height);

    public:
        SDL_Window *GetWindow() { return mWindow; }
        void GetFrameBufferSize(int *FrameWidth, int *FrameHeight);
        bool GetIsHideMouse() { return mIsHideMouse; }
    };
} // namespace vk
