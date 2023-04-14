#include "MemoryHandler.h"

#include <memory>

#include "Logger.h"
#include "RendererVk.h"

MemoryHandler::MemoryHandler(const vk::PhysicalDevice physicalDevice, const vk::Device device)
    : DeviceObject { device }
    , m_MemoryProperties { physicalDevice.getMemoryProperties() }
{
}

MemoryHandler::~MemoryHandler() {}

MemoryInfo MemoryHandler::AllocateBufferMemory(const vk::Buffer buffer, const vk::MemoryPropertyFlags propertyFlags)
{
    const vk::MemoryRequirements& memoryRequirements { m_ContextDevice.getBufferMemoryRequirements(buffer) };
    const vk::MemoryAllocateInfo allocateInfo {
        .allocationSize { memoryRequirements.size },
        .memoryTypeIndex { FindMemoryType(memoryRequirements.memoryTypeBits, propertyFlags) },
    };

    const vk::FenceCreateInfo fenceCreateInfo { .flags = vk::FenceCreateFlagBits::eSignaled };

    // todo: do bulk allocation rather than individual allocations
    MemoryInfo memoryInfo {
        .m_Memory { m_ContextDevice.allocateMemory(allocateInfo) },
        .m_Size { memoryRequirements.size },
        .m_TransferFence { m_ContextDevice.createFence(fenceCreateInfo) },
    };

    return memoryInfo;
}

MemoryInfo MemoryHandler::AllocateImageMemory(const vk::Image image, const vk::MemoryPropertyFlags propertyFlags)
{
    const vk::MemoryRequirements& memoryRequirements { m_ContextDevice.getImageMemoryRequirements(image) };
    const vk::MemoryAllocateInfo allocateInfo {
        .allocationSize { memoryRequirements.size },
        .memoryTypeIndex { FindMemoryType(memoryRequirements.memoryTypeBits, propertyFlags) },
    };

    const vk::FenceCreateInfo fenceCreateInfo { .flags = vk::FenceCreateFlagBits::eSignaled };

    // todo: do bulk allocation rather than individual allocations
    MemoryInfo memoryInfo {
        .m_Memory { m_ContextDevice.allocateMemory(allocateInfo) },
        .m_Size { memoryRequirements.size },
        .m_TransferFence { m_ContextDevice.createFence(fenceCreateInfo) },
    };

    return memoryInfo;
}

void MemoryHandler::FreeMemory(MemoryInfo& memoryInfo)
{
    const vk::Result& result = m_ContextDevice.waitForFences(memoryInfo.m_TransferFence, VK_TRUE, UINT64_MAX);
    ASSERT(result == vk::Result::eSuccess);

    m_ContextDevice.destroyFence(memoryInfo.m_TransferFence);
    m_ContextDevice.freeMemory(memoryInfo.m_Memory);

    // Clear handles to prevent use
    memset(static_cast<void*>(&memoryInfo), 0, sizeof(MemoryInfo));
}

void MemoryHandler::WriteToMemory(const void* src, const size_t size, const MemoryInfo& target)
{
    ASSERT(size <= target.m_Size);

    void* dst = m_ContextDevice.mapMemory(target.m_Memory, target.m_Offset, target.m_Size);
    memcpy(dst, src, size);
    m_ContextDevice.unmapMemory(target.m_Memory);
}

uint32_t MemoryHandler::FindMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i) && m_MemoryProperties.memoryTypes[i].propertyFlags & properties)
        {
            return i;
        }
    }

    // Failed to find memory type
    ASSERT(false);

    return 0;
}
