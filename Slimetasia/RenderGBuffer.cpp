#include "RenderGBuffer.h"

#include "Application.h"
#include "Camera.h"
#include "CorePrerequisites.h"
#include "Layer.h"
#include "Mesh.h"
#include "RenderLayer.h"
#include "Scene.h"
#include "MeshRenderer.h"

struct UniformBufferObject
{
    Matrix4 ModelTransform;
    Matrix4 NodeTransform;
    Matrix4 ViewProjectionTransform;
    Matrix4 BoneTransforms[RenderGBuffer::MAX_BONES];
    Vector4 ClipPlane;
    Vector4 MeshColor;
    iVector4 TextureFlags;  // todo: maybe not needed? just initialze to some default color
    uint32_t PickingID;
};

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
    const vk::CommandBuffer& commandBuffer = m_CommandBuffers[frameInfo.m_FrameIndex];
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.begin(beginInfo);

    const std::vector<vk::ClearValue> clearValues { GBufferIndex::Count + 1 };  // +1 for depth buffer
    const vk::RenderPassBeginInfo renderPassBeginInfo { m_RenderPass, m_Framebuffers[frameInfo.m_FrameIndex], vk::Rect2D { {}, m_Context.m_RenderExtent }, clearValues };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

    const vk::Viewport viewport { 0, 0, static_cast<float>(m_Context.m_RenderExtent.width), static_cast<float>(m_Context.m_RenderExtent.height) };
    const vk::Rect2D scissor { {}, m_Context.m_RenderExtent };

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    RenderScene(commandBuffer);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::Semaphore& signalSemaphore = m_SignalSemaphores[frameInfo.m_FrameIndex];
    const std::vector<vk::PipelineStageFlags> waitStages { waitSemaphores.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const vk::SubmitInfo commandSubmitInfo { waitSemaphores, waitStages, commandBuffer, signalSemaphore };

    m_Context.m_Queues[QueueType::Graphics].submit(commandSubmitInfo, signalFence);

    return RenderOutputs { signalSemaphore };
}

void RenderGBuffer::SetWindowExtent(const vk::Extent2D& extent) {}

void RenderGBuffer::CreateDescriptors()
{
    // todo: can be optimized. currently is assuming non-instanced rendering.
    const std::vector<vk::DescriptorPoolSize> poolSizes {
        vk::DescriptorPoolSize { vk::DescriptorType::eCombinedImageSampler, m_Context.m_FramesInFlight * MAX_OBJECTS * 4 },  // 4 different texture types
        vk::DescriptorPoolSize { vk::DescriptorType::eUniformBuffer, m_Context.m_FramesInFlight * MAX_OBJECTS },
    };

    const vk::DescriptorPoolCreateInfo createInfo { {}, m_Context.m_FramesInFlight * GBufferIndex::Count, poolSizes };

    // note: Need 1 pool for each thread if doing threaded recording
    m_DescriptorPool = m_Context.m_Device.createDescriptorPool(createInfo);
}

