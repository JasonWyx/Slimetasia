#include "DeviceObject.h"

#include "Logger.h"

DeviceObject::DeviceObject(const vk::Device device)
    : m_OwnerDevice { device }
{
    ASSERT(m_OwnerDevice);
}

DeviceObject::~DeviceObject() {}
