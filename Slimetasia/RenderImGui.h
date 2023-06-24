#pragma once
#include "RenderObject.h"
#include "SwapchainHandler.h"

class RenderImGui : public RenderObject
{
public:

    RenderImGui(const RenderContext& renderContext, const vk::Instance instance, const vk::PhysicalDevice physicalDevice);
    ~RenderImGui();

    // Inherited via RenderObject
    RenderOutputs Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence) override;

    void OnSwapchainFramebuffersChanged();

protected:

    virtual void CreateDescriptors() override;
    virtual void CreateRenderPass() override;
    virtual void CreateFramebuffers() override;
    virtual void CreatePipeline() override;
};
