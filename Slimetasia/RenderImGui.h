#pragma once
#include "RenderObject.h"

#include "SwapchainHandler.h"

class RenderImGui : public RenderObject
{
public:

    RenderImGui(const RenderContext& renderContext, const vk::Instance instance, const vk::PhysicalDevice physicalDevice, const std::unique_ptr<SwapchainHandler>& swapchain);
    ~RenderImGui();

    // Inherited via RenderObject
    RenderSyncObjects Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores) override;


    void InitializeAfterSwapchain();

protected:

    virtual void CreateDescriptors() override;
    virtual void DestroyDescriptors() override;
    virtual void CreateRenderPass() override;
    virtual void DestroyRenderPass() override;
    virtual void CreateFramebuffers() override;
    virtual void DestroyFramebuffers() override;
    virtual void CreatePipeline() override;
    virtual void DestroyPipeline() override;

private:

    const std::unique_ptr<SwapchainHandler>& m_SwapchainCache;
};
