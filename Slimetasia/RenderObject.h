#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

#include "QueueType.h"
import ShaderModuleObject;

// Commonly used constructs
struct RenderContext
{
    const vk::Device m_Device {};
    const std::array<uint32_t, QueueType::Count> m_QueueIndices {};
    const std::array<vk::Queue, QueueType::Count> m_Queues {};
    const uint32_t m_FramesInFlight {};
    const uint32_t m_SwapchainCount {};

    vk::Extent2D m_WindowExtent {};
    vk::Extent2D m_RenderExtent {};
    bool m_IsRenderToTarget {};
};

struct RenderOutputs
{
    vk::Semaphore m_SignaledSemaphore {};
    vk::Fence m_SignaledFence {}; // not really used yet, usually synchronization on cpu is external
    std::vector<vk::ImageView> m_OutputImages {};
    std::vector<vk::BufferView> m_OutputBuffers {};
};

struct FrameInfo
{
    const uint32_t m_FrameIndex {};
    const uint32_t m_SwapchainIndex {};
};

class RenderObject
{
public:

    RenderObject(const RenderContext& renderContext);
    virtual ~RenderObject();

    // Returns semaphore for other render commands to wait on
    virtual RenderOutputs Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence) = 0;
    virtual void SetWindowExtent(const vk::Extent2D& extent);
    virtual void SetRenderExtent(const vk::Extent2D& extent);
    virtual void SetIsRenderToTarget(const bool isRenderToTarget);

    vk::RenderPass GetRenderPass() const { return m_RenderPass; }

protected:

    virtual void CreateDescriptors() {};
    virtual void DestroyDescriptors();

    virtual void CreateRenderPass() {};
    virtual void DestroyRenderPass();

    virtual void CreateFramebuffers() {};
    virtual void DestroyFramebuffers();

    virtual void CreatePipeline() {};
    virtual void DestroyPipeline();

    RenderContext m_Context;
    std::vector<vk::Semaphore> m_SignalSemaphores;

    // Assuming only need 1 command buffer per "render pass" for now
    vk::CommandPool m_CommandPool;
    std::vector<vk::CommandBuffer> m_CommandBuffers;

    vk::DescriptorPool m_DescriptorPool;
    std::vector<vk::DescriptorSet> m_DescriptorSets;

    vk::RenderPass m_RenderPass;
    std::vector<vk::Framebuffer> m_Framebuffers;
    vk::PipelineLayout m_PipelineLayout;
    vk::Pipeline m_Pipeline;
};
