#include "RenderGBuffer.h"

#include "CorePrerequisites.h"
#include "Mesh.h"

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
    DestroyDescriptors();
    DestroyRenderPass();
    DestroyFramebuffers();
    DestroyPipeline();
}

RenderOutputs RenderGBuffer::Render(const FrameInfo& frameInfo, const std::vector<vk::Semaphore>& waitSemaphores, const vk::Fence& signalFence)
{
    return {};
}

void RenderGBuffer::SetWindowExtent(const vk::Extent2D& extent) {}

void RenderGBuffer::CreateDescriptors() {}

void RenderGBuffer::CreateRenderPass() {}

void RenderGBuffer::CreateFramebuffers()
{
    const vk::Extent3D extent { m_Context.m_RenderExtent.width, m_Context.m_RenderExtent.height, 1 };

    // DepthStencil
    m_GBufferImages.push_back(ImageObject::CreateDepthImage(vk::Format::eD32Sfloat, extent, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

    // Diffuse
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

    // Specular
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

    // Emissive - Only first 3 components used
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

    // Position - Only first 3 components used
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

    // Normal
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

#ifdef EDITOR
    // Debug TexCoords
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
    m_GBufferImages.back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

    // Picking ID
    m_GBufferImages.push_back(ImageObject::CreateImage(vk::Format::eR32Uint, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
    m_GBufferImages.back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
#endif  // EDITOR
}

void RenderGBuffer::DestroyFramebuffers()
{
    std::ranges::for_each(m_GBufferImages, [](const ImageObject* image) { delete image; });
    m_GBufferImages.clear();
}

void RenderGBuffer::CreatePipeline()
{
    const vk::PipelineLayoutCreateInfo layoutCreateInfo {

    };

    m_PipelineLayout = m_Context.m_Device.createPipelineLayout(layoutCreateInfo);

    ShaderModuleObject vertModule { "gbuffer.vert.hlsl", m_Context.m_Device };
    ShaderModuleObject fragModule { "gbuffer.frag.hlsl", m_Context.m_Device };

    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages { {

        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertModule.GetModule(),
            .pName = "main",
        },

        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragModule.GetModule(),
            .pName = "main",
        },

    } };

    // todo: maybe this should be part of mesh description?
    const std::vector<vk::VertexInputBindingDescription> bindingDescriptions { {

        {
            // Position
            .binding = static_cast<uint32_t>(MeshBufferID::Position),
            .stride = sizeof(Vector3),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Normal
            .binding = static_cast<uint32_t>(MeshBufferID::Normal),
            .stride = sizeof(Vector3),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Tangent
            .binding = static_cast<uint32_t>(MeshBufferID::Tangent),
            .stride = sizeof(Vector3),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Bitangent
            .binding = static_cast<uint32_t>(MeshBufferID::Bitangent),
            .stride = sizeof(Vector3),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Color
            .binding = static_cast<uint32_t>(MeshBufferID::Color),
            .stride = sizeof(Vector3),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Texture Coords
            .binding = static_cast<uint32_t>(MeshBufferID::TexCoord),
            .stride = sizeof(Vector2),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Joint ID
            .binding = static_cast<uint32_t>(MeshBufferID::JointId),
            .stride = sizeof(iVector4),
            .inputRate = vk::VertexInputRate::eVertex,
        },

        {
            // Joint Weight
            .binding = static_cast<uint32_t>(MeshBufferID::JointWeight),
            .stride = sizeof(Vector4),
            .inputRate = vk::VertexInputRate::eVertex,
        },

    } };

    const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions { {

        {
            // Position
            .location = static_cast<uint32_t>(MeshBufferID::Position),
            .binding = static_cast<uint32_t>(MeshBufferID::Position),
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = 0,
        },

        {
            // Normal
            .location = static_cast<uint32_t>(MeshBufferID::Normal),
            .binding = static_cast<uint32_t>(MeshBufferID::Normal),
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = 0,
        },

        {
            // Tangent
            .location = static_cast<uint32_t>(MeshBufferID::Tangent),
            .binding = static_cast<uint32_t>(MeshBufferID::Tangent),
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = 0,
        },

        {
            // Bitangent
            .location = static_cast<uint32_t>(MeshBufferID::Bitangent),
            .binding = static_cast<uint32_t>(MeshBufferID::Bitangent),
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = 0,
        },

        {
            // Color
            .location = static_cast<uint32_t>(MeshBufferID::Color),
            .binding = static_cast<uint32_t>(MeshBufferID::Color),
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = 0,
        },

        {
            // Texture Coords
            .location = static_cast<uint32_t>(MeshBufferID::TexCoord),
            .binding = static_cast<uint32_t>(MeshBufferID::TexCoord),
            .format = vk::Format::eR32G32Sfloat,
            .offset = 0,
        },

        {
            // Joint IDs
            .location = static_cast<uint32_t>(MeshBufferID::JointId),
            .binding = static_cast<uint32_t>(MeshBufferID::JointId),
            .format = vk::Format::eR32G32B32A32Sint,
            .offset = 0,
        },

        {
            // Joint Weights
            .location = static_cast<uint32_t>(MeshBufferID::JointWeight),
            .binding = static_cast<uint32_t>(MeshBufferID::JointWeight),
            .format = vk::Format::eR32G32B32A32Sfloat,
            .offset = 0,
        },

    } };

    const vk::PipelineVertexInputStateCreateInfo inputStateCreateInfo {
        .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
        .pVertexBindingDescriptions = bindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    const vk::PipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo {
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    const vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo { .cullMode = vk::CullModeFlagBits::eBack };

    // const void *                                                       pNext               = {};
    // VULKAN_HPP_NAMESPACE::PipelineCreateFlags                          flags               = {};
    // uint32_t                                                           stageCount          = {};
    // const VULKAN_HPP_NAMESPACE::PipelineShaderStageCreateInfo *        pStages             = {};
    // const VULKAN_HPP_NAMESPACE::PipelineVertexInputStateCreateInfo *   pVertexInputState   = {};
    // const VULKAN_HPP_NAMESPACE::PipelineInputAssemblyStateCreateInfo * pInputAssemblyState = {};
    // const VULKAN_HPP_NAMESPACE::PipelineTessellationStateCreateInfo *  pTessellationState  = {};
    // const VULKAN_HPP_NAMESPACE::PipelineViewportStateCreateInfo *      pViewportState      = {};
    // const VULKAN_HPP_NAMESPACE::PipelineRasterizationStateCreateInfo * pRasterizationState = {};
    // const VULKAN_HPP_NAMESPACE::PipelineMultisampleStateCreateInfo *   pMultisampleState   = {};
    // const VULKAN_HPP_NAMESPACE::PipelineDepthStencilStateCreateInfo *  pDepthStencilState  = {};
    // const VULKAN_HPP_NAMESPACE::PipelineColorBlendStateCreateInfo *    pColorBlendState    = {};
    // const VULKAN_HPP_NAMESPACE::PipelineDynamicStateCreateInfo *       pDynamicState       = {};
    // VULKAN_HPP_NAMESPACE::PipelineLayout                               layout              = {};
    // VULKAN_HPP_NAMESPACE::RenderPass                                   renderPass          = {};
    // uint32_t                                                           subpass             = {};
    // VULKAN_HPP_NAMESPACE::Pipeline                                     basePipelineHandle  = {};
    // int32_t                                                            basePipelineIndex   = {};

    const vk::GraphicsPipelineCreateInfo pipelineCreateInfo {
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &inputStateCreateInfo,
    };
}