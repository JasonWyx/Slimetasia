#pragma once
#include "DeviceObject.h"

struct MemoryInfo
{
    vk::DeviceMemory m_Memory {};
    vk::DeviceSize m_Offset {};
    vk::DeviceSize m_Size {};
    vk::Fence m_TransferFence{};
};

class MemoryHandler : public DeviceObject
{
public:

    MemoryHandler(const vk::PhysicalDevice physicalDevice, const vk::Device device);
    ~MemoryHandler();

    // todo: might want to use unique_ptr for unique allocation
    MemoryInfo AllocateBufferMemory(const vk::Buffer buffer, const vk::MemoryPropertyFlags propertyFlags);
    MemoryInfo AllocateImageMemory(const vk::Image image, const vk::MemoryPropertyFlags propertyFlags);
    void FreeMemory(MemoryInfo& memoryInfo);
    void WriteToMemory(const void* src, const size_t size, const MemoryInfo& dst);

private:

    uint32_t FindMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties);

    const vk::PhysicalDeviceMemoryProperties m_MemoryProperties {};
};
