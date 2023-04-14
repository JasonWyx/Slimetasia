#include "ImageObject.h"

#include <set>

#include "BufferObject.h"
#include "RendererVk.h"

ImageObject::ImageObject(const vk::ImageCreateInfo& imageCreateInfo, const vk::MemoryPropertyFlags& memoryProperties, const vk::ImageViewType& viewType, const vk::ImageAspectFlags& aspect)
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

    // todo: might want to split view from image object
    const vk::ImageViewCreateInfo imageViewCreateInfo {
        .image = m_Image,
        .viewType = viewType,
        .format = imageCreateInfo.format,
        .subresourceRange {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    m_View = m_ContextDevice.createImageView(imageViewCreateInfo);
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

    const vk::ImageMemoryBarrier barrier {
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = m_Layout,
        .newLayout = layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_Image,
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    command.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);

    g_Renderer->SubmitOneShotCommandBuffer(command, vk::QueueFlagBits::eTransfer, m_MemoryInfo.m_TransferFence);

    m_ContextDevice.waitIdle();
    m_Layout = layout;
}

void ImageObject::CopyFrom(const BufferObject& source, const vk::DeviceSize& offset)
{
    const vk::CommandBuffer copyCommand = g_Renderer->CreateOneShotCommandBuffer();

    const vk::BufferImageCopy copyRegion {
        .bufferOffset = source.GetMemoryInfo().m_Offset + offset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset { 0, 0, 0 },  // todo: could support offset copy
        .imageExtent = m_Extent,
    };

    copyCommand.copyBufferToImage(source.GetBuffer(), m_Image, vk::ImageLayout::eTransferDstOptimal, copyRegion);

    g_Renderer->SubmitOneShotCommandBuffer(copyCommand, vk::QueueFlagBits::eTransfer, m_MemoryInfo.m_TransferFence);

    // todo: wait for fence maybe?
    m_ContextDevice.waitIdle();
}

/* static */
ImageObject* ImageObject::CreateImage(const vk::Format& format, const vk::Extent3D extent, const vk::ImageUsageFlags& usageFlags, const vk::ImageViewType& viewType,
                                      const vk::ImageAspectFlags& aspectFlags, const bool isHostVisible, const void* source)
{
    const std::set<uint32_t> uniqueIndices { g_Renderer->GetQueueIndex(QueueType::Graphics), g_Renderer->GetQueueIndex(QueueType::Transfer) };
    const std::vector<uint32_t> indices { uniqueIndices.begin(), uniqueIndices.end() };

    const vk::ImageCreateInfo imageCreateInfo {
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .tiling = isHostVisible ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal,
        .usage = usageFlags,
        .queueFamilyIndexCount = static_cast<uint32_t>(indices.size()),
        .pQueueFamilyIndices = indices.data(),
        .initialLayout = source == nullptr ? vk::ImageLayout::eUndefined : vk::ImageLayout::eTransferDstOptimal,
    };

    const vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    ImageObject* imageObject = new ImageObject { imageCreateInfo, memoryProperties, viewType, aspectFlags };

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
