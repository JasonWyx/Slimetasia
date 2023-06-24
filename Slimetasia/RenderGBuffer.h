#pragma once
#include "ImageObject.h"
#include "RenderObject.h"

class RenderGBuffer : public RenderObject
{
public:

    RenderGBuffer(const RenderContext& renderContext);
    ~RenderGBuffer();

    // Returns semaphore for other render commands to wait on
    RenderOutputs Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence) override;

    void SetWindowExtent(const vk::Extent2D& extent) override;

protected:

    void CreateDescriptors() override;
    void CreateRenderPass() override;
    void CreateFramebuffers() override;
    void DestroyFramebuffers() override;
    void CreatePipeline() override;

private:

    // Copied from GeometryPass.h
    enum GBuffer
    {
        Diffuse = 0,
        Specular,
        Emissive,
        WorldPosition,
        WorldNormal,
#ifdef EDITOR
        TexCoords,
        PickingID,
#endif
        Count
    };

    std::vector<ImageObject*> m_GBufferImages;
};
