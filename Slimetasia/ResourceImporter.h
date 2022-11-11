#pragma once
#include <string>
#include <vector>

#include "ResourceManager.h"

namespace ResourceImporter
{
    void ImportModel(const std::filesystem::path& filePath);
    void ImportTexture(const std::vector<std::filesystem::path>& filePaths);
};  // namespace ResourceImporter
