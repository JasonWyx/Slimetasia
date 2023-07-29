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

    static constexpr size_t MAX_OBJECTS = 1000;
    static constexpr size_t MAX_BONES = 32;

protected:

    void CreateDescriptors() override;
    void CreateRenderPass() override;
    void CreateFramebuffers() override;
    void DestroyFramebuffers() override;
    void CreatePipeline() override;

private:


    // Copied from GeometryPass.h
    enum GBufferIndex
    {
        Diffuse = 0,
        Specular,
        Emissive,
        Position,  // In world space
        Normal,    // In world space
#ifdef EDITOR
        TexCoords,
        PickingID,
#endif
        Count
    };

    std::vector<std::vector<ImageObject*>> m_GBufferImages;
};
