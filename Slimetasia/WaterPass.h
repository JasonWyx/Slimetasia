#pragma once
#ifndef USE_VULKAN

#include "GeometryPass.h"
#include "Gl\glew.h"
#include "LightPass.h"
#include "ParticlePass.h"
#include "RenderPass.h"
#include "SkyboxPass.h"

class WaterPass
{
public:

    WaterPass(const iVector2 viewportSize);
    ~WaterPass();

    void Render(Camera* camera, const RenderLayer& renderLayer, GLuint mainFramebuffer);
    void SetViewportSize(iVector2 const& viewportSize);

private:

    iVector2 m_ViewportSize;
    GeometryPass m_GeometryPass;
    LightPass m_LightPass;
    SkyboxPass m_SkyboxPass;
    ParticlePass m_ParticlePass;

    HShader m_WaterShader;
    GLuint m_DUDVTexture;
    GLuint m_DUDVNormal;

    GLuint m_Framebuffer;
    GLuint m_DSBuffer;

    GLuint m_VertexArray;
    GLuint m_VertexBuffer;

private:

    void BuildRenderTargets();
    void BuildDUDVMaps();
};

#endif // !USE_VULKAN