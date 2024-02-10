#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <volk.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp> //矩阵变换

#include <imgui_impl_sdl2.h>
#include "imgui_impl_vulkan.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <memory>
#include <optional>
#include <set>
#include <fstream>
#include <functional>
#include <chrono>

namespace vk
{
    std::string GetFileName(std::string filePath);
} // namespace vk
