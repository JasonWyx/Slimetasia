#pragma once
#include "RenderObject.h"
#include "ImageObject.h"

class RenderGBuffer : public RenderObject
{
public:

    RenderGBuffer(const RenderContext& renderContext);
    ~RenderGBuffer();

    void CreateDescriptors() override;
    void DestroyDescriptors() override;
    void CreateRenderPass() override;
    void DestroyRenderPass() override;
    void CreateFramebuffers() override;
    void DestroyFramebuffers() override;
    void CreatePipeline() override;
    void DestroyPipeline() override;

    // Returns semaphore for other render commands to wait on
    vk::Semaphore Render(const uint32_t currentFrame, const std::vector<vk::Semaphore>& waitSemaphores) override;
    void OnExtentChanged(const vk::Extent2D& extent) override;
    std::vector<vk::ImageView> GatherOutputImages(const uint32_t currentFrame) override;
    std::vector<vk::BufferView> GatherOutputBuffers(const uint32_t currentFrame) override;

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
