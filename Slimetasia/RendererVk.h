#pragma once
#include <windows.h>

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "ISystem.h"
#include "MemoryHandler.h"
#include "RenderFinalComposition.h"
#include "RenderGBuffer.h"
#include "RenderImGui.h"
#include "SwapchainHandler.h"
#include "QueueType.h"

struct ImDrawData;

class RendererVk : public ISystem<RendererVk>
{
public:

    RendererVk(const HINSTANCE appInstance, const HWND appWindow, const uint32_t windowWidth, const uint32_t windowHeight);
    ~RendererVk();

    void Update(const float deltaTime);
    void OnWindowResize();

    vk::CommandBuffer CreateOneShotCommandBuffer();
    void SubmitOneShotCommandBuffer(const vk::CommandBuffer commandBuffer, const vk::QueueFlagBits targetQueue, const vk::Fence signalFence = {});

    vk::Device GetDevice() const { return m_Device; }
    uint32_t GetQueueIndex(const QueueType queueType) const { return m_QueueIndices[queueType]; }
    vk::Queue GetQueue(const QueueType queueType) const { return m_Queues[queueType]; }

    const std::unique_ptr<SwapchainHandler>& GetSwapchainHandler() const { return m_SwapchainHandler; }
    const std::unique_ptr<MemoryHandler>& GetMemoryHandler() const { return m_MemoryHandler; }

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
    void CreateHandlers();
    void CreateRenderers();
    void CreateCommandPool();
    void CreateFramebuffers();
    void CreateSyncObjects();

    HWND m_AppWindow;

    vk::Instance m_Instance;
    vk::SurfaceKHR m_Surface;
    vk::PhysicalDevice m_PhysicalDevice;
    vk::Device m_Device;

    std::array<uint32_t, QueueType::Count> m_QueueIndices;
    std::array<vk::Queue, QueueType::Count> m_Queues;
    std::unique_ptr<SwapchainHandler> m_SwapchainHandler;
    std::unique_ptr<MemoryHandler> m_MemoryHandler;

    vk::CommandPool m_OneShotCommandPool;

    std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::Fence> m_InFlightFences;
    uint32_t m_CurrentFrame;

    std::unique_ptr<RenderGBuffer> m_RenderGBuffer;
    std::unique_ptr<RenderFinalComposition> m_RenderFinalComposition;
    std::unique_ptr<RenderImGui> m_RenderImGui;
};

#define g_Renderer RendererVk::InstancePtr()