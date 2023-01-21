#include "Renderer.h"

#include "Application.h"
#include "CorePrerequisites.h"
#include "GameObject.h"
#include "Input.h"
#include "Layer.h"
#include "RenderLayer.h"
#include "ResourceManager.h"
#include "Scene.h"

Renderer::Renderer(iVector2 const& viewportSize)
    : m_SimpleDrawShader(ResourceManager::Instance().CreateResource<Shader>("SimpleDrawShader"))
    , m_Framebuffer(0)
    , m_FinalRenderTarget(0)
    , m_TempRenderTarget(0)
    , m_DSBuffer(0)
    , m_WindowSize(viewportSize)
    , m_IsWireframeMode(false)
    , m_IsDebugDrawOn(false)
    , m_GeometryPass(viewportSize)
    , m_ShadowPass()
    , m_LightPass(viewportSize)
    , m_ParticlePass()
    , m_WaterPass(viewportSize)
    , m_DebugPass(viewportSize)
    , m_SkyboxPass()
    , m_PostProcessPass(viewportSize)
    , m_FinalPass(viewportSize)
    , m_CurrentLayer(nullptr)
    , m_RenderIndex(0)
    , m_UseCamera(false)
{
    m_SimpleDrawShader->SetVertShaderFilePath("BasicTransform.vert");
    m_SimpleDrawShader->SetFragShaderFilePath("BasicDraw.frag");
    m_SimpleDrawShader->Compile();

    glCreateFramebuffers(1, &m_Framebuffer);

    BuildRenderTargets();

    glCreateVertexArrays(1, &m_ScreenQuadVAO);
    glCreateBuffers(1, &m_ScreenQuadVBO);

    glVertexArrayVertexBuffer(m_ScreenQuadVAO, 0, m_ScreenQuadVBO, 0, sizeof(float) * 5);

    glEnableVertexArrayAttrib(m_ScreenQuadVAO, 0);
    glVertexArrayAttribBinding(m_ScreenQuadVAO, 0, 0);
    glVertexArrayAttribFormat(m_ScreenQuadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(m_ScreenQuadVAO, 1);
    glVertexArrayAttribBinding(m_ScreenQuadVAO, 1, 0);
    glVertexArrayAttribFormat(m_ScreenQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);

    GLfloat bufferData[] = { -1, -1, 0, 0, 0, 1, -1, 0, 1, 0, 1, 1, 0, 1, 1, -1, 1, 0, 0, 1 };

    glNamedBufferStorage(m_ScreenQuadVBO, sizeof(bufferData), bufferData, 0);
}

Renderer::~Renderer()
{
    glDeleteTextures(1, &m_FinalRenderTarget);
    glDeleteTextures(1, &m_TempRenderTarget);
    glDeleteTextures(1, &m_DSBuffer);
    glDeleteFramebuffers(1, &m_Framebuffer);

    glDeleteBuffers(1, &m_ScreenQuadVBO);
    glDeleteVertexArrays(1, &m_ScreenQuadVAO);
}

void Renderer::BuildRenderTargets()
{
    glDeleteTextures(1, &m_FinalRenderTarget);
    glDeleteTextures(1, &m_TempRenderTarget);
    glDeleteTextures(1, &m_DSBuffer);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_FinalRenderTarget);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_TempRenderTarget);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_DSBuffer);

    glTextureStorage2D(m_FinalRenderTarget, 1, GL_RGBA16F, m_WindowSize.x, m_WindowSize.y);
    glTextureParameteri(m_FinalRenderTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_FinalRenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTextureStorage2D(m_TempRenderTarget, 1, GL_RGBA16F, m_WindowSize.x, m_WindowSize.y);
    glTextureParameteri(m_TempRenderTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_TempRenderTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_TempRenderTarget, 0);

    glTextureStorage2D(m_DSBuffer, 1, GL_DEPTH24_STENCIL8, m_WindowSize.x, m_WindowSize.y);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_DSBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(m_Framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, m_DSBuffer, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Check framebuffer creation is a success
    GLenum status = glCheckNamedFramebufferStatus(m_Framebuffer, GL_FRAMEBUFFER);
    ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}

void Renderer::Update(float dt)
{
#ifdef EDITOR
    if (Input::Instance().GetKeyUp(KEY_O))
    {
        m_IsDebugDrawOn = !m_IsDebugDrawOn;
    }
#endif
    // Update light positions
    m_CurrentLayer->GetRenderLayer().UpdateAabbTree();

    // Clear screen buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_FinalRenderTarget, 0);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Editor mode render
    if (Application::Instance().GetGameTimer().IsEditorPaused() || Application::Instance().GetGameTimer().IsPlayModePaused())
    {
        if (m_CurrentLayer == nullptr)
        {
            return;
        }

        if (EditorCamera* camera = m_CurrentLayer->GetEditorCamera())
        {
            const RenderLayer& renderLayer = m_CurrentLayer->GetRenderLayer();
            const iVector2 vpSize = camera->GetViewportSize();
            const iVector2 vpOffset = camera->GetViewportOffset();
            const GLuint cameraRenderTarget = camera->GetRenderTarget();

            if (!camera->IsUICamera())
            {
                m_GeometryPass.StartMeshCulling(*camera, renderLayer);
                m_LightPass.StartLightCulling(*camera, renderLayer);
                // m_ParticlePass.StartSortingParticleData(*camera);
            }

            // const auto& cameras = renderLayer.GetCameras();
            //
            // for (const auto& cam : cameras)
            //{
            //  if (cam == camera)
            //	  continue;
            //  Frustum frustum;
            //  std::vector<Vector3> fp = cam->GetFrustumPoints();
            //  frustum.Set(fp[0], fp[1], fp[2], fp[3], fp[4], fp[5], fp[6], fp[7]);
            //
            //  frustum.DebugDraw(const_cast<Camera&>(*cam).GetParentObject()->GetParentLayer()->GetId());
            //}

            // Generate shadow maps
            glDisable(GL_SCISSOR_TEST);
            m_ShadowPass.Render(renderLayer);
            glEnable(GL_SCISSOR_TEST);

            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_TempRenderTarget, 0);
            glClearColor(0, 0, 0, 0);
            glClearDepth(1.0f);
            glClearStencil(0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // Set render area for geometry pass
            glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
            glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

            m_GeometryPass.Render(camera, renderLayer);

            // Copy depth buffer from geometry pass
            glBlitNamedFramebuffer(m_GeometryPass.GetFramebuffer(), m_Framebuffer, vpOffset.x, vpOffset.y, vpSize.x, vpSize.y, vpOffset.x, vpOffset.y, vpSize.x, vpSize.y, GL_DEPTH_BUFFER_BIT,
                                   GL_NEAREST);

            // Change back to scene buffer
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
            // Change viewport to screen space but retain scissor
            glViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
            glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

            if (camera->IsUICamera())
            {
                if (m_IsDebugDrawOn)
                {
                    glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                    glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                    m_DebugPass.Render(*camera, m_GeometryPass);
                }

                glCopyImageSubData(m_GeometryPass.GetGBuffers()[(int)GBuffer::Diffuse], GL_TEXTURE_2D, 0, vpOffset.x, vpOffset.y, 0, cameraRenderTarget, GL_TEXTURE_2D, 0, 0, 0, 0, vpSize.x, vpSize.y,
                                   1);
            }
            else
            {
                glViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
                glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                m_LightPass.Render(*camera, renderLayer, m_GeometryPass.GetGBuffers(), m_Framebuffer);

                glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                m_SkyboxPass.Render(*camera);
                m_WaterPass.Render(camera, renderLayer, m_Framebuffer);
                m_ParticlePass.Render(*camera);

                glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

                if (m_IsDebugDrawOn)
                {
                    glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                    glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                    m_DebugPass.Render(*camera, m_GeometryPass);
                }

                // Setup viewport for post process pass
                glViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
                glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                m_PostProcessPass.Render(*camera, m_TempRenderTarget, m_Framebuffer);
                m_FinalPass.Render(*camera, m_TempRenderTarget);

                glCopyImageSubData(m_FinalPass.GetRenderTexture(), GL_TEXTURE_2D, 0, vpOffset.x, vpOffset.y, 0, cameraRenderTarget, GL_TEXTURE_2D, 0, 0, 0, 0, vpSize.x, vpSize.y, 1);
            }

            // Render layer to "screen" buffer
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
            glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_FinalRenderTarget, 0);

            glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
            glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

            if (m_SimpleDrawShader->Enable())
            {
                static const GLuint mvpLoc = glGetUniformLocation(m_SimpleDrawShader->GetProgramHandle(), "gMVP");
                static const GLuint textureLoc = glGetUniformLocation(m_SimpleDrawShader->GetProgramHandle(), "gTexture");

                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
                glUniform1i(textureLoc, 0);
                // glBindTextureUnit(0, camera->IsUICamera() ? m_GeometryPass.GetGBuffers()[(int)GBuffer::Diffuse] : m_FinalPass.GetRenderTexture());
                glBindTextureUnit(0, cameraRenderTarget);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBlendEquation(GL_FUNC_ADD);

                glBindVertexArray(m_ScreenQuadVAO);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

                glDisable(GL_BLEND);
            }
        }
    }
    // Game mode render
    else
    {
        LayerList activeLayers = Application::Instance().GetCurrentScene()->GetLayers();

        for (Layer* currLayer : activeLayers)
        {
            const RenderLayer& renderLayer = currLayer->GetRenderLayer();
            const std::vector<Camera*>& cameras = renderLayer.GetCameras();

            for (Camera* camera : cameras)
            {
                if (camera->IsMainCamera())
                {
                    camera->SetViewportSize(m_WindowSize);
                }
            }

            glDisable(GL_SCISSOR_TEST);
            m_ShadowPass.Render(renderLayer);
            glEnable(GL_SCISSOR_TEST);

            for (Camera* camera : cameras)
            {
                if (camera->GetOwner()->GetActive())
                {
                    const iVector2 vpSize = camera->GetViewportSize();
                    const iVector2 vpOffset = camera->GetViewportOffset();

                    GLuint cameraRenderTarget = camera->GetRenderTarget();

                    if (!camera->IsUICamera())
                    {
                        m_GeometryPass.StartMeshCulling(*camera, renderLayer);
                        m_LightPass.StartLightCulling(*camera, renderLayer);
                        // m_ParticlePass.StartSortingParticleData(*camera);
                    }

                    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
                    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_TempRenderTarget, 0);
                    glClearColor(0, 0, 0, 0);
                    glClearDepth(1.0f);
                    glClearStencil(0);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                    // Set render area for geometry pass
                    glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                    glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                    m_GeometryPass.Render(camera, renderLayer);

                    // Copy depth buffer from geometry pass
                    glBlitNamedFramebuffer(m_GeometryPass.GetFramebuffer(), m_Framebuffer, vpOffset.x, vpOffset.y, vpSize.x, vpSize.y, vpOffset.x, vpOffset.y, vpSize.x, vpSize.y, GL_DEPTH_BUFFER_BIT,
                                           GL_NEAREST);

                    // Change back to scene buffer
                    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
                    // Change viewport to screen space but retain scissor
                    glViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
                    glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                    if (camera->IsUICamera())
                    {
                        if (m_IsDebugDrawOn)
                        {
                            glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                            glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                            m_DebugPass.Render(*camera, m_GeometryPass);
                        }

                        glCopyImageSubData(m_GeometryPass.GetGBuffers()[(int)GBuffer::Diffuse], GL_TEXTURE_2D, 0, vpOffset.x, vpOffset.y, 0, cameraRenderTarget, GL_TEXTURE_2D, 0, 0, 0, 0, vpSize.x,
                                           vpSize.y, 1);
                    }
                    else
                    {
                        glViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
                        glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                        m_LightPass.Render(*camera, renderLayer, m_GeometryPass.GetGBuffers(), m_Framebuffer);

                        glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                        glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                        m_SkyboxPass.Render(*camera);
                        m_WaterPass.Render(camera, renderLayer, m_Framebuffer);
                        m_ParticlePass.Render(*camera);

                        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

                        if (m_IsDebugDrawOn)
                        {
                            glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                            glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                            m_DebugPass.Render(*camera, m_GeometryPass);
                        }

                        // Setup viewport for post process pass
                        glViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
                        glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                        m_PostProcessPass.Render(*camera, m_TempRenderTarget, m_Framebuffer);
                        m_FinalPass.Render(*camera, m_TempRenderTarget);

                        glCopyImageSubData(m_FinalPass.GetRenderTexture(), GL_TEXTURE_2D, 0, vpOffset.x, vpOffset.y, 0, cameraRenderTarget, GL_TEXTURE_2D, 0, 0, 0, 0, vpSize.x, vpSize.y, 1);
                    }

                    // Render layer to "screen" buffer
                    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
                    glNamedFramebufferTexture(m_Framebuffer, GL_COLOR_ATTACHMENT0, m_FinalRenderTarget, 0);

                    glScissor(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);
                    glViewport(vpOffset.x, vpOffset.y, vpSize.x, vpSize.y);

                    if (m_SimpleDrawShader->Enable())
                    {
                        static const GLuint mvpLoc = glGetUniformLocation(m_SimpleDrawShader->GetProgramHandle(), "gMVP");
                        static const GLuint textureLoc = glGetUniformLocation(m_SimpleDrawShader->GetProgramHandle(), "gTexture");

                        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, ValuePtrFloat(Matrix4()));
                        glUniform1i(textureLoc, 0);
                        glBindTextureUnit(0, cameraRenderTarget);

                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        glBlendEquation(GL_FUNC_ADD);

                        glBindVertexArray(m_ScreenQuadVAO);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

                        glDisable(GL_BLEND);
                    }
                }
            }
            glDisable(GL_SCISSOR_TEST);
        }
    }

    // Clear framebuffer for window
