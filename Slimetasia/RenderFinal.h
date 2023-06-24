#pragma once
#include "RenderObject.h"
#include "SwapchainHandler.h"

class ImageObject;

class RenderFinal : public RenderObject
{
public:

    RenderFinal(const RenderContext& renderContext);
    ~RenderFinal();

    // Inherited via RenderObject
    RenderOutputs Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence) override;
    void SetWindowExtent(const vk::Extent2D& extent) override;
    void SetRenderExtent(const vk::Extent2D& extent) override;
    void SetIsRenderToTarget(const bool isRenderToTarget) override;

    void OnSwapchainFramebuffersChanged();

#ifdef EDITOR
    VkDescriptorSet GetRenderAttachment(const uint32_t frameIndex) const;
#endif  // EDITOR

protected:

    void CreateDescriptors() override;
    void CreateRenderPass() override;
    void CreateFramebuffers() override;
    void DestroyFramebuffers() override;
    void CreatePipeline() override;

private:

#ifdef EDITOR
    std::vector<ImageObject*> m_RenderTargets;
    std::vector<VkDescriptorSet> m_RenderAttachments;
#endif
};
