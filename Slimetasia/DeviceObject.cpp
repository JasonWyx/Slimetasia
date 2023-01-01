#include "DeviceObject.h"

#include "Logger.h"

DeviceObject::DeviceObject(const vk::Device device)
    : m_OwningDevice { device }
{
    ASSERT(m_OwningDevice);
}

DeviceObject::~DeviceObject() {}
