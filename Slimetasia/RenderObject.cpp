#include "RenderObject.h"

#include <ranges>

RenderObject::RenderObject(const RenderContext& renderContext)
    : m_Context { renderContext }
{
    const vk::SemaphoreCreateInfo semaphoreCreateInfo {};
    for (uint32_t i = 0; i < renderContext.m_FramesInFlight; ++i)
    {
        m_SignalSemaphores.push_back(m_Context.m_Device.createSemaphore(semaphoreCreateInfo));
    }

    const vk::CommandPoolCreateInfo commandPoolCreateInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    };
    m_CommandPool = m_Context.m_Device.createCommandPool(commandPoolCreateInfo);

    const vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
        .commandPool = m_CommandPool,
        .commandBufferCount = renderContext.m_FramesInFlight,
    };
    m_CommandBuffers = m_Context.m_Device.allocateCommandBuffers(commandBufferAllocateInfo);
}

RenderObject::~RenderObject()
{
    std::ranges::for_each(m_SignalSemaphores, [this](const vk::Semaphore semaphore) { m_Context.m_Device.destroySemaphore(semaphore); });
    m_Context.m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
    m_Context.m_Device.destroyCommandPool(m_CommandPool);

    m_CommandBuffers.clear();
    m_SignalSemaphores.clear();
    m_Framebuffers.clear();
}

void RenderObject::OnExtentChanged(const vk::Extent2D& extent)
{
    m_Context.m_Extent = extent;
}
