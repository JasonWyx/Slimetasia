#pragma once

#include "DeviceObject.h"
#include "MemoryHandler.h"

class BufferObject : public DeviceObject
{
public:

    BufferObject(const vk::BufferCreateInfo& createInfo, const vk::MemoryPropertyFlags& memoryProperties);
    ~BufferObject();

    void GenerateView();

    void Write(const void* source, const size_t size);
    void CopyFrom(const BufferObject& sourceBuffer, const vk::DeviceSize offset, const vk::DeviceSize size);

    vk::Buffer GetBuffer() const { return m_Buffer; }
    vk::BufferView GetView() const { return m_View; }
    MemoryInfo GetMemoryInfo() const { return m_MemoryInfo; }

    static BufferObject* CreateBuffer(vk::BufferUsageFlags usageFlags, const bool isHostVisible, const void* source, const size_t size);

private:

    vk::Buffer m_Buffer;
    vk::BufferView m_View;
    MemoryInfo m_MemoryInfo;
};