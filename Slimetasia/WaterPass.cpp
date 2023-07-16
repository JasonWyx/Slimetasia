#ifndef USE_VULKAN

#include "WaterPass.h"

#include <stb_image.h>

#include "ResourceManager.h"

WaterPass::WaterPass(const iVector2 viewportSize)
    : m_ViewportSize(viewportSize)
    , m_GeometryPass(viewportSize)
    , m_LightPass(viewportSize)
    , m_SkyboxPass()
    , m_ParticlePass()
    , m_WaterShader(ResourceManager::Instance().CreateResource<Shader>("WaterShader"))
    , m_DUDVTexture(GL_NONE)
    , m_DUDVNormal(GL_NONE)
    , m_Framebuffer(GL_NONE)
    , m_DSBuffer(GL_NONE)
{
    m_WaterShader->SetVertShaderFilePath("WaterShader.vert");
    m_WaterShader->SetFragShaderFilePath("WaterShader.frag");
    m_WaterShader->Compile();

    glCreateFramebuffers(1, &m_Framebuffer);
    glNamedFramebufferDrawBuffer(m_Framebuffer, GL_COLOR_ATTACHMENT0);
    BuildRenderTargets();
    BuildDUDVMaps();

    glCreateVertexArrays(1, &m_VertexArray);
    glCreateBuffers(1, &m_VertexBuffer);

    glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(float) * 5);

    glEnableVertexArrayAttrib(m_VertexArray, 0);
    glVertexArrayAttribBinding(m_VertexArray, 0, 0);
    glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_VertexArray, 1);
    glVertexArrayAttribBinding(m_VertexArray, 1, 0);
    glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);

    GLfloat bufferData[] = { -1, 0, -1, 0, 0, -1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, -1, 1, 0 };

    glNamedBufferStorage(m_VertexBuffer, sizeof(bufferData), bufferData, 0);
}

WaterPass::~WaterPass()
{
    glDeleteTextures(1, &m_DUDVTexture);
    glDeleteTextures(1, &m_DUDVNormal);
    glDeleteBuffers(1, &m_VertexBuffer);
    glDeleteVertexArrays(1, &m_VertexArray);
    glDeleteTextures(1, &m_DSBuffer);
    glDeleteFramebuffers(1, &m_Framebuffer);
}

void WaterPass::SetViewportSize(iVector2 const& viewportSize)
{
    if (m_ViewportSize != viewportSize)
    {
        m_ViewportSize = viewportSize;
        m_GeometryPass.SetViewportSize(m_ViewportSize);
        m_LightPass.SetViewportSize(m_ViewportSize);

        BuildRenderTargets();
    }
}

