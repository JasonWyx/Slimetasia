export module ShaderHelper;
import <vector>;
import <filesystem>;

export namespace ShaderHelper
{
    export std::vector<char> CompileToSpirv(const std::filesystem::path& filePath);

};  // namespace ShaderHelper