void RenderGBuffer::CreateRenderPass()
{
    const std::vector<vk::AttachmentDescription> attachmentDescriptions {
        vk::AttachmentDescription { {}, vk::Format::eR16G16B16A16Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentDescription { {}, vk::Format::eR16G16B16A16Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentDescription { {}, vk::Format::eR16G16B16A16Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentDescription { {}, vk::Format::eR16G16B16A16Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentDescription { {}, vk::Format::eR16G16B16A16Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
#ifdef EDITOR
        // todo: Check if still need debug texture coordinates and picking ID
        vk::AttachmentDescription { {}, vk::Format::eR16G16B16A16Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentDescription { {}, vk::Format::eR32Uint, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal },
#endif  // EDITOR
        vk::AttachmentDescription { {}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, {}, {}, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal },
    };

    const std::vector<vk::AttachmentReference> colorAttachmentRefs {
        vk::AttachmentReference { GBufferIndex::Diffuse, vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentReference { GBufferIndex::Specular, vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentReference { GBufferIndex::Emissive, vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentReference { GBufferIndex::Position, vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentReference { GBufferIndex::Normal, vk::ImageLayout::eColorAttachmentOptimal },
#ifdef EDITOR
        vk::AttachmentReference { GBufferIndex::TexCoords, vk::ImageLayout::eColorAttachmentOptimal },
        vk::AttachmentReference { GBufferIndex::PickingID, vk::ImageLayout::eColorAttachmentOptimal },
#endif
    };

    // Depth attachment is the last slot
    const vk::AttachmentReference depthAttachmentRef { static_cast<uint32_t>(colorAttachmentRefs.size()), vk::ImageLayout::eDepthStencilAttachmentOptimal };

    const vk::SubpassDescription subpassDescription { {}, vk::PipelineBindPoint::eGraphics, {}, colorAttachmentRefs, {}, &depthAttachmentRef, {} };
    const vk::SubpassDependency subpassDependency { VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite };

    const vk::RenderPassCreateInfo createInfo { {}, attachmentDescriptions, subpassDescription, subpassDependency };

    m_RenderPass = m_Context.m_Device.createRenderPass(createInfo);
}

void RenderGBuffer::CreateFramebuffers()
{
    const vk::Extent3D extent { m_Context.m_RenderExtent.width, m_Context.m_RenderExtent.height, 1 };

    m_GBufferImages.resize(m_Context.m_FramesInFlight);

    for (uint32_t i = 0; i < m_Context.m_FramesInFlight; ++i)
    {
        // Diffuse
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

        // Specular
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

        // Emissive - Only first 3 components used
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

        // Position - Only first 3 components used
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

        // Normal
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

#ifdef EDITOR
        // Debug TexCoords
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR16G16B16A16Sfloat, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

        // Picking ID
        m_GBufferImages[i].push_back(ImageObject::CreateImage(vk::Format::eR32Uint, extent, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, false, nullptr));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        // todo: need to find out how to read ID out of buffer

#endif  // EDITOR

        // DepthStencil
        m_GBufferImages[i].push_back(ImageObject::CreateDepthImage(vk::Format::eD32Sfloat, extent, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled));
        m_GBufferImages[i].back()->GenerateView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth);
        m_GBufferImages[i].back()->GenerateSampler(vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);

        std::vector<vk::ImageView> attachments {};
        std::ranges::for_each(m_GBufferImages[i], [&attachments](const ImageObject* attachmentImage) { attachments.push_back(attachmentImage->GetView()); });

        const vk::FramebufferCreateInfo createInfo { {}, m_RenderPass, attachments, extent.width, extent.height, 1 };

        m_Framebuffers.push_back(m_Context.m_Device.createFramebuffer(createInfo));
    }
}

void RenderGBuffer::DestroyFramebuffers()
{
    for (std::vector<ImageObject*>& framebufferImages : m_GBufferImages)
    {
        std::ranges::for_each(framebufferImages, [](const ImageObject* image) { delete image; });
        framebufferImages.clear();
    }
    m_GBufferImages.clear();
}

void RenderGBuffer::CreatePipeline()
{
    const vk::PipelineLayoutCreateInfo layoutCreateInfo {};

    m_PipelineLayout = m_Context.m_Device.createPipelineLayout(layoutCreateInfo);

    const ShaderModuleObject vertModule { "gbuffer.vert.hlsl", m_Context.m_Device };
    const ShaderModuleObject fragModule { "gbuffer.frag.hlsl", m_Context.m_Device };

    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vk::PipelineShaderStageCreateInfo { {}, vk::ShaderStageFlagBits::eVertex, vertModule.GetModule(), "main" },
        vk::PipelineShaderStageCreateInfo { {}, vk::ShaderStageFlagBits::eFragment, fragModule.GetModule(), "main" },
    };

    // todo: maybe this should be part of mesh description?
    const std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::Position), sizeof(Vector3) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::Normal), sizeof(Vector3) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::Tangent), sizeof(Vector3) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::Bitangent), sizeof(Vector3) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::Color), sizeof(Vector3) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::TexCoord), sizeof(Vector2) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::JointId), sizeof(iVector4) },
        vk::VertexInputBindingDescription { static_cast<uint32_t>(MeshBufferID::JointWeight), sizeof(Vector4) },
    };

    const std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::Position), static_cast<uint32_t>(MeshBufferID::Position), vk::Format::eR32G32B32Sfloat, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::Normal), static_cast<uint32_t>(MeshBufferID::Normal), vk::Format::eR32G32B32Sfloat, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::Tangent), static_cast<uint32_t>(MeshBufferID::Tangent), vk::Format::eR32G32B32Sfloat, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::Bitangent), static_cast<uint32_t>(MeshBufferID::Bitangent), vk::Format::eR32G32B32Sfloat, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::Color), static_cast<uint32_t>(MeshBufferID::Color), vk::Format::eR32G32B32Sfloat, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::TexCoord), static_cast<uint32_t>(MeshBufferID::TexCoord), vk::Format::eR32G32Sfloat, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::JointId), static_cast<uint32_t>(MeshBufferID::JointId), vk::Format::eR32G32B32A32Sint, 0 },
        vk::VertexInputAttributeDescription { static_cast<uint32_t>(MeshBufferID::JointWeight), static_cast<uint32_t>(MeshBufferID::JointWeight), vk::Format::eR32G32B32A32Sfloat, 0 },
    };

    const vk::PipelineVertexInputStateCreateInfo vertexInput { {}, bindingDescriptions, attributeDescriptions };
    const vk::PipelineInputAssemblyStateCreateInfo inputAssembly { {}, vk::PrimitiveTopology::eTriangleList };

    const vk::Extent2D renderExtent = m_Context.m_RenderExtent;
    const vk::Viewport viewport { 0.0f, 0.0f, static_cast<float>(renderExtent.width), static_cast<float>(renderExtent.height), 0.0f, 1.0f };
    const vk::Rect2D scissor { {}, renderExtent };
    const vk::PipelineViewportStateCreateInfo viewportState { {}, viewport, scissor };

    const vk::PipelineRasterizationStateCreateInfo rasterization { {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.f, 0.f,
        0.f, 1.0f };
    const vk::PipelineMultisampleStateCreateInfo multisample { {}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f };
    const vk::PipelineDepthStencilStateCreateInfo depthStencil { {}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess };
    const vk::PipelineColorBlendAttachmentState colorBlendState { VK_FALSE, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags };
    const std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments(GBufferIndex::Count, colorBlendState);
    const vk::PipelineColorBlendStateCreateInfo colorBlend { {}, VK_FALSE, vk::LogicOp::eClear, colorBlendAttachments };
    const std::vector<vk::DynamicState> dynamicStates { { vk::DynamicState::eViewport, vk::DynamicState::eScissor } };
    const vk::PipelineDynamicStateCreateInfo dynamic { {}, dynamicStates };

    const vk::GraphicsPipelineCreateInfo pipelineCreateInfo { {}, shaderStages, &vertexInput, &inputAssembly, {}, &viewportState, &rasterization, &multisample, &depthStencil, &colorBlend, &dynamic,
        m_PipelineLayout, m_RenderPass };

    const vk::ResultValue<vk::Pipeline>& resultValue = m_Context.m_Device.createGraphicsPipeline({}, pipelineCreateInfo);
    ASSERT(resultValue.result == vk::Result::eSuccess);

    m_Pipeline = resultValue.value;
}

void RenderGBuffer::RenderScene(const vk::CommandBuffer& commandBuffer)
{
    Scene* scene = Application::Instance().GetCurrentScene();
    if (scene == nullptr)
    {
        return;
    }

    const std::list<SceneLayer*>& layers = scene->GetLayers();

    // Render for each layer
    for (SceneLayer* layer : layers)
    {
        const RenderLayer& renderLayer = layer->GetRenderLayer();

        // todo: Just get first main camera for now
        const std::vector<Camera*>& cameras = renderLayer.GetCameras();
        const Camera* camera = nullptr;
        for (const Camera* currCamera : cameras)
        {
            if (currCamera != nullptr && currCamera->IsMainCamera())
            {
                camera = currCamera;
                break;
            }
        }

        // No main camera, do nothing
        if (camera == nullptr)
        {
            continue;
        }

        const std::vector<MeshRenderer*>& meshes = renderLayer.GetMeshRenderers();
    }
}