void WaterPass::Render(Camera* camera, const RenderLayer& renderLayer, GLuint mainFramebuffer)
{
    const std::vector<WaterSimulator*>& waterSimulators = renderLayer.GetWaterSimulators();
    const iVector2 vpSize = camera->GetViewportSize();
    const iVector2 vpOffset = camera->GetViewportOffset();

    for (WaterSimulator* simulator : waterSimulators)
    {
        const Vector3 waterPlanePosition = simulator->GetTransform()->GetWorldPosition();
        SetViewportSize(vpSize * simulator->m_TextureResolutionScale);
        simulator->GenerateTextures(m_ViewportSize);
        camera->SetViewportSize(m_ViewportSize, false);

        glDisable(GL_SCISSOR_TEST);
        // Bind water pass framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
        glViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y);

        // Setup for rendering reflection texture
        camera->SetIsReflectionView(true);
        camera->SetReflectionHeight(simulator->GetTransform()->GetWorldPosition().y);

        // Start threaded culling
        m_GeometryPass.StartMeshCulling(*camera, renderLayer);
        m_LightPass.StartLightCulling(*camera, renderLayer);

        glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, simulator->GetReflectionTexture(), 0);
        glClearColor(0, 0, 0, 0);
        glClearDepth(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CLIP_DISTANCE0);
        m_GeometryPass.SetClipPlane(Vector4(0.0f, 1.0f, 0.0f, -waterPlanePosition.y + 0.5f));
        m_GeometryPass.Render(camera, renderLayer);
        glDisable(GL_CLIP_DISTANCE0);

        glBlitNamedFramebuffer(m_GeometryPass.GetFramebuffer(), m_Framebuffer, 0, 0, m_ViewportSize.x, m_ViewportSize.y, 0, 0, m_ViewportSize.x, m_ViewportSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        m_LightPass.Render(*camera, renderLayer, m_GeometryPass.GetGBuffers(), m_Framebuffer, true);
        m_SkyboxPass.Render(*camera);
        // m_ParticlePass.Render(*camera);

        // Setup for rendering refraction texture
        camera->SetIsReflectionView(false);

        // Start threaded culling
        m_GeometryPass.StartMeshCulling(*camera, renderLayer);
        m_LightPass.StartLightCulling(*camera, renderLayer);

        glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, simulator->GetRefractionTexture(), 0);
        glClearColor(0, 0, 0, 0);
        glClearDepth(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CLIP_DISTANCE0);
        m_GeometryPass.SetClipPlane(Vector4(0.0f, -1.0f, 0.0f, waterPlanePosition.y + 0.5f));
        m_GeometryPass.Render(camera, renderLayer);
        glDisable(GL_CLIP_DISTANCE0);

        glBlitNamedFramebuffer(m_GeometryPass.GetFramebuffer(), m_Framebuffer, 0, 0, m_ViewportSize.x, m_ViewportSize.y, 0, 0, m_ViewportSize.x, m_ViewportSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        m_LightPass.Render(*camera, renderLayer, m_GeometryPass.GetGBuffers(), m_Framebuffer, true);
        m_SkyboxPass.Render(*camera);
        // m_ParticlePass.Render(*camera);

        // Setup for rendering water to scene texture
        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        // Draw water plane to scene
        glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);
        glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
        glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

        Matrix4 vp = camera->GetViewProjTransform();
        Matrix4 m = simulator->GetTransform()->GetWorldTransformMatrix() * Matrix4::Scale(simulator->m_PlaneSize);

        if (m_WaterShader->Enable())
        {
            static const GLuint MTransformLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uMTransform");
            static const GLuint VPTransformLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uVPTransform");
            static const GLuint cameraPositionLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uCameraPosition");
            static const GLuint tilingFactorLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uTilingFactor");
            static const GLuint reflectTexLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uReflectionTex");
            static const GLuint refractTexLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uRefractionTex");
            static const GLuint dudvTexLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uDUDVTex");
            static const GLuint dudvNormTexLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uDUDVNorm");
            static const GLuint depthTexLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uDepthTex");
            static const GLuint fresnelPowerLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uFresnelPower");
            static const GLuint waveFactorLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uWaveFactor");
            static const GLuint waveStrengthLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uWaveStrength");
            static const GLuint waterDepthLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uWaterDepth");
            static const GLuint waterColorLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uWaterColor");
            static const GLuint camNearLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uCamNear");
            static const GLuint camFarLoc = glGetUniformLocation(m_WaterShader->GetProgramHandle(), "uCamFar");

            glUniformMatrix4fv(MTransformLoc, 1, GL_FALSE, ValuePtrFloat(m));
            glUniformMatrix4fv(VPTransformLoc, 1, GL_FALSE, ValuePtrFloat(vp));
            glUniform1i(reflectTexLoc, 0);
            glUniform1i(refractTexLoc, 1);
            glUniform1i(depthTexLoc, 2);
            glUniform1i(dudvTexLoc, 3);
            glUniform1i(dudvNormTexLoc, 4);
            glUniform1f(fresnelPowerLoc, simulator->m_FresnelPower);
            glUniform1f(waveFactorLoc, simulator->GetWaveFactor());
            glUniform1f(waveStrengthLoc, simulator->m_WaveStrength);
            glUniform1f(waterDepthLoc, simulator->m_WaterDepth);
            glUniform1f(tilingFactorLoc, simulator->m_TilingFactor);
            glUniform1f(camNearLoc, camera->GetNearPlane());
            glUniform1f(camFarLoc, camera->GetFarPlane());
            glUniform3fv(waterColorLoc, 1, ValuePtrFloat(simulator->m_WaterColor));
            glUniform3fv(cameraPositionLoc, 1, ValuePtrFloat(camera->GetTransform()->GetWorldPosition()));

            glBindTextureUnit(0, simulator->GetReflectionTexture());
            glBindTextureUnit(1, simulator->GetRefractionTexture());
            glBindTextureUnit(2, m_GeometryPass.GetDepthBuffer());
            glBindTextureUnit(3, m_DUDVTexture);
            glBindTextureUnit(4, m_DUDVNormal);

            glBindVertexArray(m_VertexArray);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    // Restore camer viewport size
    camera->SetViewportSize(vpSize, false);
}

void WaterPass::BuildRenderTargets()
{
    glDeleteTextures(1, &m_DSBuffer);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_DSBuffer);
    glTextureStorage2D(m_DSBuffer, 1, GL_DEPTH24_STENCIL8, m_ViewportSize.x, m_ViewportSize.y);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_DSBuffer, 0);
}

void WaterPass::BuildDUDVMaps()
{
    int width, height, channels;
    unsigned char* pixels = stbi_load("EngineResources/WaterDUDV_T.png", &width, &height, &channels, 3);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_DUDVTexture);
    glTextureStorage2D(m_DUDVTexture, 1, GL_RGB8, width, height);
    glTextureSubImage2D(m_DUDVTexture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTextureParameteri(m_DUDVTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_DUDVTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_DUDVTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_DUDVTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(pixels);

    pixels = stbi_load("EngineResources/WaterDUDV_N.png", &width, &height, &channels, 3);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_DUDVNormal);
    glTextureStorage2D(m_DUDVNormal, 1, GL_RGB8, width, height);
    glTextureSubImage2D(m_DUDVNormal, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glTextureParameteri(m_DUDVNormal, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_DUDVNormal, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_DUDVNormal, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_DUDVNormal, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(pixels);
}

#endif // !USE_VULKAN