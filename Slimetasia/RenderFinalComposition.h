#pragma once
#include "RenderObject.h"

#include "SwapchainHandler.h"

class RenderFinalComposition : public RenderObject
{
public:

    RenderFinalComposition(const RenderContext& renderContext, const std::unique_ptr<SwapchainHandler>& swapchain);
    ~RenderFinalComposition();

    // Inherited via RenderObject
    RenderSyncObjects Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence) override;

    void InitializeAfterSwapchain();

protected:

    void CreateDescriptors() override;
    void CreateRenderPass() override;
    void CreateFramebuffers() override;
    void CreatePipeline() override;

private:

    const std::unique_ptr<SwapchainHandler>& m_SwapchainCache;
};
