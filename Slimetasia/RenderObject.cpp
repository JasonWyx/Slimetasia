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

    const vk::CommandPoolCreateInfo commandPoolCreateInfo { vk::CommandPoolCreateFlagBits::eResetCommandBuffer };
    m_CommandPool = m_Context.m_Device.createCommandPool(commandPoolCreateInfo);

    const vk::CommandBufferAllocateInfo commandBufferAllocateInfo { m_CommandPool, vk::CommandBufferLevel::ePrimary, renderContext.m_FramesInFlight };
    m_CommandBuffers = m_Context.m_Device.allocateCommandBuffers(commandBufferAllocateInfo);
}

RenderObject::~RenderObject()
{
    std::ranges::for_each(m_SignalSemaphores, [this](const vk::Semaphore semaphore) { m_Context.m_Device.destroySemaphore(semaphore); });
    m_Context.m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
    m_Context.m_Device.destroyCommandPool(m_CommandPool);

    m_CommandBuffers.clear();
    m_SignalSemaphores.clear();
}

void RenderObject::SetWindowExtent(const vk::Extent2D& extent)
{
    m_Context.m_WindowExtent = extent;
}

void RenderObject::SetRenderExtent(const vk::Extent2D& extent)
{
    m_Context.m_RenderExtent = extent;
}

void RenderObject::SetIsRenderToTarget(const bool isRenderToTarget)
{
    m_Context.m_IsRenderToTarget = isRenderToTarget;
}

void RenderObject::DestroyDescriptors()
{
    if (m_DescriptorPool)
    {
        m_Context.m_Device.destroyDescriptorPool(m_DescriptorPool);
    }
}

void RenderObject::DestroyRenderPass()
{
    if (m_RenderPass)
    {
        m_Context.m_Device.destroyRenderPass(m_RenderPass);
    }
}

void RenderObject::DestroyFramebuffers()
{
    for (const vk::Framebuffer& framebuffer : m_Framebuffers)
    {
        if (framebuffer)
        {
            m_Context.m_Device.destroyFramebuffer(framebuffer);
        }
    }
    m_Framebuffers.clear();
}

void RenderObject::DestroyPipeline()
{
    if (m_Pipeline)
    {
        m_Context.m_Device.destroyPipeline(m_Pipeline);
    }

    if (m_PipelineLayout)
    {
        m_Context.m_Device.destroyPipelineLayout(m_PipelineLayout);
    }
}