#ifdef EDITOR
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK_LEFT);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBlitNamedFramebuffer(m_Framebuffer, 0, 0, 0, m_WindowSize.x, m_WindowSize.y, 0, 0, m_WindowSize.x, m_WindowSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#endif
}

void Renderer::DrawLine(const Vector3& s, const Vector3& e, Color4 color)
{
#ifdef EDITOR
    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();
    std::vector<Vector3> pts;
    pts.push_back(s);
    pts.push_back(e);
    m_DebugPass.DrawDebug(currentLayerID, pts, color, DebugPrimitiveType::Lines);
#endif
}

void Renderer::DrawDebug(unsigned layerId, std::vector<Vector3> const& geometry, Color4 color, DebugPrimitiveType type)
{
#ifdef EDITOR
    m_DebugPass.DrawDebug(layerId, geometry, color, type);
#endif
}

void Renderer::DrawCube(float w, Vector3 pos)
{
#ifdef EDITOR
    std::vector<Vector3> pts;
    float halfWidth = w;
    float halfHeight = w;
    float halfDepth = w;
    Vector3 minPt(pos.x - halfWidth, pos.y - halfHeight, pos.z - halfDepth);
    Vector3 maxPt(pos.x + halfWidth, pos.y + halfHeight, pos.z + halfDepth);
    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { maxPt.x, maxPt.y, minPt.z });

    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, maxPt.z });

    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { minPt.x, maxPt.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, minPt.y, maxPt.z });
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, maxPt.z });

    pts.emplace_back(Vector3 { maxPt.x, minPt.y, minPt.z });
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, maxPt.z });

    pts.emplace_back(Vector3 { maxPt.x, maxPt.y, minPt.z });
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, minPt.z });

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { maxPt.x, minPt.y, minPt.z });

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { minPt.x, minPt.y, maxPt.z });

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { minPt.x, maxPt.y, minPt.z });

    pts.emplace_back(Vector3 { minPt.x, maxPt.y, maxPt.z });
    pts.emplace_back(Vector3 { minPt.x, minPt.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, maxPt.y, minPt.z });
    pts.emplace_back(Vector3 { minPt.x, maxPt.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, maxPt.y, minPt.z });
    pts.emplace_back(Vector3 { maxPt.x, maxPt.y, minPt.z });

    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();

    DrawDebug(currentLayerID, pts, Color4(1.0f, 0.0f, 0.0f, 1.0f), DebugPrimitiveType::Lines);
