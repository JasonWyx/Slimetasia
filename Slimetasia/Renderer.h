#pragma once
#ifndef USE_VULKAN
#include <GL\glew.h>
#include <Windows.h>

#include <map>

#include "DebugPass.h"
#include "EditorCamera.h"
#include "FinalPass.h"
#include "GeometryPass.h"
#include "ISystem.h"
#include "LightPass.h"
#include "ParticlePass.h"
#include "PostProcessPass.h"
#include "Shader.h"
#include "ShadowPass.h"
#include "SkyboxPass.h"
#include "WaterPass.h"

class Renderer : public ISystem<Renderer>
{
private:

    friend class ISystem<Renderer>;

    Renderer(iVector2 const& viewportSize);
    ~Renderer();

    iVector2 m_WindowSize;
    bool m_IsWireframeMode;
    bool m_IsDebugDrawOn;
    bool m_UseCamera;

    HShader m_SimpleDrawShader;

    GLuint m_ScreenQuadVAO;
    GLuint m_ScreenQuadVBO;

    // Framebuffer
    GLuint m_Framebuffer;

    // Render textures/buffers
    GLuint m_FinalRenderTarget;
    GLuint m_TempRenderTarget;
    GLuint m_DSBuffer;

    GeometryPass m_GeometryPass;
    ShadowPass m_ShadowPass;
    LightPass m_LightPass;
    DebugPass m_DebugPass;
    SkyboxPass m_SkyboxPass;
    ParticlePass m_ParticlePass;
    WaterPass m_WaterPass;
    PostProcessPass m_PostProcessPass;
    FinalPass m_FinalPass;

    SceneLayer* m_CurrentLayer;

    int m_RenderIndex;

    void BuildRenderTargets();

public:

    void Update(float dt);

    void DrawDebug(unsigned layerId, std::vector<Vector3> const& geometry, Color4 color, DebugPrimitiveType type);
    void DrawCube(float w, Vector3 pos);
    void Draw2DBox(float w, float h, Vector3 pos, Color4 color = Color4(0., 0., 1., 1.));
    void DrawLine(const Vector3& s, const Vector3& e, Color4 color = Color4(0., 0., 1., 1.));
    void DrawDebugBox(unsigned layerId, Color4 color);
    void DrawSelectionBox(float left, float right, float top, float bottom);

    void SetCurrentLayer(SceneLayer* layer);
    void SetWindowSize(iVector2 const& windowSize);
    iVector2 GetWindowSize() const;

    SceneLayer* GetCurrentEditorLayer() const { return m_CurrentLayer; }
    GLuint GetRenderTexture() const;
    GLuint GetPickedObject(iVector2 mousePosition) const;
    void SetSelectedObjects(std::vector<unsigned> const& selectedObjects);
    void ChangeCamera(bool b);
};

#endif // !USE_VULKAN