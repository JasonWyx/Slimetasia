module;

#include <vulkan/vulkan.hpp>

export module ShaderModuleObject;
import <filesystem>;

export class ShaderModuleObject
{
public:

    ShaderModuleObject(const std::filesystem::path& filePath, const vk::Device& device);
    ~ShaderModuleObject();

    vk::ShaderModule GetModule() const;

private:

    const vk::Device m_OwnerDevice {};
    const std::vector<char> m_SpirV {};

    vk::ShaderModule m_Module {};
};