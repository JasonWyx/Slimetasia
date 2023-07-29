module;

#include <vulkan/vulkan.hpp>

module ShaderModuleObject;
import ShaderHelper;

ShaderModuleObject::ShaderModuleObject(const std::filesystem::path& filePath, const vk::Device& device)
    : m_OwnerDevice { device }
    , m_SpirV { ShaderHelper::CompileToSpirv(filePath) }
{
    const vk::ShaderModuleCreateInfo createInfo { {}, m_SpirV.size(), reinterpret_cast<const uint32_t*>(m_SpirV.data()) };

    m_Module = m_OwnerDevice.createShaderModule(createInfo);
}

ShaderModuleObject::~ShaderModuleObject()
{
    m_OwnerDevice.destroyShaderModule(m_Module);
}

vk::ShaderModule ShaderModuleObject::GetModule() const
{
    return m_Module;
}