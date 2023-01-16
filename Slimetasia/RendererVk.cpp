#include "RendererVk.h"

#include <vulkan/vulkan_win32.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <set>

#include "External Libraries/imgui/backends/imgui_impl_vulkan.h"
#include "ShaderHelper.h"

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

RendererVk::RendererVk(HINSTANCE appInstance, HWND appWindow, const uint32_t windowWidth, const uint32_t windowHeight)
    : m_CurrentFrame { 0 }
{
    CreateInstance();
    CreateSurface(appInstance, appWindow);
    ChoosePhysicalDevice();
    CreateDevice();
    CreateSwapchain(appWindow);
    CreateDescriptorPool();
    CreateCommandPool();
    CreateRenderPass();
    CreateFramebuffers();
    CreatePipelineLayout();
    CreatePipeline();
    CreateSyncObjects();

    InitializeImGui();
}

RendererVk::~RendererVk()
{
    ShutdownImGui();

    std::ranges::for_each(m_InFlightFences, [this](const vk::Fence& fence) { m_Device.destroyFence(fence); });
    std::ranges::for_each(m_RenderFinishedSemaphores, [this](const vk::Semaphore& semaphore) { m_Device.destroySemaphore(semaphore); });
    std::ranges::for_each(m_ImageAvailableSemaphores, [this](const vk::Semaphore& semaphore) { m_Device.destroySemaphore(semaphore); });
    m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
    m_Device.destroyCommandPool(m_CommandPool);
    m_Device.destroyPipeline(m_Pipeline);
    m_Device.destroyPipelineLayout(m_PipelineLayout);
    std::ranges::for_each(m_Framebuffers, [this](const vk::Framebuffer& framebuffer) { m_Device.destroyFramebuffer(framebuffer); });
    m_Device.destroyRenderPass(m_RenderPass);
    m_Device.destroyDescriptorPool(m_DescriptorPool);
    m_SwapchainHandler.release();
    m_Device.destroy();
    m_Instance.destroy();
}

void RendererVk::Update(const float deltaTime)
{
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    const vk::Result waitResult = m_Device.waitForFences(m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    m_Device.resetFences(m_InFlightFences[m_CurrentFrame]);

    const uint32_t imageIndex = m_SwapchainHandler->AcquireNextImageIndex(m_ImageAvailableSemaphores[m_CurrentFrame]);

    const std::vector<vk::Semaphore> waitSemaphores { m_ImageAvailableSemaphores[m_CurrentFrame] };
    const std::vector<vk::Semaphore> signalSemaphores { m_RenderFinishedSemaphores[m_CurrentFrame] };

    const vk::CommandBuffer commandBuffer { m_CommandBuffers[m_CurrentFrame] };
    commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo {};
    commandBuffer.begin(beginInfo);

    const vk::ClearValue clearColor {
        .color = { .float32 = {{ 0.0f, 0.0f, 0.0f, 1.0f }}, },
    };
    const vk::RenderPassBeginInfo renderPassBeginInfo {
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers[imageIndex],
        .renderArea = { .extent = m_SwapchainHandler->GetExtent() },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    // DrawImGui(commandBuffer);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    const vk::SubmitInfo commandSubmitInfo {
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = &waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data(),
    };

    m_GraphicsQueue.submit(commandSubmitInfo, m_InFlightFences[m_CurrentFrame]);

    const vk::SwapchainKHR swapchain = m_SwapchainHandler->GetSwapchain();

    vk::PresentInfoKHR presentInfo {
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
    };

    const vk::Result presentResult = m_PresentQueue.presentKHR(presentInfo);
}

void RendererVk::CreateInstance()
{
    // Instance layers
    const std::vector<const char*> layerNames
    {
#if defined(_DEBUG)
        "VK_LAYER_KHRONOS_validation"
#endif  // #if defined(_DEBUG)
        "VK_LAYER_KHRONOS_validation"
    };

    // Instance extensions
    const std::vector<const char*> extensionNames
    {
        VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if defined(_DEBUG)
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif  // #if defined(_DEBUG)
    };

    // Check instance layers exists
    const std::vector<vk::LayerProperties> layerPropertiesList = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : layerNames)
    {
        bool matchFound = false;

        for (const vk::LayerProperties& layerProperties : layerPropertiesList)
        {
            if (std::strcmp(layerName, layerProperties.layerName.data()))
            {
                matchFound = true;
            }
        }

        if (!matchFound)
        {
            std::cerr << "Required vulkan instance layer missing: " << layerName << std::endl;
        }
    }

    const vk::ApplicationInfo applicationInfo {
        .pApplicationName = "Slimetasia",
        .applicationVersion = 1,
        .pEngineName = "Slimetasia Engine",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3,
    };

    const vk::InstanceCreateInfo createInfo {
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(layerNames.size()),
        .ppEnabledLayerNames = layerNames.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensionNames.size()),
        .ppEnabledExtensionNames = extensionNames.data(),
    };

    m_Instance = vk::createInstance(createInfo);
}