#endif
}

void Renderer::Draw2DBox(float w, float h, Vector3 pos, Color4 col)
{
#ifdef EDITOR

    std::vector<Vector3> pts;
    float halfWidth = w;
    float halfDepth = h;
    Vector3 minPt(pos.x - halfWidth, pos.y, pos.z - halfDepth);
    Vector3 maxPt(pos.x + halfWidth, pos.y, pos.z + halfDepth);
    pts.emplace_back(maxPt);
    pts.emplace_back(Vector3 { maxPt.x, pos.y, minPt.z });

    pts.emplace_back(Vector3 { maxPt.x, pos.y, minPt.z });
    pts.emplace_back(minPt);

    pts.emplace_back(minPt);
    pts.emplace_back(Vector3 { minPt.x, pos.y, maxPt.z });

    pts.emplace_back(Vector3 { minPt.x, pos.y, maxPt.z });
    pts.emplace_back(maxPt);

    auto currentLayerID = Renderer::Instance().GetCurrentEditorLayer()->GetId();

    DrawDebug(currentLayerID, pts, col, DebugPrimitiveType::Lines);
#endif
}

void Renderer::DrawDebugBox(unsigned layerId, Color4 color)
{
#ifdef EDITOR
    DrawDebug(layerId, gWireBoxMesh, color, DebugPrimitiveType::Lines);
#endif
}

