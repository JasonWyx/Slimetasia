#pragma once
#include "RenderObject.h"

class RenderFinalComposition : public RenderObject
{
public:

    RenderFinalComposition(const RenderContext& renderContext);
    ~RenderFinalComposition();

    RenderSyncObjects Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores) override;

protected:

    void CreateDescriptors() override;
    void DestroyDescriptors() override;
    void CreateRenderPass() override;
    void DestroyRenderPass() override;
    void CreateFramebuffers() override;
    void DestroyFramebuffers() override;
    void CreatePipeline() override;
    void DestroyPipeline() override;
};
