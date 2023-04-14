#include "DeviceObject.h"

#include "Logger.h"
#include "RendererVk.h"

DeviceObject::DeviceObject()
    : m_ContextDevice { g_Renderer->GetDevice() }
{
    ASSERT(m_ContextDevice);
}

DeviceObject::DeviceObject(const vk::Device device)
    : m_ContextDevice { device }
{
    ASSERT(m_ContextDevice);
}

DeviceObject::~DeviceObject() {}
