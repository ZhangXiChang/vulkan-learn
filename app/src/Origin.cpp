#include "vk/Origin.h"

namespace vk
{
    std::string GetFileName(std::string filePath)
    {
        size_t Start = 0;
        size_t END = 0;
        for (int i = filePath.size() - 1; i >= 0; i--)
        {
            if (filePath[i] == '.')
            {
                END = i;
            }
            if (filePath[i] == '/')
            {
                Start = i;
                break;
            }
        }
        std::string fileName;
        for (size_t i = Start + 1; i < END; i++)
        {
            fileName.push_back(filePath[i]);
        }
        return fileName;
    }
} // namespace vk
