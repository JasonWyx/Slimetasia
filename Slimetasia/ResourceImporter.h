#pragma once
#include <string>
#include <vector>

#include "ResourceManager.h"

namespace ResourceImporter
{
    void ImportModel(const filesystem::path& filePath);
    void ImportTexture(const std::vector<filesystem::path>& filePaths);
};  // namespace ResourceImporter
