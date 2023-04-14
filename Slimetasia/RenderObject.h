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

class RenderObject
{
public:

    RenderObject(const RenderContext& renderContext);
    virtual ~RenderObject();

    // Returns semaphore for other render commands to wait on
    virtual vk::Semaphore Render(const uint32_t currentFrame, const std::vector<vk::Semaphore>& waitSemaphores) = 0;
    virtual void OnExtentChanged(const vk::Extent2D& extent);
    virtual std::vector<vk::ImageView> GatherOutputImages(const uint32_t currentFrame) { return {}; }
    virtual std::vector<vk::BufferView> GatherOutputBuffers(const uint32_t currentFrame) { return {}; }

protected:

    virtual void CreateDescriptors() = 0;
    virtual void DestroyDescriptors() = 0;
    virtual void CreateRenderPass() = 0;
    virtual void DestroyRenderPass() = 0;
    virtual void CreateFramebuffers() = 0;
    virtual void DestroyFramebuffers() = 0;
    virtual void CreatePipeline() = 0;
    virtual void DestroyPipeline() = 0;

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
