#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

#include "ShaderHelper.h"
#include "QueueType.h"

// Commonly used constructs
struct RenderContext
{
    const vk::Device m_Device {};
    const std::array<uint32_t, QueueType::Count> m_QueueIndices;
    const std::array<vk::Queue, QueueType::Count> m_Queues;
    const uint32_t m_FramesInFlight {};

    vk::Extent2D m_Extent {};
};

struct RenderSyncObjects
{
    vk::Semaphore signaledSemaphore;
    // vk::Fence signaledFence;
};

struct FrameInfo
{
    const uint32_t frameIndex {};
    const uint32_t swapchainIndex {};
};

class RenderObject
{
public:

    RenderObject(const RenderContext& renderContext);
    virtual ~RenderObject();

    // Returns semaphore for other render commands to wait on
    virtual RenderSyncObjects Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence) = 0;
    virtual void OnExtentChanged(const vk::Extent2D& extent);
    virtual std::vector<vk::ImageView> GatherOutputImages(const uint32_t currentFrame) { return {}; }
    virtual std::vector<vk::BufferView> GatherOutputBuffers(const uint32_t currentFrame) { return {}; }

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
