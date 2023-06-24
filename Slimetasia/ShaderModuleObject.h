#pragma once
#include <filesystem>
#include <vulkan/vulkan.hpp>

#include "ShaderHelper.h"

class ShaderModuleObject
{
public:

    ShaderModuleObject(const std::filesystem::path& filePath, const vk::Device& device)
        : m_OwnerDevice { device }
        , m_SpirV { ShaderHelper::CompileToSpirv(filePath) }
    {
        const vk::ShaderModuleCreateInfo createInfo {
            .codeSize = m_SpirV.size(),
            .pCode = reinterpret_cast<const uint32_t*>(m_SpirV.data()),
        };

        m_Module = m_OwnerDevice.createShaderModule(createInfo);
    }

    ~ShaderModuleObject() { m_OwnerDevice.destroyShaderModule(m_Module); }

    vk::ShaderModule GetModule() const { return m_Module; }

private:

    const vk::Device m_OwnerDevice {};
    const std::vector<char> m_SpirV {};

    vk::ShaderModule m_Module {};
};