#pragma once
#include <filesystem>
#include <vector>

namespace ShaderHelper
{
    std::vector<char> CompileToSpirv(const std::filesystem::path& filePath);

};  // namespace ShaderHelper
