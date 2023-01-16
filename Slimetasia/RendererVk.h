#pragma once
#include <windows.h>

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "ISystem.h"
#include "SwapchainHandler.h"

struct ImDrawData;

class RendererVk : public ISystem<RendererVk>
{
public:

    RendererVk(const HINSTANCE appInstance, const HWND appWindow, const uint32_t windowWidth, const uint32_t windowHeight);
    ~RendererVk();

    void Update(const float deltaTime);
    // void SetCurrentLayer(Layer* layer);
    // void SetWindowSize(iVector2 const& windowSize);
    // iVector2 GetWindowSize() const;

    // Layer* GetCurrentEditorLayer() const { return m_CurrentLayer; }
    // GLuint GetRenderTexture() const;
    // GLuint GetPickedObject(iVector2 mousePosition) const;
    // void SetSelectedObjects(std::vector<unsigned> const& selectedObjects);
    // void ChangeCamera(bool b);

    //// Debug draw
    // void DrawDebug(unsigned layerId, std::vector<Vector3> const& geometry, Color4 color, DebugPrimitiveType type);
    // void DrawCube(float w, Vector3 pos);
    // void Draw2DBox(float w, float h, Vector3 pos, Color4 color = Color4(0., 0., 1., 1.));
    // void DrawLine(const Vector3& s, const Vector3& e, Color4 color = Color4(0., 0., 1., 1.));
    // void DrawDebugBox(unsigned layerId, Color4 color);
    // void DrawSelectionBox(float left, float right, float top, float bottom);

private:

    void CreateInstance();
    void CreateSurface(const HINSTANCE appInstance, const HWND appWindow);
    void ChoosePhysicalDevice();
    void CreateDevice();
    void CreateSwapchain(const HWND appWindow);
    void CreateDescriptorPool();
    void CreateCommandPool();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateSyncObjects();

    vk::CommandBuffer CreateOneShotCommandBuffer();
    void SubmitOneShotCommandBuffer(const vk::CommandBuffer commandBuffer);

    // ImGui
    void InitializeImGui();
    void ShutdownImGui();
    void DrawImGui(const vk::CommandBuffer commandBuffer);

    vk::Instance m_Instance;
    vk::SurfaceKHR m_Surface;
    vk::PhysicalDevice m_PhysicalDevice;
    vk::Device m_Device;

    std::optional<uint32_t> m_PresentQueueIndex;
    std::optional<uint32_t> m_GraphicsQueueIndex;
    vk::Queue m_PresentQueue;
    vk::Queue m_GraphicsQueue;
    std::unique_ptr<SwapchainHandler> m_SwapchainHandler;

    vk::DescriptorPool m_DescriptorPool;
    vk::CommandPool m_CommandPool;
    std::vector<vk::CommandBuffer> m_CommandBuffers;

    vk::RenderPass m_RenderPass;
    std::vector<vk::Framebuffer> m_Framebuffers;
    vk::PipelineLayout m_PipelineLayout;
    vk::Pipeline m_Pipeline;

    std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::Fence> m_InFlightFences;
    uint32_t m_CurrentFrame;
};