void RendererVk::CreateSurface(const HINSTANCE hInstance, const HWND hWindow)
{
    const vk::Win32SurfaceCreateInfoKHR createInfo {
        .hinstance = hInstance,
        .hwnd = hWindow,
    };

    m_Surface = m_Instance.createWin32SurfaceKHR(createInfo);
}

void RendererVk::ChoosePhysicalDevice()
{
    const std::vector<vk::PhysicalDevice> physicalDevices = m_Instance.enumeratePhysicalDevices();

    // Assuming only 1 physical device for now.
    // TODO: Implement choosing and scoring of devices

    m_PhysicalDevice = physicalDevices.front();
}

void RendererVk::CreateDevice()
{
    ASSERT(m_PhysicalDevice);
    ASSERT(m_Surface);

    // Getting queue indices
    const std::vector<vk::QueueFamilyProperties> queueFamilyPropertiesList = m_PhysicalDevice.getQueueFamilyProperties();

    uint32_t queueIndex = 0;
    for (const vk::QueueFamilyProperties& queueFamilyProperties : queueFamilyPropertiesList)
    {
        if (m_PhysicalDevice.getSurfaceSupportKHR(queueIndex, m_Surface))
        {
            m_PresentQueueIndex = queueIndex;
        }

        if (queueFamilyProperties.queueFlags | vk::QueueFlagBits::eGraphics)
        {
            m_GraphicsQueueIndex = queueIndex;
        }

        if (m_PresentQueueIndex.has_value() && m_GraphicsQueueIndex.has_value())
        {
            break;
        }

        queueIndex++;
    }

    // Create queue only for each unique index
    const float queuePriority = 1.0f;
    const std::set<uint32_t> uniqueQueueIndices { m_PresentQueueIndex.value(), m_GraphicsQueueIndex.value() };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};

    for (const uint32_t uniqueQueueIndex : uniqueQueueIndices)
    {
        const vk::DeviceQueueCreateInfo queueCreateInfo {
            .queueFamilyIndex = uniqueQueueIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }

    const std::vector<const char*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    vk::DeviceCreateInfo createInfo {
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    // No Layers
    // No Extensions
    // No PhysicalDeviceFeatures

    m_Device = m_PhysicalDevice.createDevice(createInfo);

    ASSERT(m_Device);

    m_PresentQueue = m_Device.getQueue(m_PresentQueueIndex.value(), 0);
    m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueIndex.value(), 0);
}

void RendererVk::CreateSwapchain(HWND appWindow)
{
    m_SwapchainHandler = std::make_unique<SwapchainHandler>(m_PhysicalDevice, m_Device, m_Surface, appWindow);
}

void RendererVk::CreateDescriptorPool()
{
    ASSERT(m_Device);

    std::vector<vk::DescriptorPoolSize> poolSizes { { vk::DescriptorType::eSampler, 1000 },
                                                    { vk::DescriptorType::eCombinedImageSampler, 1000 },
                                                    { vk::DescriptorType::eSampledImage, 1000 },
                                                    { vk::DescriptorType::eStorageImage, 1000 },
                                                    { vk::DescriptorType::eUniformTexelBuffer, 1000 },
                                                    { vk::DescriptorType::eStorageTexelBuffer, 1000 },
                                                    { vk::DescriptorType::eUniformBuffer, 1000 },
                                                    { vk::DescriptorType::eStorageBuffer, 1000 },
                                                    { vk::DescriptorType::eUniformBufferDynamic, 1000 },
                                                    { vk::DescriptorType::eStorageBufferDynamic, 1000 },
                                                    { vk::DescriptorType::eInputAttachment, 1000 } };

    vk::DescriptorPoolCreateInfo createInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1 << 10,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    m_DescriptorPool = m_Device.createDescriptorPool(createInfo);
}

void RendererVk::CreateCommandPool()
{
    vk::CommandPoolCreateInfo createInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_GraphicsQueueIndex.value(),
    };

    m_CommandPool = m_Device.createCommandPool(createInfo);

    // Create command buffer for imgui
    vk::CommandBufferAllocateInfo allocateInfo {
        .commandPool = m_CommandPool,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    m_CommandBuffers = m_Device.allocateCommandBuffers(allocateInfo);
}

