#include "ImageObject.h"

#include <set>

#include "BufferObject.h"
#include "RendererVk.h"

ImageObject::ImageObject(const vk::ImageCreateInfo& imageCreateInfo, const vk::MemoryPropertyFlags& memoryProperties)
    : DeviceObject {}
    , m_Extent { imageCreateInfo.extent }
    , m_Format { imageCreateInfo.format }
    , m_Layout { imageCreateInfo.initialLayout }
{
    const vk::Extent3D& imageExtent = imageCreateInfo.extent;
    const uint32_t imageSize = imageExtent.width * imageExtent.height * imageExtent.depth;

    ASSERT(imageSize > 0);

    const std::unique_ptr<MemoryHandler>& memoryHandler = g_Renderer->GetMemoryHandler();
    m_Image = m_ContextDevice.createImage(imageCreateInfo);
    m_MemoryInfo = memoryHandler->AllocateImageMemory(m_Image, memoryProperties);

    m_ContextDevice.bindImageMemory(m_Image, m_MemoryInfo.m_Memory, m_MemoryInfo.m_Offset);
}

ImageObject::~ImageObject()
{
    const std::unique_ptr<MemoryHandler>& memoryHandler = g_Renderer->GetMemoryHandler();
    memoryHandler->FreeMemory(m_MemoryInfo);

    m_ContextDevice.destroyImage(m_Image);
    if (m_View)
    {
        m_ContextDevice.destroyImageView(m_View);
    }
    if (m_Sampler)
    {
        m_ContextDevice.destroySampler(m_Sampler);
    }
}

void ImageObject::GenerateView(const vk::ImageViewType& viewType, const vk::ImageAspectFlags& aspectFlags)
{
    ASSERT(!m_View);
    ASSERT(m_Image);

    const vk::ImageViewCreateInfo imageViewCreateInfo { {}, m_Image, viewType, m_Format, {}, vk::ImageSubresourceRange { aspectFlags, 0, 1, 0, 1 } };

    m_View = m_ContextDevice.createImageView(imageViewCreateInfo);
}

void ImageObject::GenerateSampler(const vk::Filter filter, const vk::SamplerAddressMode addressMode)
{
    ASSERT(!m_Sampler);

    vk::SamplerCreateInfo createInfo {};
    createInfo.setMagFilter(filter);
    createInfo.setMinFilter(filter);
    createInfo.setAddressModeU(addressMode);
    createInfo.setAddressModeV(addressMode);
    createInfo.setAddressModeW(addressMode);

    m_Sampler = m_ContextDevice.createSampler(createInfo);
}

void ImageObject::TransitionImageLayout(const vk::ImageLayout layout)
{
    vk::AccessFlags srcAccessMask {};
    vk::AccessFlags dstAccessMask {};
    vk::PipelineStageFlags srcStage {};
    vk::PipelineStageFlags dstStage {};

    if (m_Layout == vk::ImageLayout::eUndefined && layout == vk::ImageLayout::eTransferDstOptimal)
    {
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (m_Layout == vk::ImageLayout::eTransferDstOptimal && layout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        ASSERT(false);  // Invalid transition
    }

    // todo: need layout transition
    const vk::CommandBuffer command = g_Renderer->CreateOneShotCommandBuffer();

    const vk::ImageMemoryBarrier barrier { srcAccessMask,           dstAccessMask,           m_Layout, layout,
                                           VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_Image,  vk::ImageSubresourceRange { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

    command.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);

    g_Renderer->SubmitOneShotCommandBuffer(command, vk::QueueFlagBits::eTransfer, m_MemoryInfo.m_TransferFence);

    m_ContextDevice.waitIdle();
    m_Layout = layout;
}

void ImageObject::CopyFrom(const BufferObject& source, const vk::DeviceSize& offset)
{
    const vk::CommandBuffer copyCommand = g_Renderer->CreateOneShotCommandBuffer();
    const vk::BufferImageCopy copyRegion { source.GetMemoryInfo().m_Offset + offset, 0, 0, vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, {}, m_Extent };

    copyCommand.copyBufferToImage(source.GetBuffer(), m_Image, vk::ImageLayout::eTransferDstOptimal, copyRegion);

    g_Renderer->SubmitOneShotCommandBuffer(copyCommand, vk::QueueFlagBits::eTransfer, m_MemoryInfo.m_TransferFence);

    // todo: wait for fence maybe?
    m_ContextDevice.waitIdle();
}

/* static */
ImageObject* ImageObject::CreateImage(const vk::Format& format, const vk::Extent3D extent, const vk::ImageUsageFlags& usageFlags, const bool isHostVisible, const void* source)
{
    const std::set<uint32_t> uniqueIndices { g_Renderer->GetQueueIndex(QueueType::Graphics), g_Renderer->GetQueueIndex(QueueType::Transfer) };
    const std::vector<uint32_t> indices { uniqueIndices.begin(), uniqueIndices.end() };

    const vk::ImageTiling imageTiling = isHostVisible ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal;
    const vk::ImageLayout imageLayout = source == nullptr ? vk::ImageLayout::eUndefined : vk::ImageLayout::eTransferDstOptimal;

    const vk::ImageCreateInfo imageCreateInfo { {}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, imageTiling, usageFlags, {}, indices, imageLayout };
    const vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    ImageObject* imageObject = new ImageObject { imageCreateInfo, memoryProperties };

    if (source != nullptr)
    {
        const size_t imageDataSize = extent.width * extent.height * extent.depth;

        BufferObject* stagingBuffer = BufferObject::CreateBuffer(vk::BufferUsageFlagBits::eTransferSrc, true, source, imageDataSize);

        // todo: async transfer?
        imageObject->CopyFrom(*stagingBuffer, {});

        // Free staging resources
        delete stagingBuffer;
    }

    return imageObject;
}

ImageObject* ImageObject::CreateDepthImage(const vk::Format& format, const vk::Extent3D extent, const vk::ImageUsageFlags& usageFlags)
{
    const std::set<uint32_t> uniqueIndices { g_Renderer->GetQueueIndex(QueueType::Graphics), g_Renderer->GetQueueIndex(QueueType::Transfer) };
    const std::vector<uint32_t> indices { uniqueIndices.begin(), uniqueIndices.end() };

    const vk::ImageCreateInfo imageCreateInfo { {}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usageFlags, {}, indices, vk::ImageLayout::eUndefined };
    const vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    ImageObject* imageObject = new ImageObject { imageCreateInfo, memoryProperties };

    return imageObject;
}
