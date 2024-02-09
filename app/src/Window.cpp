#include "vk/Window.h"

namespace vk
{
    Window::Window(std::string title, uint32_t width, uint32_t height, bool isFullScreen, bool isHideMouse)
        : mIsFullScreen(isFullScreen), mIsHideMouse(isHideMouse)
    {
        CreateWindow(title, width, height);
    }
    Window::~Window()
    {
        if (mWindow != nullptr)
        {
            SDL_DestroyWindow(mWindow);
        }
        SDL_Quit();
    }

    void Window::CreateWindow(std::string title, uint32_t width, uint32_t height)
    {
        if (mIsFullScreen)
        {
            mWindow = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
        }
        else
        {
            mWindow = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_VULKAN);
        }
        if (mWindow == nullptr)
        {
            std::runtime_error(SDL_GetError());
        }
        if (mIsHideMouse)
        {
            if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0)
            {
                std::runtime_error(SDL_GetError());
            }
        }
    }

    void Window::Init()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        {
            std::runtime_error(SDL_GetError());
        }
    }
    SDL_Rect Window::GetDisplayBound(uint32_t displayIndex)
    {
        SDL_Rect DisplayBound;
        if (SDL_GetDisplayBounds(displayIndex, &DisplayBound) != 0)
        {
            std::runtime_error(SDL_GetError());
        }
        return DisplayBound;
    }
    bool Window::PollEvent(SDL_Event *Event)
    {
        return SDL_PollEvent(Event) != 0;
    }
    void Window::GetFrameBufferSize(int *FrameWidth, int *FrameHeight)
    {
        SDL_Vulkan_GetDrawableSize(mWindow, FrameWidth, FrameHeight);
    }
    uint32_t Window::GetMouseRelativePos(int *xrel, int *yrel)
    {
        return SDL_GetRelativeMouseState(xrel, yrel);
    }
    uint8_t *Window::GetKeyboardStatus()
    {
        return (uint8_t *)SDL_GetKeyboardState(nullptr);
    }
} // namespace vk