void RendererVk::CreatePipeline()
{
    ASSERT(m_Device);
    ASSERT(static_cast<bool>(m_SwapchainHandler));

    const std::vector<char>& vertSpirv = ShaderHelper::CompileToSpirv("SimpleTriangle.vert");
    const std::vector<char>& fragSpirv = ShaderHelper::CompileToSpirv("SimpleTriangle.frag");

    const vk::ShaderModuleCreateInfo vertModuleCreateInfo {
        .codeSize = vertSpirv.size(),
        .pCode = reinterpret_cast<const uint32_t*>(vertSpirv.data()),
    };

    const vk::ShaderModuleCreateInfo fragModuleCreateInfo {
        .codeSize = fragSpirv.size(),
        .pCode = reinterpret_cast<const uint32_t*>(fragSpirv.data()),
    };

    const vk::ShaderModule vertShader { m_Device.createShaderModule(vertModuleCreateInfo) };
    const vk::ShaderModule fragShader { m_Device.createShaderModule(fragModuleCreateInfo) };

    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages {
        {
            // vert shader stage
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertShader,
            .pName = "main",
        },
        {
            // frag shader stage
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragShader,
            .pName = "main",
        },
    };

    const vk::PipelineVertexInputStateCreateInfo vertexInputState {
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState {
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    const vk::Extent2D swapchainExtent = m_SwapchainHandler->GetExtent();

    const vk::Viewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const vk::Rect2D scissor {
        .offset = { 0, 0 },
        .extent = swapchainExtent,
    };

    const vk::PipelineViewportStateCreateInfo viewportState {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    const vk::PipelineRasterizationStateCreateInfo rasterizationState {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    const vk::PipelineMultisampleStateCreateInfo multisampleState {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    const vk::PipelineDepthStencilStateCreateInfo depthStencilState {};  // todo: setup depth stencil

    const vk::PipelineColorBlendAttachmentState colorBlendAttachmentState {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendState {
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = { { 0.0f, 0.0f, 0.0f, 0.0f } },
    };

    const std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    const vk::PipelineDynamicStateCreateInfo dynamicState {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    const vk::GraphicsPipelineCreateInfo pipelineCreateInfo {
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        //.pDynamicState = &dynamicState,
        .layout = m_PipelineLayout,
        .renderPass = m_RenderPass,
        .subpass = 0,
    };

    const vk::ResultValue<vk::Pipeline>& resultValue = m_Device.createGraphicsPipeline({}, pipelineCreateInfo);
    ASSERT(resultValue.result == vk::Result::eSuccess);

    m_Pipeline = resultValue.value;
}

void RendererVk::CreateRenderPass()
{
    ASSERT(static_cast<bool>(m_SwapchainHandler));
    ASSERT(m_Device);

    const vk::AttachmentDescription colorAttachment {
        .format = m_SwapchainHandler->GetSurfaceFormat().format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };

    const vk::AttachmentReference colorAttachmentRef {
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    const vk::SubpassDescription subpass {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    // Initial dependency to transition 
    const vk::SubpassDependency dependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    };

    const vk::RenderPassCreateInfo createInfo {
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    m_RenderPass = m_Device.createRenderPass(createInfo);
}

void RendererVk::CreateFramebuffers()
{
    const vk::Extent2D extent = m_SwapchainHandler->GetExtent();
    const std::vector<vk::ImageView> imageViews = m_SwapchainHandler->GetImageViews();

    for (size_t i = 0; i < imageViews.size(); ++i)
    {
        const vk::ImageView imageView = imageViews[i];

        vk::FramebufferCreateInfo createInfo {
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .pAttachments = &imageView,
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };

        m_Framebuffers.push_back(m_Device.createFramebuffer(createInfo));
    }
}

void RendererVk::CreatePipelineLayout()
{
    ASSERT(m_Device);

    // For uniform data and sorts
    const vk::PipelineLayoutCreateInfo createInfo {};

    m_PipelineLayout = m_Device.createPipelineLayout(createInfo);
}

void RendererVk::CreateSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo {};
    vk::FenceCreateInfo fenceCreateInfo {};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphores.push_back(m_Device.createSemaphore(semaphoreCreateInfo));
        m_RenderFinishedSemaphores.push_back(m_Device.createSemaphore(semaphoreCreateInfo));
        m_InFlightFences.push_back(m_Device.createFence(fenceCreateInfo));
    }
}

vk::CommandBuffer RendererVk::CreateOneShotCommandBuffer()
{
    ASSERT(m_Device);

    const vk::CommandBufferAllocateInfo allocateInfo {
        .commandPool = m_CommandPool,
        .commandBufferCount = 1,
    };

    vk::CommandBuffer commandBuffer = m_Device.allocateCommandBuffers(allocateInfo).front();

    const vk::CommandBufferBeginInfo beginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void RendererVk::SubmitOneShotCommandBuffer(const vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    m_GraphicsQueue.submit(submitInfo);
    m_Device.waitIdle();
    m_Device.freeCommandBuffers(m_CommandPool, { commandBuffer });
}

void RendererVk::InitializeImGui()
{
    ImGui::CreateContext();

    ImGui_ImplVulkan_InitInfo imguiInitInfo {
        .Instance = m_Instance,
        .PhysicalDevice = m_PhysicalDevice,
        .Device = m_Device,
        .QueueFamily = m_GraphicsQueueIndex.value_or(0),
        .Queue = m_GraphicsQueue,
        .DescriptorPool = m_DescriptorPool,
        .Subpass = 0,
        .MinImageCount = MAX_FRAMES_IN_FLIGHT,
        .ImageCount = static_cast<uint32_t>(m_SwapchainHandler->GetImageViews().size()),
    };

    ImGui_ImplVulkan_Init(&imguiInitInfo, m_RenderPass);
    const vk::CommandBuffer createFontCommands = CreateOneShotCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(createFontCommands);
    SubmitOneShotCommandBuffer(createFontCommands);
}

void RendererVk::ShutdownImGui()
{
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    ImGui_ImplVulkan_Shutdown();
}

void RendererVk::DrawImGui(const vk::CommandBuffer commandBuffer)
{
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData == nullptr)
    {
        return;
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}
