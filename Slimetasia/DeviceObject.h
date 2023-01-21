#pragma once
#include <vulkan/vulkan.hpp>

// Helper base class to capture the owning vulkan device that the object was created from
class DeviceObject
{
protected:

    DeviceObject(const vk::Device device);
    virtual ~DeviceObject();

    const vk::Device m_OwnerDevice;
};