void Renderer::DrawSelectionBox(float left, float right, float top, float bottom)
{
    m_DebugPass.DrawSelectionBox(left, right, top, bottom);
}

void Renderer::SetCurrentLayer(Layer* layer)
{
    m_CurrentLayer = layer;
}

void Renderer::SetWindowSize(iVector2 const& windowSize)
{
    if (windowSize.x <= 0 || windowSize.y <= 0 || windowSize.u <= 0 || windowSize.v <= 0) return;
    if (windowSize != m_WindowSize)
    {
        m_WindowSize = windowSize;
        BuildRenderTargets();

        m_GeometryPass.SetViewportSize(windowSize);
        m_LightPass.SetViewportSize(windowSize);
        // m_WaterPass.SetViewportSize(windowSize);
        m_PostProcessPass.SetViewportSize(windowSize);
        m_FinalPass.SetViewportSize(windowSize);
    }
}

iVector2 Renderer::GetWindowSize() const
{
    return m_WindowSize;
}

GLuint Renderer::GetRenderTexture() const
{
#ifdef EDITOR
    if (m_RenderIndex == 1) return m_GeometryPass.GetGBuffers()[(int)GBuffer::Diffuse];
    if (m_RenderIndex == 2) return m_GeometryPass.GetGBuffers()[(int)GBuffer::WorldNormal];
    if (m_RenderIndex == 3) return m_GeometryPass.GetGBuffers()[(int)GBuffer::Specular];
    if (m_RenderIndex == 4) return m_GeometryPass.GetGBuffers()[(int)GBuffer::WorldPosition];
    if (m_RenderIndex == 5) return m_GeometryPass.GetGBuffers()[(int)GBuffer::TexCoords];
#endif
    return m_FinalRenderTarget;
}

GLuint Renderer::GetPickedObject(iVector2 mousePosition) const
{
    return m_GeometryPass.GetPickedID(mousePosition);
}

void Renderer::SetSelectedObjects(std::vector<unsigned> const& selectedObjects)
{
    m_DebugPass.SetSelectedObjects(selectedObjects);
}

void Renderer::ChangeCamera(bool b)
{
    m_UseCamera = b;
}
