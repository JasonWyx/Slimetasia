#include "BufferObject.h"

#include <set>

#include "RendererVk.h"

BufferObject::BufferObject(const vk::BufferCreateInfo& createInfo, const vk::MemoryPropertyFlags& memoryProperties)
    : DeviceObject {}
{
    const std::unique_ptr<MemoryHandler>& memoryHandler = g_Renderer->GetMemoryHandler();

    m_Buffer = m_ContextDevice.createBuffer(createInfo);
    m_MemoryInfo = memoryHandler->AllocateBufferMemory(m_Buffer, memoryProperties);

    m_ContextDevice.bindBufferMemory(m_Buffer, m_MemoryInfo.m_Memory, m_MemoryInfo.m_Offset);
}

BufferObject::~BufferObject()
{
    const std::unique_ptr<MemoryHandler>& memoryHandler = g_Renderer->GetMemoryHandler();
    memoryHandler->FreeMemory(m_MemoryInfo);

    m_ContextDevice.destroyBuffer(m_Buffer);
    if (m_View)
    {
        m_ContextDevice.destroyBufferView(m_View);
    }
}

void BufferObject::GenerateView()
{
    // todo
}

void BufferObject::Write(const void* source, const size_t size)
{
    const std::unique_ptr<MemoryHandler>& memoryHandler = g_Renderer->GetMemoryHandler();
    memoryHandler->WriteToMemory(source, size, m_MemoryInfo);
}

void BufferObject::CopyFrom(const BufferObject& sourceBuffer, const vk::DeviceSize offset, const vk::DeviceSize size)
{
    const vk::CommandBuffer copyCommand = g_Renderer->CreateOneShotCommandBuffer();

    const vk::BufferCopy copyRegion {
        .srcOffset = sourceBuffer.GetMemoryInfo().m_Offset,
        .dstOffset = m_MemoryInfo.m_Offset + offset,
        .size = size,
    };
    copyCommand.copyBuffer(sourceBuffer.GetBuffer(), m_Buffer, copyRegion);

    g_Renderer->SubmitOneShotCommandBuffer(copyCommand, vk::QueueFlagBits::eTransfer, m_MemoryInfo.m_TransferFence);

    // todo: wait for fence maybe?
    m_ContextDevice.waitIdle();
}

/* static */
BufferObject* BufferObject::CreateBuffer(vk::BufferUsageFlags usageFlags, const bool isHostVisible, const void* source, const size_t size)
{
    const std::set<uint32_t> uniqueIndices { g_Renderer->GetQueueIndex(QueueType::Graphics), g_Renderer->GetQueueIndex(QueueType::Transfer) };
    const std::vector<uint32_t> indices { uniqueIndices.begin(), uniqueIndices.end() };

    // Trasfer dst only if writing to device local
    if (source != nullptr && !isHostVisible)
    {
        usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
    }

    const vk::BufferCreateInfo createInfo {
        .size = size,
        .usage = usageFlags,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(indices.size()),
        .pQueueFamilyIndices = indices.data(),
    };

    constexpr vk::MemoryPropertyFlags HOST_VISIBLE_MEMORY_FLAGS = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    constexpr vk::MemoryPropertyFlags DEVICE_LOCAL_MEMORY_FLAGS = vk::MemoryPropertyFlagBits::eDeviceLocal;

    const vk::MemoryPropertyFlags memoryProperties { isHostVisible ? HOST_VISIBLE_MEMORY_FLAGS : DEVICE_LOCAL_MEMORY_FLAGS };

    BufferObject* result = new BufferObject { createInfo, memoryProperties };

    if (source != nullptr)
    {
        if (isHostVisible)
        {
            result->Write(source, size);
        }
        else  // Not host visible, needs staging transfer
        {
            const vk::BufferCreateInfo stagingCreateInfo {
                .size = size,
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .sharingMode = vk::SharingMode::eExclusive,
                .queueFamilyIndexCount = static_cast<uint32_t>(indices.size()),
                .pQueueFamilyIndices = indices.data(),
            };

            BufferObject* staging = new BufferObject { stagingCreateInfo, HOST_VISIBLE_MEMORY_FLAGS };
            staging->Write(source, size);
            result->CopyFrom(*staging, 0, size);
            delete staging;
        }
    }

    return result;
}
