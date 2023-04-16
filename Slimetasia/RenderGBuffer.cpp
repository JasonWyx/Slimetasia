#include "RenderGBuffer.h"

RenderGBuffer::RenderGBuffer(const RenderContext& renderContext)
    : RenderObject { renderContext }
{
    CreateDescriptors();
    CreateRenderPass();
    CreateFramebuffers();
    CreatePipeline();
}

RenderGBuffer::~RenderGBuffer()
{
    RenderObject::DestroyDescriptors();
    RenderObject::DestroyRenderPass();
    RenderObject::DestroyFramebuffers();
    RenderObject::DestroyPipeline();
}

RenderSyncObjects RenderGBuffer::Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence)
{
    return {};
}

void RenderGBuffer::OnExtentChanged(const vk::Extent2D& extent) {}

std::vector<vk::ImageView> RenderGBuffer::GatherOutputImages(const uint32_t currentFrame)
{
    return std::vector<vk::ImageView>();
}

std::vector<vk::BufferView> RenderGBuffer::GatherOutputBuffers(const uint32_t currentFrame)
{
    return std::vector<vk::BufferView>();
}

void RenderGBuffer::CreateDescriptors() {}

void RenderGBuffer::CreateRenderPass() {}

void RenderGBuffer::CreateFramebuffers()
{
    const vk::Extent3D extent { m_Context.m_Extent.width, m_Context.m_Extent.height, 1 };

    // DepthStencil
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eD24UnormS8Uint, extent, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageViewType::e2D,
                                                       vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, false, nullptr));

    // Diffuse
    m_GBufferImages.push_back(
        ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, false, nullptr));

    // Specular
    m_GBufferImages.push_back(
        ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, false, nullptr));

    // Emissive
    m_GBufferImages.push_back(
        ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, false, nullptr));

    m_GBufferImages.push_back(
        ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, false, nullptr));

    m_GBufferImages.push_back(
        ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, false, nullptr));

    m_GBufferImages.push_back(
        ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, false, nullptr));
}

void RenderGBuffer::CreatePipeline() {}