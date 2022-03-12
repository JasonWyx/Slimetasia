#pragma once
#include "ISystem.h"

#include <cstdint>
#include <windows.h>
#include <vulkan/vulkan.hpp>

class RendererVk : public ISystem<RendererVk>
{
public:

    RendererVk(const HINSTANCE hInstance, const HWND hWindow, const uint32_t windowWidth, const uint32_t windowHeight);
    ~RendererVk();

    void Update(const float deltaTime);

    //void SetCurrentLayer(Layer* layer);
    //void SetWindowSize(iVector2 const& windowSize);
    //iVector2 GetWindowSize() const;

    //Layer* GetCurrentEditorLayer() const { return m_CurrentLayer; }
    //GLuint GetRenderTexture() const;
    //GLuint GetPickedObject(iVector2 mousePosition) const;
    //void SetSelectedObjects(std::vector<unsigned> const& selectedObjects);
    //void ChangeCamera(bool b);

    //// Debug draw
    //void DrawDebug(unsigned layerId, std::vector<Vector3> const& geometry, Color4 color, DebugPrimitiveType type);
    //void DrawCube(float w, Vector3 pos);
    //void Draw2DBox(float w, float h, Vector3 pos, Color4 color = Color4(0., 0., 1., 1.));
    //void DrawLine(const Vector3& s, const Vector3& e, Color4 color = Color4(0., 0., 1., 1.));
    //void DrawDebugBox(unsigned layerId, Color4 color);
    //void DrawSelectionBox(float left, float right, float top, float bottom);

private:

    void CreateInstance();
    void CreateSurface(const HINSTANCE hInstance, const HWND hWindow);
    void ChoosePhysicalDevice();
    void CreateDevice();
    void CreateSwapchain();

    vk::Instance m_Instance;
    vk::SurfaceKHR m_Surface;
    vk::PhysicalDevice m_PhysicalDevice;
    vk::Device m_Device;
    vk::Queue m_PresentQueue;
    vk::Queue m_GraphicsQueue;

};
